#include "deamon.h"

#include "alsa.h"

#define MUSIKFILE "/home/<USER>/<MORE_PATH>/<File>.wav"

unsigned char *WavePtr;
snd_pcm_t *PlaybackHandle;
static const char SoundCardPortName[] = "plughw:1,0";
unsigned char WaveChannels;
unsigned short WaveRate;
unsigned char WaveBits;

int
main()
{
	int res;
	int ttl=6000;
	int delay=600;
	if( (res=daemonize("musik-deamon","/tmp",NULL,NULL,NULL)) != 0 ) 
	{
		fprintf(stderr,"error: daemonize failed\n");
		exit(EXIT_FAILURE);
	}
	while( ttl>0 ) 
	{
		//daemon code here
		//#######################################################################
		
		//No wave data loaded yet
		WavePtr = NULL;
		if (!waveLoad(MUSIKFILE)) //Load the wave file
		{
			register int err;
			//Open audio card we wish to use for playback
			if ((err = snd_pcm_open(&PlaybackHandle, &SoundCardPortName[0], SND_PCM_STREAM_PLAYBACK, 0)) < 0)
			{
				syslog(LOG_NOTICE,"Can't open audio %s: %s\n", &SoundCardPortName[0], snd_strerror(err));
			}
			else
			{
				switch (WaveBits)
				{
				case 8:
					err = SND_PCM_FORMAT_U8;
					break;

				case 16:
					err = SND_PCM_FORMAT_S16;
					break;

				case 24:
					err = SND_PCM_FORMAT_S24;
					break;

				case 32:
					err = SND_PCM_FORMAT_S32;
					break;
				}

				//Set the audio card's hardware parameters (sample rate, bit resolution, etc)
				if ((err = snd_pcm_set_params(PlaybackHandle, err, SND_PCM_ACCESS_RW_INTERLEAVED, WaveChannels, WaveRate, 1, 500000)) < 0)
				{
					syslog(LOG_NOTICE,"Can't set sound parameters: %s\n", snd_strerror(err));
				}
				else //Play the waveform
				{
					play_audio();
				}

				//Close sound card
				snd_pcm_close(PlaybackHandle);
			}
		}
	
		//#######################################################################

		syslog(LOG_NOTICE,"daemon ttl %d",ttl);
		sleep(delay);
		ttl-=delay;
	}
	syslog(LOG_NOTICE,"daemon ttl expired");

	//Free the WAVE data
	free_wave_data();

	closelog();
	return(EXIT_SUCCESS);
}
