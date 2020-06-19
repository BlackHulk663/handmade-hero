#if !defined(_WIN32_HANDMADE_)
#define _WIN32_HANDMADE_H_

struct Win32DIBBuffer
{
	BITMAPINFO Info;
	void* Memory;
	int32 Width;
	int32 Height;
	int32 Pitch;
	int32 BytesPerPixel;
};

struct Win32WindowDimension
{
	int32 Width;
	int32 Height;
};


struct Win32SoundOutput
{
	int32 samplesPerSecond;
	int32 bytesPerSample;
	int32 bufferSize;
	f32 latencySampleCount;
	uint32 runningSampleIndex;
};

#endif
