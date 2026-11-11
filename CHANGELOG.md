# Changelog

## v2.8.2
- REDMAGIC 12 Pro (NX819J) preset: nubia/nubia per reference
- Global spoof: ro.product.device/name = NX819J (nubia profile only)
- Spoof SoC manufacturer (ro.soc.manufacturer / Build.SOC_MANUFACTURER) in global + Zygisk game mode
- New banner

## v2.8.1
**ID**
- Rilis uji update otomatis KSU Next, tanpa perubahan fitur.

**EN**
- Auto-update test release for KSU Next, no feature changes.

## v2.8
**ID**
- Default dan preset kini ZTE nubia RedMagic 12 Pro: merek ZTE, nama "nubia RedMagic 12 Pro", model `NX819J`, SM8975 Snapdragon 8 Elite Extreme Gen 6, GPU Adreno 850.

**EN**
- Default and preset are now ZTE nubia RedMagic 12 Pro: brand ZTE, name "nubia RedMagic 12 Pro", model `NX819J`, SM8975 Snapdragon 8 Elite Extreme Gen 6, Adreno 850 GPU.

## v2.7
**ID**
- Preset baru: RedMagic 12 Pro (global) `NX819J`, SM8975 Snapdragon 8 Elite Extreme Gen 6, Adreno 850.

**EN**
- New preset: RedMagic 12 Pro (global) `NX819J`, SM8975 Snapdragon 8 Elite Extreme Gen 6, Adreno 850.

## v2.5
**ID**
- Notifikasi status: mode "hanya game pilihan" kini tertulis "spoof Zygisk" (sebelumnya "spoof per-game"). Mode global tetap "spoof global".

**EN**
- Status notification: "selected games only" mode now reads "Zygisk spoof" (was "per-game spoof"). Global mode still reads "global spoof".

## v2.4
**ID**
- Notifikasi status setelah Simpan kini mengikuti bahasa WebUI (Indonesia / English). Jika belum pernah memilih, mengikuti bahasa sistem.
- Changelog di KSU Next sekarang lengkap per versi, dua bahasa.
- Catatan rilis GitHub otomatis diambil dari changelog ini.

**EN**
- The status notification after Save now follows the WebUI language (Indonesian / English). If none was chosen yet, it follows the system language.
- The changelog shown in KSU Next is now complete per version, in two languages.
- GitHub release notes are generated from this changelog.

## v2.3
**ID**
- Notifikasi status (ikon modul + ringkasan: Hz, GPU, mode spoof, jumlah game) muncul sekali setelah Simpan dan terapkan di WebUI.

**EN**
- Status notification (module icon + summary: Hz, GPU, spoof mode, game count) shown once after Save and apply in the WebUI.

## v2.2
**ID**
- Boost performa saat main: CPU governor performance, animasi dipercepat, swappiness diturunkan selama game di daftar ada di layar depan. Kembali otomatis saat keluar dari game.
- Saklar baru "Boost performa saat game dibuka" di WebUI (default mati).

**EN**
- Performance boost while playing: CPU governor forced to performance, animations sped up, swappiness lowered while a listed game is in the foreground. Reverts automatically when you leave the game.
- New "Boost performance while a game is open" switch in the WebUI (off by default).

## v2.1
**ID**
- Kembali ke fitur v1.8 (profil per game dan tampilan baru dari v1.9 dibatalkan). Versi ini stabil.

**EN**
- Reverted to the v1.8 feature set (the per-game profile and restyle from v1.9 were rolled back). Stable release.

## v2.0
**ID**
- Hanya naik versi.

**EN**
- Version bump only.

## v1.9
**ID**
- Tampilan WebUI baru: tema gelap dan animasi.
- Profil per game: override perangkat + GPU lewat Zygisk.

**EN**
- Restyled WebUI: dark theme and animations.
- Per-game profile: device + GPU override via Zygisk.

## v1.8
**ID**
- Spoof model dan SoC lewat Zygisk, pilih mode global atau hanya game pilihan. Rilis stabil (sebelumnya v1.8-beta1).

**EN**
- Model and SoC spoof via Zygisk, choose global or selected games only. Stable release (previously v1.8-beta1).

## v1.7
**ID**
- Downscale 30%.
- Kunci FPS per game.

**EN**
- 30% downscale option.
- Per-game FPS lock.

## v1.6
**ID**
- Kolom cari aplikasi dihapus dari WebUI.

**EN**
- Removed the app search box from the WebUI.

## v1.5
**ID**
- Hook GPU untuk sisi Java.
- Downscale per aplikasi.
- Deteksi game terpasang otomatis.

**EN**
- GPU hook for the Java side.
- Per-app downscale.
- Automatic detection of installed games.

## v1.4
**ID**
- Preset POCO F8 Ultra.
- Perbaikan crash zygote.

**EN**
- POCO F8 Ultra preset.
- Fixed a zygote crash.

## v1.3
**ID**
- Default perangkat POCO F8 Ultra.
- Hook GOT menggantikan Dobby.

**EN**
- Default device is now POCO F8 Ultra.
- GOT hook replaces Dobby.

## v1.2
**ID**
- Spoof GPU (GL_RENDERER, GL_VENDOR, nama perangkat Vulkan) lewat Zygisk, hanya untuk paket game yang terdaftar.
- WebUI Indonesia / English, GPU, SoC, model, dan nama kustom; isi dari nilai terbaca.
- Banner lebih terang.
- Migrasi config mempertahankan pengaturan lama.

**EN**
- GPU spoof (GL_RENDERER, GL_VENDOR, Vulkan device name) via Zygisk, only for listed game packages.
- WebUI in Indonesian / English, custom GPU, SoC, model and name; fill from current values.
- Brighter banner.
- Config migration keeps your old settings.

## v1.1
**ID**
- WebUI, config.conf, banner, build GitHub Actions, update.json.

**EN**
- WebUI, config.conf, banner, GitHub Actions build, update.json.
