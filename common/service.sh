#!/system/bin/sh
MODDIR=${0%/*}
[ -f "$MODDIR/config.conf" ] && . "$MODDIR/config.conf" || . "$MODDIR/config.default"
[ "$ENABLED" = "1" ] || exit 0
sh "$MODDIR/UNLOCKER" &
until [ "$(getprop sys.boot_completed)" = "1" ]; do sleep 1; done
sleep 5
settings put system min_refresh_rate "$REFRESH"
settings put system peak_refresh_rate "$REFRESH"
