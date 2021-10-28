// udp_listener.c
#include "udp_listener.h"
#include "beatbox.h"

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define RX_BUFFER_LEN 1024
#define TX_BUFFER_LEN 1024
#define PORT 12345

static int stopFlag = 0;



// Prepare response accordingly
static void reply(char* messageRx, char* messageTx)
{
	// No beat
	if (strncmp(messageRx, "beat 0", sizeof("beat 0") - 1) == 0) {
		Beatbox_playDrumBeat(0);
	}
	
	// Rock #1
	else if (strncmp(messageRx, "beat 1", sizeof("beat 1") - 1) == 0) {
		Beatbox_playDrumBeat(1);
	}
	
	// Rock #2
	else if (strncmp(messageRx, "beat 2", sizeof("beat 2") - 1) == 0) {
		Beatbox_playDrumBeat(2);
	}
	
	// Volume up
	else if (strncmp(messageRx, "volume up", sizeof("volume up") - 1) == 0) {
		Beatbox_increaseVolume();
	}
	
	// Volume down
	else if (strncmp(messageRx, "volume down", sizeof("volume down") - 1) == 0) {
		Beatbox_decreaseVolume();
	}
	
	// Tempo up
	else if (strncmp(messageRx, "tempo up", sizeof("tempo up") - 1) == 0) {
		Beatbox_increaseBPM();
	}
	
	// Tempo down
	else if (strncmp(messageRx, "tempo down", sizeof("tempo down") - 1) == 0) {
		Beatbox_decreaseBPM();
	}
	
	// Play hi-hat
	else if (strncmp(messageRx, "drum hi-hat", sizeof("drum hi-hat") - 1) == 0) {
		Beatbox_playAirDrums(0);
	}
	
	// Play snare
	else if (strncmp(messageRx, "drum snare", sizeof("drum snare") - 1) == 0) {
		Beatbox_playAirDrums(1);
	}
	
	// Play bass
	else if (strncmp(messageRx, "drum bass", sizeof("drum bass") - 1) == 0) {
		Beatbox_playAirDrums(2);
	}
	
	// Stop the program
	else if (strncmp(messageRx, "stop", sizeof("stop") - 1) == 0) {
		stopFlag = 1;
	}

	
	// return current info every request
	Beatbox_getCurrentInfo(messageTx);
}

// Start listening using UDP
void UDP_Listener_startUDPListener(void)
{
	// Set up UDP (mostly similar to Dr. Brian Fraser's code since there's only 1 way to do this)
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(PORT);
	
	// Create and bind to socket
	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
	bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));
	
	while (1) {
		// Get the data
		struct sockaddr_in sinRemote;
		unsigned int sin_len = sizeof(sinRemote);
		char messageRx[RX_BUFFER_LEN];	
		
		int bytesRx = recvfrom(socketDescriptor,
			messageRx, RX_BUFFER_LEN, 0,
			(struct sockaddr *) &sinRemote, &sin_len);
		
		int terminateIdx = (bytesRx < RX_BUFFER_LEN) ? bytesRx : RX_BUFFER_LEN - 1;
		messageRx[terminateIdx] = 0;
		
		// Extract the meaning
		char messageTx[TX_BUFFER_LEN] = "";
		reply(messageRx, messageTx);
		
		// Reply
		sin_len = sizeof(sinRemote);
		sendto (socketDescriptor,
			messageTx, strlen(messageTx),
			0,
			(struct sockaddr *) &sinRemote, sin_len);
		
		
		// Quit if it's a stop
		if (stopFlag) {
			break;
		}
	}
	
	// Close
	close(socketDescriptor);
}