# Spoofing Game



![Build](https://github.com/Arkael-Dev/Spoofing-Game/actions/workflows/build.yml/badge.svg)



Modul KernelSU / KernelSU Next / Magisk untuk spoof perangkat supaya game membuka opsi grafik dan FPS yang lebih tinggi. Bawaan: POCO F9 Ultra (`26077PC53G`).

## Fitur

- Spoof model, nama perangkat, brand, produsen, kode SoC, dan versi OpenGL ES
- Spoof `/proc/cpuinfo`
- Paksa refresh rate (60 / 90 / 120 / 144 Hz)
- WebUI di KSU Next untuk ganti perangkat tujuan tanpa edit file
- Banner modul

## Pasang

1. Unduh zip terbaru dari halaman Releases.
2. Flash lewat KernelSU Next atau Magisk.
3. Reboot.
4. Buka WebUI dari daftar modul, pilih perangkat tujuan, lalu tekan Simpan dan terapkan.
5. Tutup paksa game, lalu buka lagi.

## Konfigurasi

Pengaturan tersimpan di `/data/adb/modules/Spoofing/config.conf`.

    MARKET   nama perangkat (ro.product.marketname)
    MODEL    kode model (ro.product.model)
    BRAND    brand
    MFR      produsen
    SOC      kode SoC, contoh SM8850
    SOCNAME  nama SoC
    REFRESH  refresh rate dalam Hz
    ENABLED  1 = aktif, 0 = nonaktif

Menambah preset: edit objek `PRESETS` di `webroot/index.html`.

## Catatan

- Spoof GPU belum ada. Game membaca nama GPU langsung dari driver, jadi perlu modul Zygisk terpisah.
- GPU asli tetap dipakai. Preset grafik yang terbuka bisa membuat lag atau panas.
- Server game atau anti-cheat bisa menolak perangkat hasil spoof. Pakai dengan risiko sendiri.
- Bootloop: masuk mode aman atau recovery, lalu hapus `/data/adb/modules/Spoofing`.

## Build

Setiap push ke `main` menjalankan GitHub Actions dan menghasilkan zip sebagai artifact. Push tag `v*` untuk membuat Release otomatis.

## Lisensi

MIT. Lihat berkas LICENSE.
