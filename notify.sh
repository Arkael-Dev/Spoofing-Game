#!/system/bin/sh
# Posts one status notification (module icon + a one-line summary of what
# just got applied). Called by the WebUI right after Save & apply. Not run
# at boot and not a recurring/daemon notification — just a one-shot
# confirmation so you can see the settings actually took effect.
MODDIR=${0%/*}
[ -f "$MODDIR/config.conf" ] && . "$MODDIR/config.conf" || . "$MODDIR/config.default"

ICON_T=/data/local/tmp/spoofinggame_icon.png
mkdir -p /data/local/tmp 2>/dev/null
cp -f "$MODDIR/webroot/icon.png" "$ICON_T" 2>/dev/null
chmod 644 "$ICON_T" 2>/dev/null

ngames=0
OLDIFS=$IFS; IFS=,
for p in $GPU_APPS; do [ -n "$p" ] && ngames=$((ngames + 1)); done
IFS=$OLDIFS

parts=""
add() { parts="${parts:+$parts | }$1"; }
if [ "$ENABLED" = "1" ]; then
  title="Spoofing Game aktif"
  add "${REFRESH:-?} Hz"
  [ "$GPU_ENABLED" = "1" ] && add "GPU ${GPU_RENDERER:-?}"
  if [ "$DEV_MODE" = "game" ]; then add "spoof per-game"; else add "spoof global"; fi
  add "Game: $ngames"
  [ "$PERF_ENABLED" = "1" ] && add "Perf nyala"
else
  title="Spoofing Game nonaktif"
  parts="Spoof dimatikan"
fi

TO=""; command -v timeout >/dev/null 2>&1 && TO="timeout 10"
n1="cmd notification post -t '$title' -i file://$ICON_T 'SpoofingGame' '$parts'"
n2="cmd notification post -t '$title' 'SpoofingGame' '$parts'"
$TO su -lp 2000 -c "$n1" >/dev/null 2>&1 </dev/null && exit 0
$TO sh -c "$n1" >/dev/null 2>&1 </dev/null && exit 0
$TO su -lp 2000 -c "$n2" >/dev/null 2>&1 </dev/null && exit 0
$TO sh -c "$n2" >/dev/null 2>&1 </dev/null
exit 0
