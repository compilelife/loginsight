mkdir ..\build
cd ..\build
qmake ..\src\loginsight.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
nmake
mkdir artifact
windeployqt release\loginsight.exe --dir artifact
copy release\loginsight.exe artifact\
dir artifact
