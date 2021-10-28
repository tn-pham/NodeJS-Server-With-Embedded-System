#include "zencape_input.h"
#include "beatbox.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#define BUFFER_SIZE 10

#define EXPORT_FILE_PATH "/sys/class/gpio/export"

#define JOYSTICK_UP_NUM "26"
#define JOYSTICK_RIGHT_NUM "47"
#define JOYSTICK_DOWN_NUM "46"
#define JOYSTICK_LEFT_NUM "65"
#define JOYSTICK_PUSH_NUM "27"

#define JOYSTICK_UP_DIRECTION "/sys/class/gpio/gpio26/direction"
#define JOYSTICK_RIGHT_DIRECTION "/sys/class/gpio/gpio47/direction"
#define JOYSTICK_DOWN_DIRECTION "/sys/class/gpio/gpio46/direction"
#define JOYSTICK_LEFT_DIRECTION "/sys/class/gpio/gpio65/direction"
#define JOYSTICK_PUSH_DIRECTION "/sys/class/gpio/gpio27/direction"

#define JOYSTICK_UP_VALUE "/sys/class/gpio/gpio26/value"
#define JOYSTICK_RIGHT_VALUE "/sys/class/gpio/gpio47/value"
#define JOYSTICK_DOWN_VALUE "/sys/class/gpio/gpio46/value"
#define JOYSTICK_LEFT_VALUE "/sys/class/gpio/gpio65/value"
#define JOYSTICK_PUSH_VALUE "/sys/class/gpio/gpio27/value"

#define SLEEP_NANO_SEC_EXPORT 330000000
#define SLEEP_NANO_SEC_REFRESH_RATE 10000000
#define MAX_DEBOUNCE_COUNTER 100

#define REG_ADD 0x00

static int oldAcceX = 0;
static int oldAcceY = 0;
static int oldAcceZ = 0;
static int debounceCounterAcce = MAX_DEBOUNCE_COUNTER;
static int debounceCounterJoystick = MAX_DEBOUNCE_COUNTER;

static int i2cFileDesc = 0;
static pthread_t inputThreadId;
static int stopFlag = 0;

// File Management (from assignment 1)
// Write
static void writeToFile(char* filePath, char* content)
{
	FILE* file = NULL;

	// Open file
	file = fopen(filePath, "w");
	if (file == NULL) {
		printf("ERROR: Failed to open file %s\n", filePath);
		exit(1);
	}

	// Write
	fprintf(file, "%s", content);

	// Close file
	fclose(file);
}

// Read to buffer
static int readFromFile(char* filePath)
{
	FILE* file = NULL;
	
	// Open file
	file = fopen(filePath, "r");
	if (file == NULL) {
		printf("ERROR: Failed to open file %s\n", filePath);
		exit(1);
	}
	
	// Read
	char buffer[BUFFER_SIZE];
	fgets(buffer, BUFFER_SIZE, file);
	
	// Close file
	fclose(file);

	return atoi(buffer);
}

// Nanosleep
static void sleepNanoSec(int nanoseconds)
{
	long seconds = 0;
	struct timespec sleepTimer = {seconds, nanoseconds};
	nanosleep(&sleepTimer, (struct timespec*) NULL);
}

// Export pins for joystick, sleep 330ms after each export
static void exportPins(void)
{
	writeToFile(EXPORT_FILE_PATH, JOYSTICK_DOWN_NUM);
	sleepNanoSec(SLEEP_NANO_SEC_EXPORT);
	writeToFile(EXPORT_FILE_PATH, JOYSTICK_UP_NUM);
	sleepNanoSec(SLEEP_NANO_SEC_EXPORT);
	writeToFile(EXPORT_FILE_PATH, JOYSTICK_LEFT_NUM);
	sleepNanoSec(SLEEP_NANO_SEC_EXPORT);
	writeToFile(EXPORT_FILE_PATH, JOYSTICK_RIGHT_NUM);
	sleepNanoSec(SLEEP_NANO_SEC_EXPORT);
	writeToFile(EXPORT_FILE_PATH, JOYSTICK_PUSH_NUM);
	sleepNanoSec(SLEEP_NANO_SEC_EXPORT);
}

