#!/system/bin/sh
MODDIR=${0%/*}
[ -f "$MODDIR/config.conf" ] && . "$MODDIR/config.conf" || . "$MODDIR/config.default"
[ "$ENABLED" = "1" ] || exit 0
rp() { resetprop -n "$1" "$2"; }
for P in "" system. vendor. odm. product. system_ext.; do
  rp ro.product.${P}model "$MODEL"
  rp ro.product.${P}marketname "$MARKET"
  rp ro.product.${P}brand "$BRAND"
  rp ro.product.${P}manufacturer "$MFR"
done
rp ro.soc.model "$SOC"
rp ro.vendor.soc.model.part_name "$SOC"
rp ro.vendor.soc.model.external_name "$SOCNAME"
rp ro.chipset "$SOC"
rp ro.opengles.version "$GLES"
