#!/bin/bash

dir=${0%/*}
if [ -d "$dir" ]; then
  cd "$dir"
fi
echo "Working Directory : $dir"

PNAME="cellmotus-0.9-iphoneos_arm"
rm -rf "${PNAME}"
rm -rf "${PNAME}.deb"
mkdir "${PNAME}"
mkdir "${PNAME}/Applications"
mkdir "${PNAME}/DEBIAN"
cp control "${PNAME}/DEBIAN"
cp -rf ../../bin/ios/CellMotus.app "${PNAME}/Applications"
PBIN="${PNAME}/Applications/CellMotus.app/CellMotus"
ldid -S "${PBIN}"
codesign -dv "${PBIN}"
sleep 2
./dpkg-deb-fat -b "${PNAME}"
ls -la "${PNAME}.deb"
rm -rf "${PNAME}"
#scp -r "${PNAME}.deb" root@192.168.1.22:~/
exit
