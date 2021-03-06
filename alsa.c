#include "alsa.h"


//Handle to ALSA (audio card's) playback port
snd_pcm_t *PlaybackHandle;

//Handle to our callback thread
snd_async_handler_t *CallbackHandle;

//Points to loaded WAVE file's data
unsigned char *WavePtr;

//Size (in frames) of loaded WAVE file's data
snd_pcm_uframes_t WaveSize;

//Sample rate
unsigned short WaveRate;

//Bit resolution
unsigned char WaveBits;

//Number of channels in the wave file
unsigned char WaveChannels;

/*
	The name of the ALSA port we output to. In this case, we're
	directly writing to hardware card 0,0 (ie, first set of audio
	outputs on the first audio card)
*/
static const char SoundCardPortName[] = "plughw:1,0";

//For WAVE file loading
static const unsigned char Riff[4] = { 'R', 'I', 'F', 'F' };
static const unsigned char Wave[4] = { 'W', 'A', 'V', 'E' };
static const unsigned char Fmt[4]  = { 'f', 'm', 't', ' ' };
static const unsigned char Data[4] = { 'd', 'a', 't', 'a' };



/********************** compareID() *********************
* Compares the passed ID str (ie, a ptr to 4 Ascii
* bytes) with the ID at the passed ptr. Returns TRUE if
* a match, FALSE if not.
*/

unsigned char compareID(const unsigned char * id, unsigned char * ptr)
{
	register unsigned char i = 4;

	while (i--)
	{
		if ( *(id)++ != *(ptr)++ ) return(0);
	}
	return(1);
}


/********************** waveLoad() *********************
* Loads a WAVE file.
*
* fn =			Filename to load.
*
* RETURNS: 0 if success, non-zero if not.
*
* NOTE: Sets the global "WavePtr" to an allocated buffer
* containing the wave data, and "WaveSize" to the size
* in sample points.
*/

unsigned char waveLoad(const char *fn)
{
	const char *message;
	FILE_head head;
	register int inHandle;

	if ((inHandle = open(fn, O_RDONLY)) == -1)
	{
		message = "didn't open";
	}
	else //Read in IFF File header
	{
		if (read(inHandle, &head, sizeof(FILE_head)) == sizeof(FILE_head))
		{
			//Is it a RIFF and WAVE?
			if (!compareID(&Riff[0], &head.ID[0]) || !compareID(&Wave[0], &head.Type[0]))
			{
				message = "is not a WAVE file";
				goto bad;
			}

			//Read in next chunk header
			while (read(inHandle, &head, sizeof(CHUNK_head)) == sizeof(CHUNK_head))
			{
				//============================ Is it a fmt chunk? ===============================
				if (compareID(&Fmt[0], &head.ID[0]))
				{
					FORMAT	format;
					//Read in the remainder of chunk
					if (read(inHandle, &format.wFormatTag, sizeof(FORMAT)) != sizeof(FORMAT)) break;

					//Can't handle compressed WAVE files
					if (format.wFormatTag != 1)
					{
						message = "compressed WAVE not supported";
						goto bad;
					}

					WaveBits = (unsigned char)format.wBitsPerSample;
					WaveRate = (unsigned short)format.dwSamplesPerSec;
					WaveChannels = format.wChannels;
				}

				//============================ Is it a data chunk? ===============================
				else if (compareID(&Data[0], &head.ID[0]))
				{
					//Size of wave data is head.Length. Allocate a buffer and read in the wave data
					if (!(WavePtr = (unsigned char *)malloc(head.Length)))
					{
						message = "won't fit in RAM";
						goto bad;
					}

					if (read(inHandle, WavePtr, head.Length) != head.Length)
					{
						free(WavePtr);
						break;
					}

					//Store size (in frames)
					WaveSize = (head.Length * 8) / ((unsigned int)WaveBits * (unsigned int)WaveChannels);

					close(inHandle);
					return(0);
				}

				//============================ Skip this chunk ===============================
				else
				{
					if (head.Length & 1) 
					{
						++head.Length;   //If odd, round it up to account for pad byte
					}
					lseek(inHandle, head.Length, SEEK_CUR);
				}
			}
		}
		message = "is a bad WAVE file";
		bad:	close(inHandle);
	}
	printf("%s %s\n", fn, message);
	return(1);
}

/********************** play_audio() **********************
* Plays the loaded waveform.
*
* NOTE: ALSA sound card's handle must be in the global
* "PlaybackHandle". A pointer to the wave data must be in
* the global "WavePtr", and its size of "WaveSize".
*/
void play_audio(void)
{
	register snd_pcm_uframes_t count, frames;
	//Output the wave data
	count = 0;
	do
	{
		frames = snd_pcm_writei(PlaybackHandle, WavePtr + count, WaveSize - count);
		//If an error, try to recover from it
		if (frames < 0)
		{
			frames = snd_pcm_recover(PlaybackHandle, frames, 0);
		}
		if (frames < 0)
		{
			printf("Error playing wave: %s\n", snd_strerror(frames));
			break;
		}

		//Update our pointer
		count += frames;
	} while (count < WaveSize);

	//Wait for playback to completely finish
	if (count == WaveSize)
	{
		snd_pcm_drain(PlaybackHandle);
	}
}

/*********************** free_wave_data() *********************
* Frees any wave data we loaded.
*
* NOTE: A pointer to the wave data be in the global
* "WavePtr".
*/
void free_wave_data(void)
{
	if (WavePtr) 
	{
		free(WavePtr);
	}
	WavePtr = NULL;
}

