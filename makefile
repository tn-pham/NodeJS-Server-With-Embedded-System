CC_C = arm-linux-gnueabihf-gcc
LFLAGS = -L$(HOME)/mpt433/public/asound_lib_BBB
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror 
PUBDIR = $(HOME)/cmpt433/public/myApps/

all: app node wav
	
app:
	$(CC_C) $(CFLAGS) beatbox_main.c audioMixer_template.c udp_listener.c beatbox.c zencape_input.c -o beatbox $(LFLAGS) -pthread -lasound
	cp beatbox $(HOME)/cmpt433/public/myApps/
	
node:
	mkdir -p $(PUBDIR)/beatbox-server-copy/
	cp -R nodejs/* $(PUBDIR)/beatbox-server-copy/
	cd $(PUBDIR)/beatbox-server-copy/ && npm install

wav:
	mkdir -p $(PUBDIR)/beatbox-wav-files/
	cp -R beatbox-wav-files/* $(PUBDIR)/beatbox-wav-files/

clean:
	rm beatbox


