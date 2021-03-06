#if !defined _HANDMADE_H_
#define _HANDMADE_H_

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

internal void UpdateAndRender(GameOffScreenBuffer* buffer, SoundBuffer* soundBuffer);

#endif
