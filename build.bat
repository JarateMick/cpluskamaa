@echo off
ctime -begin engine.ctm

if not defined DEV_ENV_DIR (
	rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
    call "H:\Visual Studio\VC\vcvarsall.bat" x64 
	rem call "I:\VisualStudio\VC\Auxiliary\Build\vcvarsall.bat" x64
)
set DEV_ENV_DIR= ???

rem eHa -O2
set CFLAGS= -Zi -nologo -EHs -Gm- -Oi -GR-  -fp:fast -wd4311 -wd4312 
set LFLAGS= -incremental:no opengl32.lib  
set LIBS= lua51.lib luajit.lib SDL2.lib SDL2main.lib SDL2_image.lib SDL2_TTF.lib Raknet.lib
REM set INCLUDE=
set ADDITIONAL= /I"../include" /I"../src/Imgui" /I"../sln/Raknet/Include"

rem kernel32.lib user32.lib  Shell32.lib
if not exist bin mkdir bin
pushd bin

echo %cd%
del *.pdb > NUL 2> NUL


rem simple preprocessor
cl %CFLAGS% -D_CRT_SECURE_NO_WARNINGS ..\src\simple_preprocessor.cpp /link %LFLAGS%

rem Box2D.lib  Box2D.lib

REM game dll
echo "WAITING FOR PDB ..." > lock.tmp
cl %CFLAGS% -DMAIN_BUG=1 -DALLEGRO_STATIC_LINK=1 %ADDITIONAL%  ..\src\game.cpp /MD -LD %LIBS%   /link -incremental:no -opt:ref -PDB:game_%random%.pdb  /LIBPATH:"../deps/lib" -EXPORT:Draw -EXPORT:Loop
del lock.tmp

SETLOCAL EnableExtensions
set EXE=main.exe
FOR /F %%x IN ('tasklist /NH /FI "IMAGENAME eq %EXE%"') DO IF %%x == %EXE% goto FOUND
echo Not running
goto FIN
:FIN

REM W:\hotload\C-Hotloading-master\libs\gl3w\GL
REM platform exe
cl %CFLAGS% /MD /I..\libs\gl3w %ADDITIONAL% ..\src\main.cpp   %LIBS%  /link %LFLAGS% opengl32.lib /LIBPATH:"../deps/lib" /SUBSYSTEM:CONSOLE
rem ..\libs\gl3w\GL\gl3w.c 
rem -Od
rem ..\src\Imgui\imgui*.cpp 
:FOUND
echo Running

REM Imgui tools.dll 
cl %CFLAGS% -DDLLCOMPILE=1 %ADDITIONAL% ..\src\imguiTools.cpp /MD  -LD %LIBS% /link -incremental:no -PDB:imguiTools_%random%.pdb /LIBPATH:"../deps/lib" -EXPORT:Imgui

popd
echo Done!
ctime -end engine.ctm