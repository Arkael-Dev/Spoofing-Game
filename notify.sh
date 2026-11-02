#!/system/bin/sh
# Posts one status notification (module icon + a one-line summary of what
# just got applied). Called by the WebUI right after Save & apply. Not run
# at boot and not a recurring/daemon notification — just a one-shot
# confirmation so you can see the settings actually took effect.
# Usage: notify.sh [id|en]
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

# Language: 1st arg (WebUI passes its current language), else UI_LANG saved by
# the WebUI, else follow the system locale (id/in -> Indonesian, otherwise English).
L=${1:-$UI_LANG}
case "$L" in
  id|en) ;;
  *)
    loc=$(getprop persist.sys.locale 2>/dev/null)
    [ -n "$loc" ] || loc=$(getprop ro.product.locale 2>/dev/null)
    case "$loc" in id*|in*) L=id ;; *) L=en ;; esac
    ;;
esac

if [ "$L" = "id" ]; then
  S_ON="Spoofing Game aktif"; S_OFF="Spoofing Game nonaktif"; S_OFFMSG="Spoof dimatikan"
  S_PERGAME="spoof per-game"; S_GLOBAL="spoof global"; S_GAMES="Game"; S_PERF="Perf nyala"
else
  S_ON="Spoofing Game active"; S_OFF="Spoofing Game off"; S_OFFMSG="Spoof turned off"
  S_PERGAME="per-game spoof"; S_GLOBAL="global spoof"; S_GAMES="Games"; S_PERF="Perf on"
fi

parts=""
add() { parts="${parts:+$parts | }$1"; }
if [ "$ENABLED" = "1" ]; then
  title="$S_ON"
  add "${REFRESH:-?} Hz"
  [ "$GPU_ENABLED" = "1" ] && add "GPU ${GPU_RENDERER:-?}"
  if [ "$DEV_MODE" = "game" ]; then add "$S_PERGAME"; else add "$S_GLOBAL"; fi
  add "$S_GAMES: $ngames"
  [ "$PERF_ENABLED" = "1" ] && add "$S_PERF"
else
  title="$S_OFF"
  parts="$S_OFFMSG"
fi

TO=""; command -v timeout >/dev/null 2>&1 && TO="timeout 10"
n1="cmd notification post -t '$title' -i file://$ICON_T 'SpoofingGame' '$parts'"
n2="cmd notification post -t '$title' 'SpoofingGame' '$parts'"
$TO su -lp 2000 -c "$n1" >/dev/null 2>&1 </dev/null && exit 0
$TO sh -c "$n1" >/dev/null 2>&1 </dev/null && exit 0
$TO su -lp 2000 -c "$n2" >/dev/null 2>&1 </dev/null && exit 0
$TO sh -c "$n2" >/dev/null 2>&1 </dev/null
exit 0
