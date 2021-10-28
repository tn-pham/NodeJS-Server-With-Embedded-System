#include "audioMixer_template.h"
#include "beatbox.h"
#include "udp_listener.h"
#include "zencape_input.h"

#include <stdio.h>
#include <stdlib.h>

// Main function
int main(void)
{
	Beatbox_startBeatbox();
	ZenCape_input_init();
	
	// Main thread will keep listening until told to stop
	UDP_Listener_startUDPListener();

	ZenCape_input_cleanup();
	Beatbox_stopPlaying();
}