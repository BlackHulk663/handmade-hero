#include <math.h>
#include <stdint.h>

#define local_persist	 static
#define global_variable	 static
#define internal		 static

typedef uint8_t  uint8;
typedef uint16_t uint16p;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float 	f32;
typedef double 	f64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

#define PI32 3.141592653589793238462f

#include "Handmade.cpp"

#include <malloc.h>
#include <windows.h>
#include <xinput.h>
#include <dsound.h>

#include "Win32_Handmade.h"

//====================================================================================
// NOTE(afb) :: XInputGetState Loading
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
X_INPUT_GET_STATE(XinputGetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
typedef X_INPUT_GET_STATE(x_input_get_state);
global_variable x_input_get_state* XInputGetState_ = XinputGetStateStub;

// NOTE(afb) :: XInputSetState Loading
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
X_INPUT_SET_STATE(XinputSetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
typedef X_INPUT_SET_STATE(x_input_set_state);
global_variable x_input_set_state* XInputSetState_ = XinputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter)
DIRECT_SOUND_CREATE(DirectSoundCreateStub)
{
	return(DSERR_NODRIVER);
}
typedef DIRECT_SOUND_CREATE(direct_sound_create);
global_variable direct_sound_create* DirectSoundCreate_ = DirectSoundCreateStub;
#define DirectSoundCreate DirectSoundCreate_
//====================================================================================


global_variable Win32DIBBuffer GlobalBackBuffer;
global_variable bool Running;
global_variable	LPDIRECTSOUNDBUFFER GlobalSecondarySoundBuffer;


internal void
Win32InitDirectSound(HWND windowHandle, int32 samplesPerSecond, int32 bufferSize)
{
	// NOTE(afb) :: Load DirectSound Library
	HINSTANCE dSoundLibrary = LoadLibraryA("dsound.dll");
	if(dSoundLibrary)
	{
		DirectSoundCreate = (direct_sound_create*)GetProcAddress(dSoundLibrary, "DirectSoundCreate");

		LPDIRECTSOUND directSound = {};
		if(DirectSoundCreate && (SUCCEEDED(DirectSoundCreate(0, &directSound, 0))))
		{
			WAVEFORMATEX waveFormat = {};
			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nChannels = 2;
			waveFormat.nSamplesPerSec = samplesPerSecond;
			waveFormat.wBitsPerSample = 16;
			waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample)/8;
			waveFormat.nAvgBytesPerSec = samplesPerSecond *
												waveFormat.nBlockAlign;

			// NOTE(afb) :: Create Direct Sound object.
			if(SUCCEEDED(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY)))
			{
				// NOTE(afb) :: Create primary buffer


				DSBUFFERDESC bufferDescription = {};
				bufferDescription.dwSize = sizeof(DSBUFFERDESC);
				bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
				LPDIRECTSOUNDBUFFER primaryBuffer;

				if(SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
				{
					if(SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)))
					{
						// NOTE(afb) :: Format set
						OutputDebugStringA("Primary buffer created\n");
					}
					else
					{
						// TODO(afb) :: Diagnostics
					}
				}
				else
				{
				// TODO(afb) :: Diagnostics

				}
			}
			else
			{
				// TODO(afb) :: Diagnostics
			}

			DSBUFFERDESC bufferDescription = {};
			bufferDescription.dwSize = sizeof(DSBUFFERDESC);
			bufferDescription.dwBufferBytes = bufferSize;
			bufferDescription.dwFlags = 0;
			bufferDescription.lpwfxFormat = &waveFormat;

			if(SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &GlobalSecondarySoundBuffer, 0)))
			{
				OutputDebugStringA("Secondary buffer created\n");
			}
			else
			{
				// TODO(afb) :: Diagnostics

			}
		}
		else
		{
			// TODO(afb) :: Diagnostic
		}
	}
	else
	{
			// TODO(afb) :: Diagnostic
	}
}

internal void
Win32LoadXInputLibrary(void)
{
	HMODULE xInputLibrary = LoadLibraryA("xinput1_3.dll");
	if(xInputLibrary)
	{
		XInputGetState_ = (x_input_get_state*)GetProcAddress(xInputLibrary, "XInputGetState");
		if(!XInputGetState_) XInputGetState_ = XinputGetStateStub;

		XInputSetState_ = (x_input_set_state*)GetProcAddress(xInputLibrary, "XInputSetState");
		if(!XInputSetState_) XInputSetState_ = XinputSetStateStub;

		if(!XInputGetState_ || !XInputSetState_)
		{
			// TODO(afb) :: Diagnostic
			OutputDebugStringA("Failed to load xinput");
		}
	}
	else
	{
			// TODO(afb) :: Diagnostic
	}
}

