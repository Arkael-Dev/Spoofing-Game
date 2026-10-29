SKIPMOUNT=false
PROPFILE=false
POSTFSDATA=true
LATESTARTSERVICE=true

REPLACE="
"

print_modname() {
  ui_print "*******************************"
  ui_print "        Spoofing Game"
  ui_print "          Arkael-Dev"
  ui_print "*******************************"
  ui_print " Kernel    : $(uname -r)"
  ui_print " Model     : $(getprop ro.product.model)"
  ui_print " Android   : $(getprop ro.system.build.version.release)"
  ui_print " GPU spoof needs Zygisk / spoof GPU butuh Zygisk"
  ui_print " Open WebUI to configure / Buka WebUI untuk atur"
}

on_install() {
  ui_print "- Extracting module files"
  unzip -o "$ZIPFILE" 'UNLOCKER' 'apply.sh' 'config.default' 'banner.png' 'webroot/*' 'zygisk/*' -d $MODPATH >&2
  OLD=/data/adb/modules/Spoofing/config.conf
  if [ -f "$OLD" ]; then
    cp -f "$OLD" $MODPATH/config.conf
    while IFS= read -r l; do
      k=${l%%=*}
      grep -q "^$k=" $MODPATH/config.conf || echo "$l" >> $MODPATH/config.conf
    done < $MODPATH/config.default
    ui_print "- Config kept / Config lama dipertahankan"
  else
    cp -f $MODPATH/config.default $MODPATH/config.conf
  fi
}

set_permissions() {
  set_perm_recursive $MODPATH 0 0 0755 0644
  set_perm $MODPATH/UNLOCKER 0 0 0755
  set_perm $MODPATH/apply.sh 0 0 0755
  set_perm $MODPATH/service.sh 0 0 0755
  set_perm $MODPATH/post-fs-data.sh 0 0 0755
}
