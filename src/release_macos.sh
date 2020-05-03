./create_iconset.sh
cd ../build
rm -rf macos
mkdir macos
cd macos
qmake ../../src/loginsight.pro -spec macx-clang CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -j4
macdeployqt loginsight.app -dmg
mv loginsight.dmg ../bin/
