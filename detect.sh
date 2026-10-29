#!/system/bin/sh
# Prints installed known games (G:pkg) and test tools (T:pkg), one per line.
# With --merge, known games found are added to GPU_APPS in config.conf.
MODDIR=${0%/*}
GAMES="com.mobile.legends com.tencent.ig com.pubg.krmobile com.vng.pubgmobile com.rekoo.pubgm com.dts.freefireth com.dts.freefiremax com.miHoYo.GenshinImpact com.HoYoverse.hkrpgoversea com.HoYoverse.Nap com.activision.callofduty.shooter com.activision.callofduty.warzone com.garena.game.codm com.kurogame.wutheringwaves.global com.supercell.clashofclans com.supercell.brawlstars com.supercell.clashroyale com.riotgames.league.wildrift com.riotgames.league.teamfighttactics com.proximabeta.mf.uamo com.levelinfinite.sgameGlobal com.ea.gp.fifamobile com.ea.gp.apexlegendsmobilefps com.gameloft.android.ANMP.GloftA9HM com.roblox.client com.mojang.minecraftpe com.netmarble.sololv com.nianticlabs.pokemongo jp.konami.pesam"
TOOLS="flar2.devcheck com.cpuid.cpu_z com.finalwire.aida64 com.antutu.ABenchMark com.primatelabs.geekbench6 com.futuremark.dmandroid.application"
INST=" $(pm list packages 2>/dev/null | sed 's/^package://' | tr '\n\r' '  ') "
if [ "$1" = "--merge" ]; then
  CONF="$MODDIR/config.conf"
  [ -f "$CONF" ] || exit 0
  CUR=$(sed -n 's/^GPU_APPS="\(.*\)"$/\1/p' "$CONF")
  NEW="$CUR"
  for p in $GAMES; do
    case "$INST" in *" $p "*)
      case ",$NEW," in *",$p,"*) ;; *) NEW="${NEW:+$NEW,}$p" ;; esac ;;
    esac
  done
  if [ "$NEW" != "$CUR" ]; then
    if grep -q '^GPU_APPS=' "$CONF"; then
      sed -i "s|^GPU_APPS=.*|GPU_APPS=\"$NEW\"|" "$CONF"
    else
      echo "GPU_APPS=\"$NEW\"" >> "$CONF"
    fi
  fi
  exit 0
fi
for p in $GAMES; do case "$INST" in *" $p "*) echo "G:$p" ;; esac; done
for p in $TOOLS; do case "$INST" in *" $p "*) echo "T:$p" ;; esac; done
