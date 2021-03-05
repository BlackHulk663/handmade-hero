
@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM XInput libraries - xinput.lib xinput9_1_0.lib

set CompilerFlags=  -nologo -Od -MT -FmWin32_Handmade.map -WX -W4 -wd4201 -wd4100 -wd4189 -std:c++17 -FAsc -Zi -Oi -GR- -EHa

set Macros= -DHANDMADE_WIN32=1 -DHANDMADE_INTERNAL=1 -DHANDMADE_DEBUG=1

cl %CompilerFlags% %Macros% ..\code\Win32_Handmade.cpp user32.lib gdi32.lib winmm.lib

popd
	
