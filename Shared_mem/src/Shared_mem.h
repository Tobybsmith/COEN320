#pragma once
#include <pthread.h>

struct SharedClock {
    int currentTimeInSeconds;
    pthread_mutex_t clockMutex;
};

struct PSRData {
    int id;
    float x, y, z;
};

struct SSRData {
    int id;
    float x, y, fl;
    float xspeed, yspeed, zspeed;
};

struct AircraftCommand {
	int id = -1;
	float xspeed, yspeed, zspeed;
};

#define MAX_AIRCRAFT 100

struct SharedMemory {
    SharedClock simClock;
    PSRData psrData[MAX_AIRCRAFT];
    int psrDataCount;
    SSRData ssrData[MAX_AIRCRAFT];
    int ssrDataCount;
    pthread_mutex_t radarDataMutex;
    pthread_mutex_t displayConsoleMutex;
    int displayDetailId;
    AircraftCommand consoleCommand;
};
