#!/system/bin/sh
# Per-app render downscale and FPS lock through Android Game Mode (cmd game).
# config.conf: DOWNSCALE="pkg=0.8,pkg2=0.6"  SMOOTH="pkg=60,pkg2=90"
# Overrides are lost on reboot, so this runs again from service.sh.
MODDIR=${0%/*}
[ -f "$MODDIR/config.conf" ] && . "$MODDIR/config.conf" || . "$MODDIR/config.default"
[ "$ENABLED" = "1" ] || { DOWNSCALE=""; SMOOTH=""; }
command -v cmd >/dev/null 2>&1 || exit 0
STATE="$MODDIR/downscale.applied"

val() { # val "<list>" <pkg> -> value of pkg=value
  _o=$IFS; IFS=,
  for _e in $1; do
    if [ "${_e%%=*}" = "$2" ]; then echo "${_e#*=}"; break; fi
  done
  IFS=$_o
}

PKGS=""
OLDIFS=$IFS; IFS=,
for e in $DOWNSCALE $SMOOTH; do
  p=${e%%=*}
  case "$p" in ""|*[!A-Za-z0-9._]*) continue ;; esac
  case " $PKGS " in *" $p "*) ;; *) PKGS="$PKGS $p" ;; esac
done
IFS=$OLDIFS

NEWSTATE=""
for p in $PKGS; do
  r=$(val "$DOWNSCALE" "$p"); f=$(val "$SMOOTH" "$p")
  DS=""; FP=""
  case "$r" in 0.[3-9]|0.[3-9][0-9]) DS="--downscale $r" ;; esac
  case "$f" in 30|45|60|90|120) FP="--fps $f" ;; esac
  [ -n "$DS$FP" ] || continue
  cmd game set --mode 2 $DS $FP "$p" >/dev/null 2>&1
  cmd game mode 2 "$p" >/dev/null 2>&1
  NEWSTATE="$NEWSTATE $p"
done

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
