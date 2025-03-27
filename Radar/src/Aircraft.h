#ifndef AIRCRAFT_H
#define AIRCRAFT_H
#include "CUtils.h"
#include <vector>
#include <string>
using namespace std;
class Aircraft {
public:
	Aircraft();
	Aircraft(int aTime, int aId, float xpos, float ypos, float zpos,
			float xspeed, float yspeed, float zspeed);

	virtual ~Aircraft();
	int aId,aTime;
	float xpos,ypos,zpos,xspeed,yspeed,zspeed;

	void disp();
	float getxpos();
	float getypos();
	float getzpos();

	float getxspeed();
	float getyspeed();
	float getzspeed();

	int getId();

	void setxpos(float xposition);
	void setypos(float yposition);
	void setzpos(float zposition);

	void updatePos();

	const Aircraft& getAircraftById(vector<Aircraft*> aircraftList,int aId);
};
#endif
