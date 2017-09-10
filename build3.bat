@echo off
ctime -begin engine.ctm

rem if not defined DEV_ENV_DIR (
rem 	call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
rem )
rem set DEV_ENV_DIR= ???

set CFLAGS= -Z7 -nologo -Gm- -Oi -GR- -EHa 
set LFLAGS= -incremental:no opengl32.lib user32.lib kernel32.lib Shell32.lib 
set LIBS= SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib SDL2_mixer.lib
REM set INCLUDE=

cd..
if not exist .\bin mkdir .\bin
pushd .\bin

del *.pdb > NUL 2> NUL

REM game dll
echo "WAITING FOR PDB ..." > lock.tmp
rem cl %CFLAGS% /I..\libs\gl3w ..\src\game.cpp ..\libs\gl3w\GL\gl3w.c  -LD %LIBS% /link -PDB:game_%random%.pdb %LFLAGS% /LIBPATH:"W:\hotload\C-Hotloading-master\deps\lib" /SUBSYSTEM:CONSOLE
del lock.tmp

REM W:\hotload\C-Hotloading-master\libs\gl3w\GL
REM platform exe
echo %cd%
cl  %CFLAGS% /MD /I..\libs\gl3w  ..\src\main.cpp ..\libs\gl3w\GL\gl3w.c  %LIBS% /link %LFLAGS% opengl32.lib /LIBPATH:"W:\hotload\C-Hotloading-master\deps\lib" /SUBSYSTEM:CONSOLE

popd

echo Done!

ctime -end engine.ctm