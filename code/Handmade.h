#ifndef _HANDMADE_H_
#define _HANDMADE_H_

/* 
   NOTE(afb) ::
   HANDMADE_INTERNAL:
   0 = Deployment build
   1 = My PC
   
   HANDMADE_DEBUG:
   0 = No slow code
   1 = Slow code welcome
*/

#if HANDMADE_INTERNAL
// NOTE(afb) :: IMPORTANT
// These are not for shipping, they are blocking and the
// write does not protect from lost data.

struct DebugFile
{
	void* Data;
	uint32 Size;
};

internal DebugFile DEBUGPlatformReadFile(char* fileName);
internal void DEBUGPlatformFreeFileMemory(DebugFile* file);
internal bool DEBUGPlatformWriteFile(char* fileName, uint32 memorySize, void* data);
#else
#endif

#if HANDMADE_DEBUG
#define ASSERT(exp)								\
	if(!(exp)){ *(int*)0 = 0;}
#else
#define ASSERT(exp)
#endif

#define KILOBYTE(amount) ((amount) * 1024)
#define MEGABYTE(amount) (KILOBYTE(amount) * 1024)
#define GIGABYTE(amount) (MEGABYTE(amount) * 1024)
#define TERABYTE(amount) (GIGABYTE(amount) * 1024)

#define ARRAY_COUNT(array) sizeof(array) / sizeof((array)[0])

struct GameOffScreenBuffer
{
	void* Memory;
	int32 Width;
	int32 Height;
	int32 Pitch;
	int32 BytesPerPixel;
};

struct SoundBuffer
{
	int32 SampleCount;
	int32 SamplesPerSecond;  
	int16* Data;
};

struct GameState
{
	int XOffset;
	int YOffset;
	int ToneHz;
};

struct GameMemory
{
	bool IsInitialized;

	uint64 PersistentMemorySize;
	void* PersistentMemory; // NOTE(afb) :: Required to be initialized to zero

	uint64 TransientMemorySize;
	void* TransientMemory; // NOTE(afb) :: Required to be initialized to zero
	
};

struct ButtonState
{
	int HalfTransitionCount;
	bool EndedDown;
};

struct ControllerState
{
	bool IsAnalog;
	
	f32 StartX;
	f32 EndX;
	f32 MinX;
	f32 MaxX;
	
	f32 StartY;
	f32 EndY;
	f32 MinY;
	f32 MaxY;
	
	union
	{
		ButtonState buttons[6];
		struct
		{
			ButtonState Up;
			ButtonState Down;
			ButtonState Left;
			ButtonState Right;
			ButtonState RightShoulder;
			ButtonState LeftShoulder;
		};
	};
};

struct Input
{
	ControllerState Controllers[4];
};


internal void UpdateAndRender(Input* input, GameOffScreenBuffer* buffer,
							  SoundBuffer* soundBuffer);

#endif
