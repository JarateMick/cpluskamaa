@echo off
pushd W:\hotload\C-Hotloading-master

if not defined DEV_ENV_DIR (
	call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
)
set DEV_ENV_DIR= ???

rem call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
set CFLAGS= -Zi -nologo 
set LFLAGS= -incremental:no user32.lib kernel32.lib Shell32.lib
set LIBS= SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib SDL2_mixer.lib
REM set INCLUDE=

if not exist .\bin mkdir .\bin
pushd .\bin

del *.pdb > NUL 2> NUL

REM game dll
echo "WAITING FOR PDB ..." > lock.tmp
cl %CFLAGS% ..\src\game.cpp -LD %LIBS% /link -PDB:game_%random%.pdb %LFLAGS% /LIBPATH:"E:\Programmings\C\hotload\C-Hotloading-master\deps\lib" /SUBSYSTEM:CONSOLE
del lock.tmp

REM platform exe
rem cl  %CFLAGS%  ..\src\main.cpp %LIBS% /link %LFLAGS% /LIBPATH:"E:\Programmings\C\hotload\C-Hotloading-master\deps\lib" /SUBSYSTEM:CONSOLE

popd
popd
echo Done!
pause