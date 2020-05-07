mkdir -p ../build
cd ../build
qmake ../src/loginsight.pro -spec linux-g++ CONFIG+=qtquickcompiler
make -j4
mkdir -p appdir/usr/bin
cp loginsight appdir/usr/bin/
cp ../src/img/logo.png appdir/
cp ../src/loginsight.desktop appdir/
mkdir appdir/usr/lib
cp /usr/lib/x86_64-linux-gnu/libFcitxQt5DBusAddons.so* appdir/usr/lib/
cp /usr/lib/x86_64-linux-gnu/libFcitxQt5WidgetsAddons.so* appdir/usr/lib/
../tool/linuxdeployqt appdir/usr/bin/loginsight -appimage
mkdir artifact
mv loginsight-*.AppImage artifact/loginsight.AppImage

