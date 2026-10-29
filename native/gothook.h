// Minimal GOT/PLT hooker (64-bit RELA). Patches import slots of loaded
// libraries whose path starts with `prefix`. No third-party dependency.
#pragma once
#include <elf.h>
#include <link.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

namespace gothook {

struct Entry { const char *sym; void *repl; };

#if defined(__aarch64__)
static const unsigned kJump = 1026, kGlob = 1025, kAbs = 257;
#else
static const unsigned kJump = 7, kGlob = 6, kAbs = 1;   // x86_64 (host tests)
#endif

struct Ctx { const Entry *e; int n; const char *prefix; uintptr_t skip; };

static inline void patchSlot(void **slot, void *repl) {
    if (*slot == repl) return;
    uintptr_t pg = (uintptr_t) slot & ~(uintptr_t) (getpagesize() - 1);
    if (mprotect((void *) pg, getpagesize(), PROT_READ | PROT_WRITE) != 0) return;
    *slot = repl;
}

static inline void walk(const ElfW(Rela) *r, size_t bytes, uintptr_t bias, const ElfW(Sym) *sym,
                        const char *str, const Ctx *c) {
    for (size_t i = 0; i < bytes / sizeof(ElfW(Rela)); i++) {
        unsigned type = ELF64_R_TYPE(r[i].r_info), idx = ELF64_R_SYM(r[i].r_info);
        if (!idx || (type != kJump && type != kGlob && type != kAbs)) continue;
        const char *name = str + sym[idx].st_name;
        for (int k = 0; k < c->n; k++)
            if (strcmp(name, c->e[k].sym) == 0) patchSlot((void **) (bias + r[i].r_offset), c->e[k].repl);
    }
}

static inline int onLib(struct dl_phdr_info *info, size_t, void *data) {
    const Ctx *c = (const Ctx *) data;
    uintptr_t bias = info->dlpi_addr;
    const char *name = info->dlpi_name;
    if (!name || !*name || bias == c->skip) return 0;
    if (c->prefix && strncmp(name, c->prefix, strlen(c->prefix)) != 0) return 0;
    const ElfW(Dyn) *dyn = nullptr;
    for (int i = 0; i < info->dlpi_phnum; i++)
        if (info->dlpi_phdr[i].p_type == PT_DYNAMIC) dyn = (const ElfW(Dyn) *) (bias + info->dlpi_phdr[i].p_vaddr);
    if (!dyn) return 0;
    // bionic keeps d_ptr as offsets, glibc relocates them: accept both
    auto fix = [&](uintptr_t p) { return p < bias ? p + bias : p; };
    const ElfW(Sym) *sym = nullptr; const char *str = nullptr;
    const ElfW(Rela) *jmp = nullptr, *rela = nullptr; size_t jsz = 0, rsz = 0;
    for (const ElfW(Dyn) *d = dyn; d->d_tag != DT_NULL; d++) {
        switch (d->d_tag) {
            case DT_SYMTAB: sym = (const ElfW(Sym) *) fix(d->d_un.d_ptr); break;
            case DT_STRTAB: str = (const char *) fix(d->d_un.d_ptr); break;
            case DT_JMPREL: jmp = (const ElfW(Rela) *) fix(d->d_un.d_ptr); break;
            case DT_PLTRELSZ: jsz = d->d_un.d_val; break;
            case DT_RELA: rela = (const ElfW(Rela) *) fix(d->d_un.d_ptr); break;
            case DT_RELASZ: rsz = d->d_un.d_val; break;
        }
    }
    if (!sym || !str) return 0;
    if (jmp && jsz) walk(jmp, jsz, bias, sym, str, c);
    if (rela && rsz) walk(rela, rsz, bias, sym, str, c);
    return 0;
}

static inline void scan(const Entry *e, int n, const char *prefix, uintptr_t skipBase) {
    Ctx c{e, n, prefix, skipBase};
    dl_iterate_phdr(onLib, &c);
}

}  // namespace gothook
