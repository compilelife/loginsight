cd ../build
rm -rf linux
mkdir linux
cd linux
qmake ../../src/loginsight.pro -spec linux-g++ CONFIG+=qtquickcompiler
make -j4
