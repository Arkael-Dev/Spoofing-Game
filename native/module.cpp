// Zygisk module, active only inside the apps listed in GPU_APPS (config.conf):
//  - GPU spoof: GL_RENDERER / GL_VENDOR and the Vulkan device name
//  - device spoof (DEV_MODE="game"): model, brand, manufacturer, SoC
//    through android.os.Build and the native system property readers
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vulkan/vulkan.h>
#include "zygisk.hpp"
#include <pthread.h>
#include "gothook.h"
#include "prophook.h"
#include <jni.h>
#ifdef SG_DEBUG
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "SpoofGame", __VA_ARGS__)
#else
#define LOGI(...) ((void) 0)
#endif

static std::string g_renderer, g_vendor;
static std::string g_model, g_market, g_brand, g_mfr, g_soc, g_socname;

static std::string cfgGet(const std::string &txt, const char *key) {
    std::string k = std::string(key) + "=\"";
    size_t p = 0;
    while ((p = txt.find(k, p)) != std::string::npos) {
        if (p == 0 || txt[p - 1] == '\n') {
            size_t s = p + k.size(), e = txt.find('"', s);
            return e == std::string::npos ? "" : txt.substr(s, e - s);
        }
        p += k.size();
    }
    return "";
}

typedef const unsigned char *(*GetString_t)(unsigned int);
static GetString_t o_glGetString;
static const unsigned char *h_glGetString(unsigned int n) {
    if (n == 0x1F01 && !g_renderer.empty()) return (const unsigned char *) g_renderer.c_str();
    if (n == 0x1F00 && !g_vendor.empty()) return (const unsigned char *) g_vendor.c_str();
    return o_glGetString(n);
}

static void setName(VkPhysicalDeviceProperties *p) {
    if (!p || g_renderer.empty()) return;
    strncpy(p->deviceName, g_renderer.c_str(), VK_MAX_PHYSICAL_DEVICE_NAME_SIZE - 1);
    p->deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE - 1] = 0;
}
static PFN_vkGetPhysicalDeviceProperties o_vk1;
static void h_vk1(VkPhysicalDevice d, VkPhysicalDeviceProperties *p) { o_vk1(d, p); setName(p); }
static PFN_vkGetPhysicalDeviceProperties2 o_vk2;
static void h_vk2(VkPhysicalDevice d, VkPhysicalDeviceProperties2 *p) { o_vk2(d, p); if (p) setName(&p->properties); }
static PFN_vkGetPhysicalDeviceProperties2 o_vk2k;
static void h_vk2k(VkPhysicalDevice d, VkPhysicalDeviceProperties2 *p) { o_vk2k(d, p); if (p) setName(&p->properties); }

typedef void *(*Gpa_t)(const char *);
static Gpa_t o_egl;
static PFN_vkGetInstanceProcAddr o_gipa;

static void *h_egl(const char *n) {
    if (n && strcmp(n, "glGetString") == 0 && o_glGetString) return (void *) h_glGetString;
    return o_egl ? o_egl(n) : nullptr;
}
static PFN_vkVoidFunction h_gipa(VkInstance i, const char *n) {
    PFN_vkVoidFunction p = o_gipa(i, n);
    if (!p || !n) return p;
    if (!strcmp(n, "vkGetPhysicalDeviceProperties")) { o_vk1 = (PFN_vkGetPhysicalDeviceProperties) p; return (PFN_vkVoidFunction) h_vk1; }
    if (!strcmp(n, "vkGetPhysicalDeviceProperties2")) { o_vk2 = (PFN_vkGetPhysicalDeviceProperties2) p; return (PFN_vkVoidFunction) h_vk2; }
    if (!strcmp(n, "vkGetPhysicalDeviceProperties2KHR")) { o_vk2k = (PFN_vkGetPhysicalDeviceProperties2) p; return (PFN_vkVoidFunction) h_vk2k; }
    return p;
}

static gothook::Entry g_entries[16];
static int g_n = 0;
static uintptr_t g_self = 0;

