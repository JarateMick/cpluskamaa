@echo off
set files=Camera2D.cpp DebugRenderer.cpp glad.c GlSLProgram.cpp ImageLoader.cpp IOManager.cpp picoPNG.cpp ResourceManager.cpp Sprite.cpp SpriteBatch.cpp SpriteFont.cpp TextureCache.cpp
set include=-I"I:/Dev/SDL/allegro vanhat/Include" -I"I:/Dev/SDL/allegro vanhat/Graphics/include"
set libs=-lSDL2 
rem -lSDL2_ttf
set libpath=-L"I:/Dev/SDL/allegro vanhat/deps/lib"
set flags=
C:\MinGW\MinGW\bin\g++.exe -c -std=c++14 %files%  %include% %libpath% %libs%
C:\MinGW\MinGW\bin\ar.exe rcs g++Graphics.a Camera2D.o DebugRenderer.o glad.o GlSLProgram.o ImageLoader.o IOManager.o picoPNG.o ResourceManager.o Sprite.o SpriteBatch.o SpriteFont.o TextureCache.o

rem -Wl,--whole-archive libAlgatorc.a -Wl,--no-whole-archive