// Set direction as read
static void setDirectionPins(void)
{
	writeToFile(JOYSTICK_UP_DIRECTION, "in");
	writeToFile(JOYSTICK_RIGHT_DIRECTION, "in");
	writeToFile(JOYSTICK_DOWN_DIRECTION, "in");
	writeToFile(JOYSTICK_LEFT_DIRECTION, "in");
	writeToFile(JOYSTICK_PUSH_DIRECTION, "in");
}

// Take joystick input
static void takeJoystickInput(void)
{
	// Only take 1 input per 500ms
	if (debounceCounterJoystick < MAX_DEBOUNCE_COUNTER) {
		debounceCounterJoystick++;
		return;
	}
	
	// Reset counter if there's an input
	if (readFromFile(JOYSTICK_UP_VALUE) == 0) {
		Beatbox_increaseVolume();
		debounceCounterJoystick = 0;
	} else if (readFromFile(JOYSTICK_DOWN_VALUE) == 0) {
		Beatbox_decreaseVolume();
		debounceCounterJoystick = 0;
	} else if (readFromFile(JOYSTICK_LEFT_VALUE) == 0) {
		Beatbox_decreaseBPM();
		debounceCounterJoystick = 0;
	} else if (readFromFile(JOYSTICK_RIGHT_VALUE) == 0) {
		Beatbox_increaseBPM();
		debounceCounterJoystick = 0;
	} else if (readFromFile(JOYSTICK_PUSH_VALUE) == 0) {
		Beatbox_playNextBeat();
		debounceCounterJoystick = 0;
	}
}

// Take accelerometer input
void takeAccelerometerInput()
{
	unsigned char regAddr = 0x00;
	
	int res = write(i2cFileDesc, &(regAddr), sizeof(regAddr));
	if (res != sizeof(regAddr)) {
		perror("I2C: Unable to write to i2c register.");
		exit(1);
	}

	// Now read the values
	char buff[7];
	res = read(i2cFileDesc, &buff, sizeof(buff));
	if (res != sizeof(buff)) {
		perror("I2C: Unable to read from i2c register");
		exit(1);
	}

	// directions
	int16_t x = (buff[1] << 8) | (buff[2]);
	int16_t y = (buff[3] << 8) | (buff[4]);
	int16_t z = (buff[5] << 8) | (buff[6]);

	// Only accepts 1 input per 500ms
	if (debounceCounterAcce < MAX_DEBOUNCE_COUNTER) {
		debounceCounterAcce++;
		return;
	}

	// Reset counter if there's an input
	if (z - oldAcceZ < -8000 || z - oldAcceZ > 8000) {
		Beatbox_playAirDrums(2);
		debounceCounterAcce = 0;
	} else if (x - oldAcceX < -4000 || x - oldAcceX > 4000) {
		Beatbox_playAirDrums(0);
		debounceCounterAcce = 0;
	} else if (y - oldAcceY < -4000 || y - oldAcceY > 4000) {
		Beatbox_playAirDrums(1);
		debounceCounterAcce = 0;
	}

	oldAcceX = x;
	oldAcceY = y;
	oldAcceZ = z;
}

// Background thread to take inputs continuously
void* startTakingInputs(void* arg)
{
	while(!stopFlag) {
		takeJoystickInput();
		takeAccelerometerInput();
		
		
		// Sleep 10ms
		sleepNanoSec(SLEEP_NANO_SEC_REFRESH_RATE);
	}
	
	return NULL;
}

// Set up accelerometer
void setUpAccelerometer(void)
{
	// Turn it on
	FILE* acce = popen("i2cset -y 1 0x1C 0x2A 0x01", "r");
	if (acce == NULL) {
		perror("I2C: Unable to make accelerometer active.\n");
		exit(1);
	}
	pclose(acce);

	// Init reg
	i2cFileDesc = open("/dev/i2c-1", O_RDWR);
	int result = ioctl(i2cFileDesc, I2C_SLAVE, 0x1C);
	if (result < 0) {
		perror("I2C: Unable to set I2C device to slave address.");
		exit(1);
	}
}

// Start the background thread
void ZenCape_input_init(void)
{
	// Set up joystick
	exportPins();
	setDirectionPins();
	
	// Set up accelerometer
	setUpAccelerometer();
	
	// Start thread
	pthread_create(&inputThreadId, NULL, startTakingInputs, NULL);
}

// Stop the thread
void ZenCape_input_cleanup(void)
{
	stopFlag = 1;
	pthread_join(inputThreadId, NULL);
}