#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Include the ALSA .H file that defines ALSA functions/data
#include <alsa/asoundlib.h>



#pragma pack (1)
// WAVE File Stuff /
//An IFF file header looks like this
typedef struct _FILE_head
{
	unsigned char	ID[4];	 	//could be {'R', 'I', 'F', 'F'} or {'F', 'O', 'R', 'M'}
	unsigned int	Length;	 	//Length of subsequent file (including remainder of header). This is in
					//Intel reverse byte order if RIFF, Motorola format if FORM.
	unsigned char	Type[4]; 	//{'W', 'A', 'V', 'E'} or {'A', 'I', 'F', 'F'}
} FILE_head;


//An IFF chunk header looks like this
typedef struct _CHUNK_head
{
	unsigned char ID[4];		//4 ascii chars that is the chunk ID
	unsigned int	Length;	 	//Length of subsequent data within this chunk. This is in Intel reverse byte
					//order if RIFF, Motorola format if FORM. Note: this doesn't include any
					//extra byte needed to pad the chunk out to an even size.
} CHUNK_head;

//WAVE fmt chunk
typedef struct _FORMAT {
	short wFormatTag;
	unsigned short wChannels;
	unsigned int dwSamplesPerSec;
	unsigned int dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;
					//Note: there may be additional fields here, depending upon wFormatTag
} FORMAT;
#pragma pack()

/*
	Size of the audio card hardware buffer. Here we want it
	set to 1024 16-bit sample points. This is relatively
	small in order to minimize latency. If you have trouble
	with underruns, you may need to increase this, and PERIODSIZE
	(trading off lower latency for more stability)
*/
#define BUFFERSIZE	(2*1024)

/*
	How many sample points the ALSA card plays before it calls
	our callback to fill some more of the audio card's hardware
	buffer. Here we want ALSA to call our callback after every
	64 sample points have been played
*/
#define PERIODSIZE	(2*64)

/*+++++++++++++++++++++++++++++++++ Functions ++++++++++++++++++++++++++++++++++++*/

unsigned char compareID(const unsigned char * id, unsigned char * ptr);

unsigned char waveLoad(const char *fn);

void play_audio(void);

void free_wave_data(void);
