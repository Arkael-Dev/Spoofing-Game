// Property spoof for one app (bionic). Answers __system_property_get / find /
// read_callback / read with fixed values for a fixed set of names; every other
// name goes to the real function. No third-party dependency.
#pragma once
#include <stdint.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

namespace prophook {

typedef void (*Cb)(void *cookie, const char *name, const char *value, uint32_t serial);
typedef int (*Get_t)(const char *, char *);
typedef const void *(*Find_t)(const char *);
typedef void (*ReadCb_t)(const void *, Cb, void *);
typedef int (*Read_t)(const void *, char *, char *);

static Get_t o_get;
static Find_t o_find;
static ReadCb_t o_cb;
static Read_t o_read;
static void (*g_onHit)(const char *name);
static std::vector<std::pair<std::string, std::string>> g_props;
static char g_tok[64];  // fake prop_info addresses: &g_tok[i] stands for g_props[i]

static inline void add(const std::string &name, const std::string &value) {
    if (!value.empty() && g_props.size() < sizeof g_tok) g_props.emplace_back(name, value);
}
static inline int idx(const char *n) {
    if (!n) return -1;
    for (size_t i = 0; i < g_props.size(); i++)
        if (g_props[i].first == n) return (int) i;
    return -1;
}
static inline int tokIdx(const void *pi) {
    const char *p = (const char *) pi;
    return (p >= g_tok && p < g_tok + g_props.size()) ? (int) (p - g_tok) : -1;
}
static inline void hit(int i) { if (g_onHit) g_onHit(g_props[i].first.c_str()); }

static int h_get(const char *n, char *v) {
    int i = idx(n);
    if (i >= 0 && v) {
        hit(i);
        strncpy(v, g_props[i].second.c_str(), 91);
        v[91] = 0;
        return (int) strlen(v);
    }
    return o_get ? o_get(n, v) : 0;
}
static const void *h_find(const char *n) {
    int i = idx(n);
    if (i >= 0) { hit(i); return (const void *) &g_tok[i]; }
    return o_find ? o_find(n) : nullptr;
}
static void h_cb(const void *pi, Cb cb, void *cookie) {
    int i = tokIdx(pi);
    if (i >= 0) {
        if (cb) cb(cookie, g_props[i].first.c_str(), g_props[i].second.c_str(), 2);
        return;
    }
    if (o_cb) o_cb(pi, cb, cookie);
}
static int h_read(const void *pi, char *name, char *value) {
    int i = tokIdx(pi);
    if (i >= 0) {
        if (name) { strncpy(name, g_props[i].first.c_str(), 31); name[31] = 0; }
        if (value) { strncpy(value, g_props[i].second.c_str(), 91); value[91] = 0; return (int) strlen(value); }
        return (int) g_props[i].second.size();
    }
    return o_read ? o_read(pi, name, value) : 0;
}

}  // namespace prophook
