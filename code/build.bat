
@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM XInput libraries - xinput.lib xinput9_1_0.lib

cl -nologo -MT -FmWin32_Handmade.map -WX -W4 -wd4201 -wd4100 -wd4189 -DHANDMADE_WIN32=1 -DHANDMADE_INTERNAL=1 -DHANDMADE_DEBUG=1 -std:c++17 -FAsc -Zi -Oi -GR- -EHa ..\code\Win32_Handmade.cpp user32.lib gdi32.lib 

popd
	
