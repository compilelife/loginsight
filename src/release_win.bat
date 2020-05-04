cd ..\build
rmdir /s/q win
mkdir win
cd win
qmake ..\..\src\loginsight.pro -spec win32-g++ "CONFIG+=qtquickcompiler"
mingw32-make.exe -j2
windeployqt release\loginsight.exe --dir ..\bin\win
copy release\loginsight.exe ..\bin\win\
