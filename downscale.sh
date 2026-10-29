#!/system/bin/sh
# Per-app render downscale through Android Game Mode (cmd game).
# DOWNSCALE="pkg=0.8,pkg2=0.6" in config.conf. Overrides are lost on reboot,
# so this runs again from service.sh.
MODDIR=${0%/*}
[ -f "$MODDIR/config.conf" ] && . "$MODDIR/config.conf" || . "$MODDIR/config.default"
[ "$ENABLED" = "1" ] || DOWNSCALE=""
command -v cmd >/dev/null 2>&1 || exit 0
STATE="$MODDIR/downscale.applied"
NEWSTATE=""
OLDIFS=$IFS; IFS=,
for e in $DOWNSCALE; do
  p=${e%%=*}; r=${e#*=}
  case "$p" in ""|*[!A-Za-z0-9._]*) continue ;; esac
  case "$r" in 0.[3-9]|0.[3-9][0-9]) ;; *) continue ;; esac
  cmd game set --mode 2 --downscale "$r" "$p" >/dev/null 2>&1
  cmd game mode 2 "$p" >/dev/null 2>&1
  NEWSTATE="$NEWSTATE $p"
done
IFS=$OLDIFS
if [ -f "$STATE" ]; then
  for p in $(cat "$STATE"); do
    case " $NEWSTATE " in *" $p "*) ;; *)
      cmd game reset "$p" >/dev/null 2>&1
      cmd game mode 1 "$p" >/dev/null 2>&1 ;;
    esac
  done
fi
echo "$NEWSTATE" > "$STATE"
exit 0