static void add(const char *sym, void *repl) { g_entries[g_n++] = {sym, repl}; }

// Patches app libraries under /data and, inside the app only, the Java GLES bridge.
static void *scanner(void *) {
    for (int i = 0; i < 120; i++) {
        gothook::scan(g_entries, g_n, "/data/", g_self);
        // Java GLES10/GLES20.glGetString goes through libandroid_runtime
        gothook::scan(g_entries, g_n, "/system/lib64/libandroid_runtime.so", g_self);
        if (i == 0 || i == 10 || i == 119) LOGI("scan %d: %d slots patched", i, gothook::g_patched);
        usleep(500000);
    }
    return nullptr;
}

static void onPropHit(const char *name) {
    static int c = 0;
    if (c++ < 30) LOGI("prop spoofed: %s", name);
}

static void buildProps() {
    const char *P[] = {"", "system.", "vendor.", "odm.", "product.", "system_ext."};
    for (const char *p : P) {
        std::string b = std::string("ro.product.") + p;
        prophook::add(b + "model", g_model);
        prophook::add(b + "marketname", g_market);
        prophook::add(b + "brand", g_brand);
        prophook::add(b + "manufacturer", g_mfr);
    }
    prophook::add("ro.soc.model", g_soc);
    prophook::add("ro.vendor.soc.model.part_name", g_soc);
    prophook::add("ro.vendor.soc.model.external_name", g_socname);
    prophook::add("ro.chipset", g_soc);
}

static void addPropHooks() {
    buildProps();
    if (prophook::g_props.empty()) return;
    prophook::g_onHit = onPropHit;
    prophook::o_get = (prophook::Get_t) dlsym(RTLD_DEFAULT, "__system_property_get");
    prophook::o_find = (prophook::Find_t) dlsym(RTLD_DEFAULT, "__system_property_find");
    prophook::o_cb = (prophook::ReadCb_t) dlsym(RTLD_DEFAULT, "__system_property_read_callback");
    prophook::o_read = (prophook::Read_t) dlsym(RTLD_DEFAULT, "__system_property_read");
    if (prophook::o_get) add("__system_property_get", (void *) prophook::h_get);
    if (prophook::o_find) add("__system_property_find", (void *) prophook::h_find);
    if (prophook::o_cb) add("__system_property_read_callback", (void *) prophook::h_cb);
    if (prophook::o_read) add("__system_property_read", (void *) prophook::h_read);
}

static void setBuild(JNIEnv *env, const char *field, const std::string &v) {
    if (!env || v.empty()) return;
    jclass c = env->FindClass("android/os/Build");
    if (!c) { env->ExceptionClear(); return; }
    jfieldID f = env->GetStaticFieldID(c, field, "Ljava/lang/String;");
    if (!f) {
        env->ExceptionClear();
        LOGI("Build.%s not found", field);
    } else {
        jstring js = env->NewStringUTF(v.c_str());
        env->SetStaticObjectField(c, f, js);
        if (env->ExceptionCheck()) { env->ExceptionClear(); LOGI("Build.%s set failed", field); }
        else LOGI("Build.%s = %s", field, v.c_str());
        if (js) env->DeleteLocalRef(js);
    }
    env->DeleteLocalRef(c);
}

