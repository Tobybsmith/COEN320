/*
 * Aircraft.cpp
 *
 *  Created on: Mar 17, 2025
 *      Author: to_smith
 */

#include "Aircraft.h"
#include <string>

int Aircraft::mIndexCounter = 0;

using namespace std;

Aircraft::Aircraft() {
	// TODO Auto-generated constructor stub
}

Aircraft::Aircraft(int aTime, int aId, Vec3f aPos, Vec3f aSpeed)
{
	mEntryTime = aTime;
	mId = aId;
	mEntryPos = aPos;
	mSpeed = aSpeed;
	mIndex = mIndexCounter;
	mIndexCounter++;
}

std::string Aircraft::ToString()
{
	//return string(mEntryTime) + " | " + string(mId) + " | " + string(mEntryPos.x) + ", " +
	//		string(mEntryPos.y)+ ", " + string(mEntryPos.z) + " | " + string(mSpeed.x) + ", "
	//		+ string(mSpeed.y) + ", " + string(mSpeed.z) + ", INDEX: " + string(mIndex);
	return "Airplane";
}

Aircraft::~Aircraft() {
	// TODO Auto-generated destructor stub
}

