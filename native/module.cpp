// Zygisk module: spoofs GL_RENDERER / GL_VENDOR and the Vulkan device name
// only inside the apps listed in GPU_APPS (config.conf).
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vulkan/vulkan.h>
#include "zygisk.hpp"
#include "dobby.h"

static std::string g_renderer, g_vendor;

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

// Works with any Dobby version: the cast types are deduced from DobbyHook itself.
template <typename R, typename A, typename B, typename C>
static R callHook(R (*f)(A, B, C), void *a, void *b, void **c) { return f((A) a, (B) b, (C) c); }

static void hookSym(void *lib, const char *name, void *repl, void **orig) {
    void *addr = lib ? dlsym(lib, name) : nullptr;
    if (addr) callHook(&DobbyHook, addr, repl, orig);
}

static void installHooks() {
    void *gl = dlopen("libGLESv2.so", RTLD_NOW);
    hookSym(gl, "glGetString", (void *) h_glGetString, (void **) &o_glGetString);
    void *vk = dlopen("libvulkan.so", RTLD_NOW);
    hookSym(vk, "vkGetPhysicalDeviceProperties", (void *) h_vk1, (void **) &o_vk1);
    hookSym(vk, "vkGetPhysicalDeviceProperties2", (void *) h_vk2, (void **) &o_vk2);
    hookSym(vk, "vkGetPhysicalDeviceProperties2KHR", (void *) h_vk2k, (void **) &o_vk2k);
}

class SpoofGpu : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override { api_ = api; env_ = env; }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        const char *nn = env_->GetStringUTFChars(args->nice_name, nullptr);
        std::string proc = nn ? nn : "";
        if (nn) env_->ReleaseStringUTFChars(args->nice_name, nn);
        proc = proc.substr(0, proc.find(':'));

        std::string txt;
        int dir = api_->getModuleDir();
        int fd = dir >= 0 ? openat(dir, "config.conf", O_RDONLY) : -1;
        if (fd >= 0) {
            char buf[4096];
            ssize_t n;
            while ((n = read(fd, buf, sizeof buf)) > 0) txt.append(buf, n);
            close(fd);
        }
        if (cfgGet(txt, "ENABLED") == "1" && cfgGet(txt, "GPU_ENABLED") == "1") {
            std::string apps = "," + cfgGet(txt, "GPU_APPS") + ",";
            if (apps.find("," + proc + ",") != std::string::npos) {
                g_renderer = cfgGet(txt, "GPU_RENDERER");
                g_vendor = cfgGet(txt, "GPU_VENDOR");
                active_ = true;
            }
        }
        if (!active_) api_->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *) override {
        if (active_) installHooks();
    }

private:
    zygisk::Api *api_ = nullptr;
    JNIEnv *env_ = nullptr;
    bool active_ = false;
};

REGISTER_ZYGISK_MODULE(SpoofGpu)
