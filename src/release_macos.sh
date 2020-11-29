./create_iconset.sh
mkdir -p ../build
cd ../build
qmake ../src/loginsight.pro -spec macx-clang CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -j4
macdeployqt loginsight.app -dmg
mkdir artifact
mv loginsight.dmg artifact
