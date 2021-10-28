// Beatbox player

#ifndef BEATBOX_PLAYER_H
#define BEATBOX_PLAYER_H

// Run the beatbox player
// Must be called before any other functions
void Beatbox_startBeatbox(void);

// Stop the beatbox player
// Must be called after to clean up memory
void Beatbox_stopPlaying(void);


// Play air drums
void Beatbox_playAirDrums(int type);
// Play drum beat
void Beatbox_playDrumBeat(int type);
// Play next beat
void Beatbox_playNextBeat(void);

// Increase volume
int Beatbox_increaseVolume(void);
// Decrease volume
int Beatbox_decreaseVolume(void);

// Increase BPM
int Beatbox_increaseBPM(void);
// Decrease BPM
int Beatbox_decreaseBPM(void);

// Get current beat info
void Beatbox_getCurrentInfo(char* info);

#endif