internal Win32WindowDimension
Win32GetWindowDimension(HWND windowHandle)
{
	RECT clientRect = {};
	Win32WindowDimension dim = {};
	GetClientRect(windowHandle, &clientRect);
	dim.Width = clientRect.right - clientRect.left;
	dim.Height = clientRect.bottom - clientRect.top;
	return dim;
}


// DIB - Device independent bitmap
internal void
Win32ResizeDIBSection(Win32DIBBuffer* buffer, int width, int height)
{
	if(buffer->Memory)
	{
		VirtualFree(buffer->Memory, 0, MEM_RELEASE);
	}

	buffer->Width = width;
	buffer->Height = height;
	buffer->BytesPerPixel = 4; // R G B and 1 extra for alignment
	buffer->Pitch = width * buffer->BytesPerPixel;

	// NOTE(afb) :: If the height is negative then bitmp is a
	// top-down DIB and its origin(0, 0) is the top left corner.
	buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
	buffer->Info.bmiHeader.biWidth = width;
	buffer->Info.bmiHeader.biHeight = -height;
	buffer->Info.bmiHeader.biPlanes = 1;
	buffer->Info.bmiHeader.biBitCount = 32;
	buffer->Info.bmiHeader.biCompression = BI_RGB;

	int32 bitmapMemorySize = (width*height) * buffer->BytesPerPixel;
	buffer->Memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32DisplayBufferToWindow(HDC deviceContext, const Win32DIBBuffer* buffer,
						   int x, int y, int width, int height)
{
	// NOTE(afb) :: Play with stretch modes.kissm
	StretchDIBits(deviceContext,
					0, 0, width, height,  // Destination
					0, 0, buffer->Width, buffer->Height, // Source
					buffer->Memory,
					&buffer->Info,
					DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK
Win32MessgaeProcedure(HWND windowHandle,
					  UINT message,
					  WPARAM wParam,
					  LPARAM lParam)
{
	LRESULT result = 0;

	switch(message)
	{
		// Window is activated or deactiated
		case (WM_ACTIVATEAPP):
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		}break;

		// Window is resized (minimize or maximize)
		case (WM_SIZE):
		{
			// Getting the drawable area of our window and calculating
			// the width and height.
			if(wParam != 1)
			{
				OutputDebugStringA("Resize not minimize.\n");
				Win32WindowDimension dim = Win32GetWindowDimension(windowHandle);
				Win32ResizeDIBSection(&GlobalBackBuffer, dim.Width, dim.Height);
			}
		}break;

		case (WM_KEYDOWN):
		case (WM_KEYUP):
		case (WM_SYSKEYDOWN):
		case (WM_SYSKEYUP):
		{
			uint32 vkCode = wParam;

			bool wasDown = ((lParam & (1 << 30)) != 0);
			bool isDown = ((lParam & (1 << 31)) == 0);

			if(wasDown != isDown)
			{
				if(vkCode == 'W')
				{
				}
				else if(vkCode == 'S')
				{
				}
				else if(vkCode == 'A')
				{
				}
				else if(vkCode == 'D')
				{
				}
				else if(vkCode == VK_UP)
				{
				}
				else if(vkCode == VK_DOWN)
				{
				}
				else if(vkCode == VK_LEFT)
				{
				}
				else if(vkCode == VK_RIGHT)
				{
				}
				else if(vkCode == VK_SPACE)
				{
				}
				else if(vkCode == 'Q')
				{
				}
				else if(vkCode == 'E')
				{
				}
				else if(vkCode == VK_ESCAPE)
				{
					Running = false;
				}
				else if((vkCode == VK_F4) && (lParam & (1 << 29) != 0))
				{
					Running = false;
				}
			}
		}break;

		case (WM_PAINT):
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(windowHandle, &paint);

			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int width = paint.rcPaint.right - x;
			int height = paint.rcPaint.bottom - y;

			// Updating the window
			Win32DisplayBufferToWindow(deviceContext, &GlobalBackBuffer,
									   x, y, width, height);

			EndPaint(windowHandle, &paint);
		}break;

		//Window is closed
		case (WM_CLOSE):
		{
			// TODO(afb) : Handle with a message to the user?
			Running = false;
			OutputDebugStringA("WM_CLOSE\n");
			//PostQuitMessage(0);
		}break;

		//Window is being destroyed
		case (WM_DESTROY):
		{
			// TODO(afb) : Handle as a error?
			Running = false;
			OutputDebugStringA("WM_DESTROY\n");
		}break;

		default:
		{
			// OutputDebugStringA("default\n");
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}break;
	}
	return(result);
}



internal void
Win32FillSoundBuffer(Win32SoundOutput* destBuffer, DWORD bytesToWrite,
					 DWORD byteToLock, SoundBuffer* sourceBuffer)
{
	VOID* region1;
	VOID* region2;
	DWORD region1Size;
	DWORD region2Size;
	
	if(SUCCEEDED(GlobalSecondarySoundBuffer->Lock(byteToLock, bytesToWrite,
												  &region1, &region1Size,
												  &region2, &region2Size,
												  0)))
	{
		int16* sourceSample = sourceBuffer->Data;

		int16* destSample = (int16*)region1;
		DWORD region1SampleCount = region1Size / destBuffer->bytesPerSample;
		for(DWORD sampleIndex = 0; sampleIndex < region1SampleCount; sampleIndex++)
		{
			*destSample++ = *sourceSample++;
			*destSample++ = *sourceSample++;
			destBuffer->runningSampleIndex++;
		}

		
		destSample = (int16*)region2;
		DWORD region2SampleCount = region2Size / destBuffer->bytesPerSample;
		for(DWORD sampleIndex = 0; sampleIndex < region2SampleCount; sampleIndex++)
		{
			*destSample++ = *sourceSample++;
			*destSample++ = *sourceSample++;
			destBuffer->runningSampleIndex++;
		}
		
		GlobalSecondarySoundBuffer->Unlock(region1, region1Size, region2, region2Size);
	}	
}

internal void
Win32ClearSoundBuffer(Win32SoundOutput* soundData)
{
	
	VOID* region1;
	VOID* region2;
	DWORD region1Size;
	DWORD region2Size;
	
	if(SUCCEEDED(GlobalSecondarySoundBuffer->Lock(0, soundData->bufferSize,
												  &region1, &region1Size,
												  &region2, &region2Size,
												  0)))
	{
		int8* destSample  = (int8*)region1;
		for(DWORD sampleIndex = 0; sampleIndex < region1Size; sampleIndex++)
		{
			*destSample = 0;
		}

		destSample  = (int8*)region2;
		for(DWORD sampleIndex = 0; sampleIndex < region2Size; sampleIndex++)
		{
			*destSample = 0;
		}
		
		GlobalSecondarySoundBuffer->Unlock(region1, region1Size, region2, region2Size);
	}// Lock
}

int CALLBACK
WinMain(HINSTANCE instance,
		HINSTANCE prevInstance,
		LPSTR commandLine,
		int nShowCmd)
{
	LARGE_INTEGER perfFreqCounter = {};
	QueryPerformanceFrequency(&perfFreqCounter);
	int64 perfFrequency = perfFreqCounter.QuadPart;
	
	Win32LoadXInputLibrary();

	//Window data
	WNDCLASSA windowClass = {};

	// Resizing DIB used for drawing
	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

	windowClass.style = CS_OWNDC;//|CS_HREDRAW|CS_VREDRAW;
	windowClass.lpfnWndProc = Win32MessgaeProcedure;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = "HandmadeHeroClass";

	//Register window
	if(RegisterClass(&windowClass))
	{
		//Create window
		HWND windowHandle = CreateWindowExA(0,
											windowClass.lpszClassName,
											"HandmaderHero",
											WS_OVERLAPPEDWINDOW|WS_VISIBLE,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											0,
											0,
											instance,
											0);

		if(windowHandle)
		{
			// BEGIN Sound Output
			// TODO(afb) :: Make the buffer a minute long?
			Win32SoundOutput soundData = {};
			soundData.samplesPerSecond = 48000;
			soundData.runningSampleIndex = 0;
			soundData.bytesPerSample = sizeof(int16) * 2;
			soundData.bufferSize = soundData.samplesPerSecond * soundData.bytesPerSample;
			soundData.latencySampleCount = soundData.samplesPerSecond / 15;
			
			Win32InitDirectSound(windowHandle, soundData.samplesPerSecond, soundData.bufferSize);
			Win32ClearSoundBuffer(&soundData);

			int16* samples = (int16*)VirtualAlloc(0, soundData.bufferSize , MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			
			GlobalSecondarySoundBuffer->Play(0, 0, DSBPLAY_LOOPING);			
			bool soundIsPlaying = true;


			uint64 previousCycle = __rdtsc();
			LARGE_INTEGER previousCounter = {};
			QueryPerformanceCounter(&previousCounter);
			
			Running = true;
			while(Running)
			{
				MSG message;
				// Retrieve messages from queue and dispatch.
				while(PeekMessage(&message, windowHandle, 0, 0, PM_REMOVE))
				{
					if(message.message == WM_QUIT)
					{
						Running = false;
					}

  					// TODO(afb) :: We may need to poll controller more frequently than
  					// every frame.
					for (DWORD controllerIndex = 0;
							controllerIndex < XUSER_MAX_COUNT;
								controllerIndex++ )
					{
						XINPUT_STATE state = {};

						if(XInputGetState(controllerIndex, &state) == ERROR_SUCCESS)
						{
							//NOTE(afb):: Controller connected.
							XINPUT_GAMEPAD* pad = &state.Gamepad;

							//D-PAD
							bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
							bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
							bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
							bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
							// Menu
							bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
							bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
							// Bumps
							bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
							bool rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
							// Buttons
							bool aButtun = (pad->wButtons & XINPUT_GAMEPAD_A);
							bool bButton = (pad->wButtons & XINPUT_GAMEPAD_B);
							bool xButton = (pad->wButtons & XINPUT_GAMEPAD_X);
							bool yButton = (pad->wButtons & XINPUT_GAMEPAD_Y);

							int16 leftStickX = pad->sThumbLX;
							int16 leftStickY = pad->sThumbLY;
							int16 rightStickX = pad->sThumbRX;
							int16 rightStickY = pad->sThumbRY;
						}
						else
						{
							//NOTE(afb) :: Controller not connected.
						}

  					}

					TranslateMessage(&message);
					DispatchMessageA(&message);
				}

				// If there are no messgaes still write to the buffer and update the DIB

				HDC deviceContext = GetDC(windowHandle);
				Win32WindowDimension dim = Win32GetWindowDimension(windowHandle);
				Win32DisplayBufferToWindow(deviceContext, &GlobalBackBuffer, 0, 0, dim.Width, dim.Height);
				ReleaseDC(windowHandle ,deviceContext);


				// NOTE(afb) :: Direct Sound Output Test
				DWORD writeCursor;
				DWORD playCursor;
				DWORD byteToLock;
				DWORD bytesToWrite;
				DWORD targetCursor;
				bool soundIsValid = false;

				if(SUCCEEDED(GlobalSecondarySoundBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
				{
					
					byteToLock = (soundData.runningSampleIndex * soundData.bytesPerSample) % soundData.bufferSize;
					targetCursor =
						(int32)(playCursor + soundData.latencySampleCount * soundData.bytesPerSample) % soundData.bufferSize;
					
					if(byteToLock > targetCursor)
					{
						bytesToWrite = soundData.bufferSize - byteToLock;// + playCursor;
						bytesToWrite += targetCursor;
					}
					else
					{
						bytesToWrite = targetCursor - byteToLock;
					}

					soundIsValid = true;
				}
				
				//NOTE(afb) :: Sound
				SoundBuffer soundBuffer = {};
				soundBuffer.SamplesPerSecond = soundData.samplesPerSecond;  
				soundBuffer.SampleCount = bytesToWrite / soundData.bytesPerSample;
				soundBuffer.Data = samples;
				
				//NOTE(afb) :: Rendering 
				GameOffScreenBuffer screenBuffer = {};
				screenBuffer.Memory = GlobalBackBuffer.Memory;
				screenBuffer.Width = GlobalBackBuffer.Width;
				screenBuffer.Height = GlobalBackBuffer.Height;
				screenBuffer.Pitch = GlobalBackBuffer.Pitch;
				screenBuffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;
				UpdateAndRender(&screenBuffer, &soundBuffer);


				if(soundIsValid)
				{					
					Win32FillSoundBuffer(&soundData, bytesToWrite, byteToLock, &soundBuffer);
				} // soundIsValid


				// NOTE(afb) :: Frame Counter Calculation
				uint64 endCycle = __rdtsc();
				int64 cyclesElapsed = endCycle - previousCycle;
				int32 megaCyclesPerFrame = (int32)cyclesElapsed / (1000 * 1000);
				
				LARGE_INTEGER endCounter = {};
				QueryPerformanceCounter(&endCounter);

				int64 counterElapsed  = endCounter.QuadPart - previousCounter.QuadPart;
				// Time is in milliiseconds
				int32 timeElapsedPerFrame = (int32)((counterElapsed*1000) / perfFrequency);
				int32 fps = 1000/timeElapsedPerFrame;
#if 0
				char buffer[256];
				wsprintf(buffer, "%dms, %dfps, %dMc/f\n", timeElapsedPerFrame, fps, megaCyclesPerFrame);
				OutputDebugStringA(buffer);
#endif			
				previousCycle = endCycle;
				previousCounter = endCounter;
			}// Running Loop
			VirtualFree(samples, 0, MEM_RELEASE);
		}
		else
		{
			// TODO(afb) :: Error Logging
		}
	}
	else
	{
		// TODO(afb) :: Error Logging
	}
		
	return(0);
}
