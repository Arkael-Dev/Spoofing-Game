# Spoofing Game

![Build](https://github.com/Arkael-Dev/Spoofing-Game/actions/workflows/build.yml/badge.svg)

**EN** KernelSU / KernelSU Next / Magisk module that spoofs device model, SoC and GPU so games unlock higher graphics and FPS options. Default: POCO F8 Ultra (`25102PCBEG`, Adreno 840). Everything is editable in the WebUI.
**ID** Modul KernelSU / KernelSU Next / Magisk untuk spoof model, SoC, dan GPU supaya game membuka opsi grafik dan FPS lebih tinggi. Semua bisa diubah di WebUI.

## Features / Fitur
- Device name, model code, brand, manufacturer, SoC, OpenGL ES, `/proc/cpuinfo`
- GPU spoof (OpenGL ES + Vulkan) per game package, needs Zygisk (Zygisk Next)
- Refresh rate 60 / 90 / 120 / 144 Hz
- WebUI in English and Indonesian

## Install / Pasang
1. Download the latest zip from Releases / Unduh zip terbaru dari Releases.
2. Flash in KernelSU Next or Magisk, enable Zygisk, reboot / Flash, aktifkan Zygisk, reboot.
3. Open WebUI, pick or type your target device, tap Save and apply / Buka WebUI, pilih atau isi perangkat tujuan, tekan Simpan dan terapkan.
4. Force-stop the game and reopen / Tutup paksa game lalu buka lagi.

## Notes / Catatan
- GPU spoof only changes the names games read. The real GPU still renders, so heavy presets can lag or heat up. / Spoof GPU hanya mengubah nama yang dibaca game. GPU asli tetap yang merender.
- Anti-cheat or servers may reject spoofed devices. Use at your own risk. / Anti-cheat atau server bisa menolak perangkat hasil spoof.
- Bootloop: safe mode or recovery, delete `/data/adb/modules/Spoofing`.

## Build
Push to `main` builds artifacts (module zip and `-debug` zip).
Push a tag for a Release: `v1.8-beta1` (with a dash) = pre-release, `v1.8` = full release and `update.json`.
The `-debug` zip logs to logcat: `logcat -s SpoofGame`.

## License / Lisensi
MIT, see LICENSE.
