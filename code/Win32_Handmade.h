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
	int32 SamplesPerSecond;
	int32 BytesPerSample;
	int32 BufferSize;
	f32 LatencySampleCount; // Amount of samples of latency for the sound
	uint32 RunningSampleIndex; // Sample to write at
	//TODO(afb) :: Maybe have a bytes per sc field?
	//TODO(afb) :: RunningSampleIndex should maybe be in mytes
};

struct Win32DebugSoundMarker
{
	DWORD PlayCursor;
	DWORD WriteCursor;
};

#endif
