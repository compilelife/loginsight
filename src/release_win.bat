mkdir ..\build
cd ..\build
qmake ..\loginsight.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
nmake
mkdir artifact
dir src
windeployqt src\release\loginsight.exe --dir artifact
copy src\release\loginsight.exe artifact\
