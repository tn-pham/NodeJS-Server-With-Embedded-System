// Beatbox thread to keep playing the beat

#define WAVE_SNARE "/mnt/remote/myApps/beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"
#define WAVE_HI_HAT "/mnt/remote/myApps/beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define WAVE_BASS "/mnt/remote/myApps/beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"

#define DEFAULT_BPM 120
#define DEFAULT_VOLUME 80
#define MAX_BPM 300
#define MIN_BPM 40
#define MAX_VOL 100
#define MIN_VOL 0

#define VOL_INC_DEC_RATE 5
#define BPM_INC_DEC_RATE 5

#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "audioMixer_template.h"
#include "beatbox.h"


static int stopFlag = 0;
static int beatMode = 2;

static wavedata_t snare;
static wavedata_t hi_hat;
static wavedata_t bass;

// volume and bpm with mutexes
static pthread_mutex_t bpmMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t volumeMutex = PTHREAD_MUTEX_INITIALIZER;
static int bpm = DEFAULT_BPM;
static int volume = DEFAULT_VOLUME;

// thread
static pthread_t beatboxThreadId;

// bpm
static void changeBPM(int newBPM)
{
	if (newBPM <= MAX_BPM && newBPM >= MIN_BPM) {
		pthread_mutex_lock(&bpmMutex);
		{
			bpm = newBPM;
		}
		pthread_mutex_unlock(&bpmMutex);
	}	
	
	printf("BPM: %d\n", bpm);
}

// volume
static void changeVolume(int newVolume)
{
	if (newVolume <= MAX_VOL && newVolume >= MIN_VOL) {
		pthread_mutex_lock(&volumeMutex);
		{
			volume = newVolume;
			AudioMixer_setVolume(volume);
		}
		pthread_mutex_unlock(&volumeMutex);
	}	
	
	printf("Volume: %d\n", volume);
}

// Increase volume
int Beatbox_increaseVolume(void)
{
	changeVolume(volume + VOL_INC_DEC_RATE);
	return volume;
}

// Decrease volume
int Beatbox_decreaseVolume(void)
{
	changeVolume(volume - VOL_INC_DEC_RATE);
	return volume;
}

// Increase BPM
int Beatbox_increaseBPM(void)
{
	changeBPM(bpm + BPM_INC_DEC_RATE);
	return bpm;
}

// Decrease BPM
int Beatbox_decreaseBPM(void)
{
	changeBPM(bpm - BPM_INC_DEC_RATE);
	return bpm;
}

// Load drum files to memeory
static void loadDrumFiles(void)
{
	AudioMixer_readWaveFileIntoMemory(WAVE_SNARE, &snare);
	AudioMixer_readWaveFileIntoMemory(WAVE_HI_HAT, &hi_hat);
	AudioMixer_readWaveFileIntoMemory(WAVE_BASS, &bass);
}

// Clear drum files out of memory
static void freeDrumFiles(void)
{
	AudioMixer_freeWaveFileData(&snare);
	AudioMixer_freeWaveFileData(&hi_hat);
	AudioMixer_freeWaveFileData(&bass);
}

// Sleep time for half-beat in nanoseconds based on BPM
static void sleepNanoSeconds(void)
{
	long milisec;
	// Get real time BPM
	pthread_mutex_lock(&bpmMutex);
	{
		// Time For Half Beat [sec] = 60 [sec/min] / BPM / 2 [half-beats per beat] * 1000000 [nanosec]
		milisec = 60 * 1000 / 2 / bpm;
		//printf("%ld\n", nanoseconds);
	}
	pthread_mutex_unlock(&bpmMutex);
	
	// Sleep
	long seconds = 0;
	struct timespec sleepTimer = {seconds, milisec * 1000000};
	nanosleep(&sleepTimer, (struct timespec*) NULL);
}

// Beat mode 1
static void runBeatMode1(void)
{
	AudioMixer_queueSound(&hi_hat);
	AudioMixer_queueSound(&bass);
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&hi_hat);
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&hi_hat);
	AudioMixer_queueSound(&snare);
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&hi_hat);
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&hi_hat);
	AudioMixer_queueSound(&bass);
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&hi_hat);
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&hi_hat);
	AudioMixer_queueSound(&snare);
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&hi_hat);
	sleepNanoSeconds();

}

// Beat mode 2
static void runBeatMode2(void)
{
	AudioMixer_queueSound(&bass);
	AudioMixer_queueSound(&hi_hat);
	sleepNanoSeconds();
	
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&snare);
	sleepNanoSeconds();
	
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&bass);
	AudioMixer_queueSound(&hi_hat);
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&bass);
	sleepNanoSeconds();
	
	AudioMixer_queueSound(&snare);
	sleepNanoSeconds();
	
	sleepNanoSeconds();
}

// Play drum beat
void Beatbox_playDrumBeat(int type)
{
	beatMode = type;
}

// Play next beat
void Beatbox_playNextBeat(void)
{
	beatMode = (beatMode + 1) % 3;
}

// Play air drums
void Beatbox_playAirDrums(int type)
{
	if (type == 0) {
		AudioMixer_queueSound(&hi_hat);
	} else if (type == 1) {
		AudioMixer_queueSound(&snare);
	} else if (type == 2) {
		AudioMixer_queueSound(&bass);
	} else {
		// Error
		fprintf(stderr, "ERROR: Invalid air drum mode./n");
		exit(EXIT_FAILURE);
	}
}

// Get current beat info
void Beatbox_getCurrentInfo(char* info)
{
	if (beatMode == 0) {
		sprintf(info, "None--%d--%d", volume, bpm);
	} else if (beatMode == 1) {
		sprintf(info, "Rock #1--%d--%d", volume, bpm);
	} else if (beatMode == 2) {
		sprintf(info, "Rock #2--%d--%d", volume, bpm);
	} else {
		sprintf(info, "Unknown beat--%d--%d", volume, bpm);
	}
}

// Stop the thread
void Beatbox_stopPlaying(void)
{
	stopFlag = 1;
	pthread_join(beatboxThreadId, NULL);
}


// Spawn new thread to keep playing beatbox
void* beatboxThread(void* arg)
{
	// Load files
	loadDrumFiles();
	
	// Spawn background thread
	AudioMixer_init();
	
	// Cache volume
	volume = AudioMixer_getVolume();
	
	while(!stopFlag) {
		if (beatMode == 0) {
			// Do nothing
		} else if (beatMode == 1) {
			runBeatMode1();
		} else if (beatMode == 2) {
			runBeatMode2();
		} else {
			// Error
			fprintf(stderr, "ERROR: Invalid beat mode./n");
			exit(EXIT_FAILURE);
		}
	}
	
	
	// Stop background thread
	AudioMixer_cleanup();
	
	// Free memory
	freeDrumFiles();
	
	return NULL;
}

// Run the play thread
void Beatbox_startBeatbox(void)
{
	pthread_create(&beatboxThreadId, NULL, beatboxThread, NULL);
}