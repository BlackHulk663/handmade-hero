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
	bool IsConnected;
	bool IsAnalog;
	f32 LeftStickAverageY;
	f32 LeftStickAverageX;
	
	union
	{
		ButtonState Buttons[12];
		struct
		{
			ButtonState MoveUp;
			ButtonState MoveDown;
			ButtonState MoveLeft;
			ButtonState MoveRight;
			
			ButtonState ActionUp;
			ButtonState ActionDown;
			ButtonState ActionLeft;
			ButtonState ActionRight;
		
			ButtonState RightShoulder;
			ButtonState LeftShoulder;

			// NOTE(afb) :: All buttons must be added above this line
			ButtonState Start;
			ButtonState Back;
		};
	};
};

struct Input
{
	ControllerState Controllers[5];
};

inline ControllerState*
GetController(Input* input, uint32 index)
{
	ASSERT(index < ARRAY_COUNT(input->Controllers));
	return &input->Controllers[index];
}

internal void UpdateAndRender(Input* input, GameOffScreenBuffer* buffer,
							  SoundBuffer* soundBuffer);

#endif