static void installHooks(bool gpu, bool dev) {
    Dl_info di;
    if (dladdr((void *) &installHooks, &di)) g_self = (uintptr_t) di.dli_fbase;
    if (dev) addPropHooks();
    if (gpu) {
    void *gl = dlopen("libGLESv2.so", RTLD_NOW);
    if (gl && (o_glGetString = (GetString_t) dlsym(gl, "glGetString"))) add("glGetString", (void *) h_glGetString);
    void *egl = dlopen("libEGL.so", RTLD_NOW);
    if (egl && (o_egl = (Gpa_t) dlsym(egl, "eglGetProcAddress"))) add("eglGetProcAddress", (void *) h_egl);
    void *vk = dlopen("libvulkan.so", RTLD_NOW);
    if (vk) {
        o_vk1 = (PFN_vkGetPhysicalDeviceProperties) dlsym(vk, "vkGetPhysicalDeviceProperties");
        o_vk2 = (PFN_vkGetPhysicalDeviceProperties2) dlsym(vk, "vkGetPhysicalDeviceProperties2");
        o_vk2k = (PFN_vkGetPhysicalDeviceProperties2) dlsym(vk, "vkGetPhysicalDeviceProperties2KHR");
        o_gipa = (PFN_vkGetInstanceProcAddr) dlsym(vk, "vkGetInstanceProcAddr");
        if (o_vk1) add("vkGetPhysicalDeviceProperties", (void *) h_vk1);
        if (o_vk2) add("vkGetPhysicalDeviceProperties2", (void *) h_vk2);
        if (o_vk2k) add("vkGetPhysicalDeviceProperties2KHR", (void *) h_vk2k);
        if (o_gipa) add("vkGetInstanceProcAddr", (void *) h_gipa);
    }
    }
    LOGI("hooks: %d symbols (gpu=%d dev=%d)", g_n, (int) gpu, (int) dev);
    if (!g_n) return;
    pthread_t th;
    if (pthread_create(&th, nullptr, scanner, nullptr) == 0) pthread_detach(th);
}

class SpoofGame : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override { api_ = api; env_ = env; }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        const char *nn = args->nice_name ? env_->GetStringUTFChars(args->nice_name, nullptr) : nullptr;
        std::string proc = nn ? nn : "";
        if (nn) env_->ReleaseStringUTFChars(args->nice_name, nn);
        proc = proc.substr(0, proc.find(':'));
        // Never touch module fds in zygote-type processes (webview_zygote,
        // <pkg>_zygote): a leaked fd there makes the zygote abort on fork
        // (FileDescriptorInfo::CreateFromFd) and browsers stop opening.
        if (proc.empty() || (proc.size() >= 6 && proc.compare(proc.size() - 6, 6, "zygote") == 0)) {
            api_->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        std::string txt;
        int dir = api_->getModuleDir();
        int fd = dir >= 0 ? openat(dir, "config.conf", O_RDONLY) : -1;
        if (fd >= 0) {
            char buf[4096];
            ssize_t n;
            while ((n = read(fd, buf, sizeof buf)) > 0) txt.append(buf, n);
            close(fd);
        }
        if (cfgGet(txt, "ENABLED") == "1") {
            std::string apps = "," + cfgGet(txt, "GPU_APPS") + ",";
            if (apps.find("," + proc + ",") != std::string::npos) {
                if (cfgGet(txt, "GPU_ENABLED") == "1") {
                    g_renderer = cfgGet(txt, "GPU_RENDERER");
                    g_vendor = cfgGet(txt, "GPU_VENDOR");
                    gpuOn_ = true;
                }
                if (cfgGet(txt, "DEV_MODE") == "game") {
                    g_model = cfgGet(txt, "MODEL");
                    g_market = cfgGet(txt, "MARKET");
                    g_brand = cfgGet(txt, "BRAND");
                    g_mfr = cfgGet(txt, "MFR");
                    g_soc = cfgGet(txt, "SOC");
                    g_socname = cfgGet(txt, "SOCNAME");
                    devOn_ = !g_model.empty();
                }
            }
        }
        LOGI("%s: gpu=%d dev=%d", proc.c_str(), (int) gpuOn_, (int) devOn_);
        if (!gpuOn_ && !devOn_) api_->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *) override {
        if (!gpuOn_ && !devOn_) return;
        if (devOn_) {
            setBuild(env_, "MODEL", g_model);
            setBuild(env_, "BRAND", g_brand);
            setBuild(env_, "MANUFACTURER", g_mfr);
            setBuild(env_, "SOC_MODEL", g_soc);
        }
        installHooks(gpuOn_, devOn_);
    }

private:
    zygisk::Api *api_ = nullptr;
    JNIEnv *env_ = nullptr;
    bool gpuOn_ = false, devOn_ = false;
};

REGISTER_ZYGISK_MODULE(SpoofGame)
