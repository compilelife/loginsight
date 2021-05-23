./create_iconset.sh
mkdir -p ../build
cd ../build
qmake ../loginsight.pro -spec macx-clang CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -j4
macdeployqt src/loginsight.app -dmg
mkdir artifact
mv src/loginsight.dmg artifact
