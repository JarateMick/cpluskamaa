@echo off
ctime -begin engine.ctm

if not defined DEV_ENV_DIR (
 	call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
)
set DEV_ENV_DIR= ???

set CFLAGS= -Zi -nologo -Gm- -Oi -GR- -EHa -fp:fast -FC
set LFLAGS= -incremental:no opengl32.lib user32.lib kernel32.lib Shell32.lib 
set LIBS= allegro-debug.lib allegro_image-debug.lib allegro_primitives-debug.lib
REM set INCLUDE=
set ADDITIONAL= /I"../include" /I"../src/Imgui" 

if not exist ..\bin mkdir ..\bin
pushd ..\bin

del *.pdb > NUL 2> NUL

REM game dll
echo "WAITING FOR PDB ..." > lock.tmp
echo %cd%
cl %CFLAGS% -DMAIN_BUG=1 -DALLEGRO_STATIC_LINK=1  /I..\libs\gl3w %ADDITIONAL% ..src\game.cpp  -LD %LIBS% /link -incremental:no -opt:ref -PDB:game_%random%.pdb  /LIBPATH:"E:\BGFX\allegro2\ConsoleApplication1\packages\Allegro.5.2.2.1\build\native\v140\x64\lib" -EXPORT:Draw -EXPORT:Loop
del lock.tmp

REM W:\hotload\C-Hotloading-master\libs\gl3w\GL
REM platform exe
rem cl  %CFLAGS% /MD /I..\libs\gl3w %ADDITIONAL% ..\src\main.cpp ..\src\Imgui\imgui*.cpp   %LIBS% /link %LFLAGS% opengl32.lib /LIBPATH:"../deps/lib"
rem ..\libs\gl3w\GL\gl3w.c rem -Od

REM Imgui tools.dll 
REM cl %CFLAGS% /I..\libs\gl3w ..\src\imguiTools.cpp  -LD %LIBS% /link -PDB:game_%random%.pdb %LFLAGS% /LIBPATH:"W:\hotload\C-Hotloading-master\deps\lib" /SUBSYSTEM:CONSOLE

popd
echo Done!
ctime -end engine.ctm