@echo off
ctime -begin engine.ctm

rem eHa -O2 -Oi 
set CFLAGS=  -fopenmp -fpermissive -std=c++14 
rem -O2
rem --verbose
set LFLAGS= opengl32 
set LIBS=  -lgra -llua51 -lluajit -lSDL2main -lSDL2 -lSDL2_image -lSDL2_TTF  -lRaknet -lkernel32 -luser32 -lShell32
REM set INCLUDE= 
set ADDITIONAL= -I"../include" -I"../src/Imgui" -I"../sln/Raknet/Include" -I../Graphics/include

set OpenGl = -L"E:/sdk/Lib/10.0.15063.0/um/x86"
set libpath=-LI:/Dev/SDL/deps/lib
rem set LIBRARY_PATH ="I:/Dev/SDL/allegro vanhat/deps/lib"
rem set LD_LIBRARY_PATH="I:/Dev/SDL/allegro vanhat/deps/lib/"
rem kernel32.lib user32.lib  Shell32.lib
if not exist bin mkdir bin
pushd bin

echo %cd%
del *.pdb > NUL 2> NUL


rem simple preprocessor
rem I:\Dev\GCC\MinGW\bin\g++.exe %CFLAGS% -D_CRT_SECURE_NO_WARNINGS ..\src\simple_preprocessor.cpp -L"E:\sdk\Lib\10.0.15063.0\um\x86" -lOpenGl32

rem Box2D.lib  Box2D.lib

REM game dll
echo "WAITING FOR PDB ..." > lock.tmp
I:/Dev/GCC/MinGW/bin/g++.exe -g -DBUILDING_EXAMPLE_DLL -shared -o game.dll -I../Graphics/include %CFLAGS% -DMAIN_BUG=1 %ADDITIONAL% ../src/game.cpp %libpath% %LIBS% 
del lock.tmp

SETLOCAL EnableExtensions
set EXE=main.exe
FOR /F %%x IN ('tasklist /NH /FI "IMAGENAME eq %EXE%"') DO IF %%x == %EXE% goto FOUND
echo Not running
goto FIN
:FIN

REM W:\hotload\C-Hotloading-master\libs\gl3w\GL
REM platform exe

I:/Dev/GCC/MinGW/bin/g++.exe  %CFLAGS% %ADDITIONAL% -o gmain ../src/main.cpp  -lopengl32 %libpath%  %LIBS%

rem ..\libs\gl3w\GL\gl3w.c 
rem -Od
rem ..\src\Imgui\imgui*.cpp 
:FOUND
echo Running

REM Imgui tools.dll 

I:/Dev/GCC/MinGW/bin/g++.exe -g -shared %CFLAGS% -DMAIN_BUG=1 -DDLLCOMPILE=1 %ADDITIONAL% -o imgui.dll ..\src\imguiTools.cpp %libpath% %LIBS% 

popd
echo Done!
ctime -end engine.ctm