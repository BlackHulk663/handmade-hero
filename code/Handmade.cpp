#include "Handmade.h"

internal void
RenderWeirdGradient(GameOffScreenBuffer* buffer, int xOffset, int yOffset)
{
	uint8* row = (uint8*)buffer->Memory;

	for(int y = 0; y < buffer->Height; y++)
	{
		uint32* pixel = (uint32*)row;
		for (int x = 0; x < buffer->Width; x++)
		{
			int32 green = ((xOffset + x) << 8);
			int32 blue = (yOffset + y);
			*pixel = green | blue;
			pixel++;
		}
		row += buffer->Pitch;
	}
}

internal void
OutputSound(SoundBuffer* soundBuffer, int hz)
{
	local_persist int count = 0;
	local_persist f32 tSine = 0.0f;
	int32 toneVolume = 4000;
	int32 toneHZ = 256;
	int wavePeriod = (soundBuffer->SamplesPerSecond/toneHZ);
	
	int16* sampleOut = soundBuffer->Data;
	for(int32 sampleIndex = 0; sampleIndex < soundBuffer->SampleCount; sampleIndex++)
	{
		f32 sineValue = sinf(tSine);
		int16 sampleValue = (int16)(toneVolume * sineValue);
		
		*sampleOut++ = sampleValue;
		*sampleOut++ = sampleValue;
		
		tSine += 2 * PI32 * 1.0f / (f32)wavePeriod;	   
	}
	count++;
}

internal void
UpdateAndRender(GameOffScreenBuffer* buffer, SoundBuffer* soundBuffer)
{
	local_persist int xOffset = 0;
	local_persist int yOffset = 0;
	local_persist int toneHz = 256;
		
	OutputSound(soundBuffer, toneHz);
	RenderWeirdGradient(buffer, xOffset, yOffset);
}
