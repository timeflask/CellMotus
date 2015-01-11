#!/bin/bash

dir=${0%/*}
if [ -d "$dir" ]; then
  cd "$dir"
fi
echo "Working Directory : $dir"

APP_NAME="CellMotus"
APP_BUNDLE_SOURCE="../../bin/mac/${APP_NAME}.app"
APP_BUNDLE="${APP_NAME}.app"
APP_EXE="${APP_BUNDLE}/Contents/MacOS/${APP_NAME}"
VERSION="0.9"
VOL_NAME="${APP_NAME}"
DMG_BACKGROUND_IMG_SOURCE="../../res/dmgbg.png"
DMG_BACKGROUND_IMG="dmgbg.png"
DMG_TMP="${VOL_NAME}-temp.dmg"
DMG_FINAL="cellmotus-${VERSION}-darwin64.dmg"         
STAGING_DIR="./release"

rm ${DMG_BACKGROUND_IMG}
rm ${DMG_BACKGROUND_IMG%.*}"_dpifix."${DMG_BACKGROUND_IMG##*.}
cp ${DMG_BACKGROUND_IMG_SOURCE} ${DMG_BACKGROUND_IMG}

_BACKGROUND_IMAGE_DPI_H=`sips -g dpiHeight ${DMG_BACKGROUND_IMG} | grep -Eo '[0-9]+\.[0-9]+'`
_BACKGROUND_IMAGE_DPI_W=`sips -g dpiWidth ${DMG_BACKGROUND_IMG} | grep -Eo '[0-9]+\.[0-9]+'`
 
if [ $(echo " $_BACKGROUND_IMAGE_DPI_H != 72.0 " | bc) -eq 1 -o $(echo " $_BACKGROUND_IMAGE_DPI_W != 72.0 " | bc) -eq 1 ]; then
   echo "WARNING: The background image's DPI is not 72.  This will result in distorted backgrounds on Mac OS X 10.7+."
   echo "         I will convert it to 72 DPI for you."
   
   _DMG_BACKGROUND_TMP="${DMG_BACKGROUND_IMG%.*}"_dpifix."${DMG_BACKGROUND_IMG##*.}"
 
   sips -s dpiWidth 72 -s dpiHeight 72 ${DMG_BACKGROUND_IMG} --out ${_DMG_BACKGROUND_TMP}
   
   DMG_BACKGROUND_IMG2="${DMG_BACKGROUND_IMG}"
   DMG_BACKGROUND_IMG="${_DMG_BACKGROUND_TMP}"
fi

rm -rf "${STAGING_DIR}" "${DMG_TMP}" "${DMG_FINAL}"
mkdir -p "${STAGING_DIR}"
cp -rpf "${APP_BUNDLE_SOURCE}" "${STAGING_DIR}"

pushd "${STAGING_DIR}" 
 echo "Stripping ${APP_EXE}..."
 strip -u -r "${APP_EXE}"
 
 if hash upx 2>/dev/null; then
    echo "Compressing (UPX) ${APP_EXE}..."
    upx -9 "${APP_EXE}"
 fi
popd

export LC_NUMERIC="en_US.UTF-8"

SIZE=`du -sh "${STAGING_DIR}" | sed 's/\([0-9]*\)M\(.*\)/\1/'`
SIZE=`echo "${SIZE} + 1.0" | bc | awk '{print int($1+0.5)}'`
 
if [ $? -ne 0 ]; then
   echo "Error: Cannot compute size of staging dir"
   exit
fi

hdiutil create -srcfolder "${STAGING_DIR}" -volname "${VOL_NAME}" -fs HFS+ \
      -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${SIZE}M "${DMG_TMP}"
 
echo "Created DMG: ${DMG_TMP}"
 
DEVICE=$(hdiutil attach -readwrite -noverify "${DMG_TMP}" | \
         egrep '^/dev/' | sed 1q | awk '{print $1}')
sleep 3

echo "Add link to /Applications"
pushd /Volumes/"${VOL_NAME}"
ln -s /Applications
popd
 
mkdir /Volumes/"${VOL_NAME}"/.background
cp "${DMG_BACKGROUND_IMG}" /Volumes/"${VOL_NAME}"/.background/
echo '
   tell application "Finder"
     tell disk "'${VOL_NAME}'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {400, 100, 920, 440}
           set viewOptions to the icon view options of container window
           set arrangement of viewOptions to not arranged
           set icon size of viewOptions to 72
           set background picture of viewOptions to file ".background:'${DMG_BACKGROUND_IMG}'"
           set position of item "'${APP_NAME}'.app" of container window to {160, 205}
           set position of item "Applications" of container window to {360, 205}
           close
           open
           update without registering applications
           delay 2
     end tell
   end tell
' | osascript
 
sync

hdiutil detach "${DEVICE}"
hdiutil convert "${DMG_TMP}" -format UDZO -imagekey zlib-level=9 -o "${DMG_FINAL}"
rm -rf "${DMG_TMP}"
rm -rf "${STAGING_DIR}" "${DMG_BACKGROUND_IMG}" "${DMG_BACKGROUND_IMG2}" 
exit

