@echo off
cls
echo Compiling SDL project...

g++ main.cpp -o myapp.exe -mconsole -IC:\msys64\mingw64\include\SDL2 -LC:\msys64\mingw64\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lcomdlg32

myapp.exe
pause

