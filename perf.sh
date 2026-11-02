#!/system/bin/sh
# Performance-while-gaming only. While a package listed in GPU_APPS is the
# foreground app, bumps CPU governor to "performance" and trims animation
# scale + swappiness for smoother play, then restores the original values
# the instant you leave that game. Nothing else from this file touches
# refresh rate, downscale/FPS (downscale.sh) or device/GPU spoof (apply.sh,
# the Zygisk module) — those are untouched and keep working as before.
MODDIR=${0%/*}
[ -f "$MODDIR/config.conf" ] && . "$MODDIR/config.conf" || . "$MODDIR/config.default"
[ "$ENABLED" = "1" ] && [ "$PERF_ENABLED" = "1" ] || exit 0

DATA=/data/adb/spoofinggame
mkdir -p "$DATA"
BASE="$DATA/perf.base"
PIDF="$DATA/perf.pid"
STATE="$DATA/perf.state"

isnum() { case "$1" in ""|*[!0-9]*) return 1 ;; esac; return 0; }
in_gpu_apps() { case ",$GPU_APPS," in *",$1,"*) return 0 ;; esac; return 1; }
gov_files() { echo /sys/devices/system/cpu/cpufreq/policy*/scaling_governor; }

backup() {
  [ -f "$BASE" ] && return 0
  : > "$BASE"
  for f in $(gov_files); do [ -r "$f" ] && echo "G|$f|$(cat "$f")" >> "$BASE"; done
  for k in window_animation_scale transition_animation_scale animator_duration_scale; do
    echo "S|$k|$(settings get global $k)" >> "$BASE"
  done
  sw=$(cat /proc/sys/vm/swappiness 2>/dev/null)
  isnum "$sw" && echo "V|$sw" >> "$BASE"
}

restore() {
  [ -f "$BASE" ] || return 0
  while IFS='|' read -r t a b; do
    case "$t" in
      G) [ -w "$a" ] && echo "$b" > "$a" 2>/dev/null ;;
      S) v=$b; [ "$v" = "null" ] && v=1; settings put global "$a" "$v" </dev/null ;;
      V) isnum "$b" && echo "$b" > /proc/sys/vm/swappiness 2>/dev/null ;;
    esac
  done < "$BASE"
}

boost() {
  backup
  for f in $(gov_files); do
    avail="${f%scaling_governor}scaling_available_governors"
    [ -r "$avail" ] || continue
    case " $(tr -d '[]' < "$avail") " in
      *" performance "*) [ -w "$f" ] && echo performance > "$f" 2>/dev/null ;;
    esac
  done
  for k in window_animation_scale transition_animation_scale animator_duration_scale; do
    settings put global "$k" 0.5 </dev/null
  done
  echo 10 > /proc/sys/vm/swappiness 2>/dev/null
}

fg_pkg() {
  l=$(dumpsys window 2>/dev/null | grep -m1 mCurrentFocus)
  [ -z "$l" ] && l=$(dumpsys activity activities 2>/dev/null | grep -m1 -E 'topResumedActivity|ResumedActivity')
  echo "$l" | sed -n 's/.* \([A-Za-z0-9_.]*\)\/[^ ]*.*/\1/p'
}

alive() {
  [ -f "$PIDF" ] || return 1
  p=$(cat "$PIDF" 2>/dev/null); isnum "$p" || return 1
  kill -0 "$p" 2>/dev/null || return 1
  [ "$p" = "$$" ] && return 0
  tr '\0' ' ' < "/proc/$p/cmdline" 2>/dev/null | grep -q perf.sh
}

if alive && [ "$(cat "$PIDF")" != "$$" ]; then exit 0; fi
echo $$ > "$PIDF"
rm -f "$STATE"

key=""
while :; do
  [ -f "$MODDIR/config.conf" ] && . "$MODDIR/config.conf"
  if [ "$ENABLED" = "1" ] && [ "$PERF_ENABLED" = "1" ]; then
    pkg=$(fg_pkg)
    if [ -n "$pkg" ] && in_gpu_apps "$pkg"; then want="on:$pkg"; else want="off"; fi
  else
    want="off"
  fi
  if [ "$want" != "$key" ]; then
    case "$want" in
      on:*) boost ;;
      off) restore ;;
    esac
    key="$want"
    echo "$key" > "$STATE"
  fi
  sleep 3
done
