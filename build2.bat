@echo off
ctime -begin engine.ctm

REM if not defined DEV_ENV_DIR (
REM 	rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
REM     call "H:\Visual Studio\VC\vcvarsall.bat" x64 
REM 	rem call "I:\VisualStudio\VC\Auxiliary\Build\vcvarsall.bat" x64
REM )
REM set DEV_ENV_DIR= ???

rem eHa -O2 -Oi
set CFLAGS= -Zi -nologo -EHs -Gm-  -GR-  -fp:fast -wd4311 -wd4312 
set LFLAGS= -incremental:no opengl32.lib  
set LIBS= lua51.lib luajit.lib SDL2.lib SDL2main.lib SDL2_image.lib SDL2_TTF.lib Raknet.lib Graphics.lib
REM set INCLUDE=
set ADDITIONAL= /I"../include" /I"../src/Imgui" /I"../sln/Raknet/Include" /I"../Graphics/include"

rem kernel32.lib user32.lib  Shell32.lib
if not exist bin mkdir bin
pushd bin

echo %cd%
del *.pdb > NUL 2> NUL

rem simple preprocessor
cl %CFLAGS% -D_CRT_SECURE_NO_WARNINGS ..\src\simple_preprocessor.cpp /link %LFLAGS%
rem use preprocessor
pushd ..\src
call ..\bin\simple_preprocessor.exe > handmade_generated.h
popd


REM game dll
echo "WAITING FOR PDB ..." > lock.tmp
cl %CFLAGS% -DMAIN_BUG=1 -DALLEGRO_STATIC_LINK=1 %ADDITIONAL%  ..\src\game.cpp /MD -LD %LIBS%   /link -incremental:no -opt:ref -PDB:game_%random%.pdb  /LIBPATH:"../deps/lib" -EXPORT:Draw -EXPORT:Loop
del lock.tmp

popd
echo Done!
ctime -end engine.ctm