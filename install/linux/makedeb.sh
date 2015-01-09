#sudo apt-get install autotools-dev fakeroot dh-make build-essential

PACKAGE_NAME=cellmotus
PACKAGE_VER=0.9
PACKAGE_DIR=/tmp/${PACKAGE_NAME}-${PACKAGE_VER}

rm -rf /tmp/cellmotus*
mkdir ${PACKAGE_DIR}
cp -rf ../../src ${PACKAGE_DIR}
cp -rf ../../assets ${PACKAGE_DIR}
cp -rf icon.png ${PACKAGE_DIR}/assets 
cp -rf ../../install ${PACKAGE_DIR}
cp -rf ../../CMakeLists.txt ${PACKAGE_DIR}

cd ${PACKAGE_DIR}
#mkdir ${PACKAGE_DIR}/build
#cd ${PACKAGE_DIR}/build
cmake ./ -DDEBUG_MODE=0
#find . -name "CMake*" -type f -delete
#cd ${PACKAGE_DIR}
dh_make --createorig --single
cp ${PACKAGE_DIR}/install/linux/control ${PACKAGE_DIR}/debian/control
cp ${PACKAGE_DIR}/install/linux/copyright ${PACKAGE_DIR}/debian/copyright
cp ${PACKAGE_DIR}/install/linux/changelog ${PACKAGE_DIR}/debian/changelog
cp ${PACKAGE_DIR}/install/linux/watch ${PACKAGE_DIR}/debian/watch

cp ${PACKAGE_DIR}/install/linux/CellMotus.1 ${PACKAGE_DIR}/debian/CellMotus.1
cp ${PACKAGE_DIR}/install/linux/cellmotus.manpages ${PACKAGE_DIR}/debian/cellmotus.manpages

rm ${PACKAGE_DIR}/debian/docs
rm ${PACKAGE_DIR}/debian/README.Debian
rm ${PACKAGE_DIR}/debian/README.source
rm -rf ${PACKAGE_DIR}/debian/*.ex
rm -rf ${PACKAGE_DIR}/debian/*.EX
rm -rf ${PACKAGE_DIR}/debian/EX.*

dpkg-buildpackage -rfakeroot

