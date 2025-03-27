/*
 * Aircraft.cpp
 *
 *  Created on: Mar 17, 2025
 *      Author: to_smith
 */

#include "Aircraft.h"
#include <string>
#include <iostream>
#include <vector>

using namespace std;

Aircraft::Aircraft() {
	// TODO Auto-generated constructor stub
}

Aircraft::Aircraft(int aTime, int aId, float xpos, float ypos, float zpos,
		float xspeed, float yspeed, float zspeed){
	this->aTime=aTime;
	this->aId=aId;
	this->xpos=xpos;
	this->ypos=ypos;
	this->zpos=zpos;
	this->xspeed=xspeed;
	this->yspeed=yspeed;
	this->zspeed=zspeed;
}
int Aircraft:: getId(){
	return aId;
}
float Aircraft:: getxpos(){
	return xpos;
}
float Aircraft:: getypos(){
	return ypos;
	}
float Aircraft:: getzpos(){
	return zpos;
	}
float Aircraft:: getxspeed(){
	return xspeed;
	}
float Aircraft:: getyspeed(){
	return yspeed;
	}
float Aircraft:: getzspeed(){
	return zspeed;
	}
void Aircraft:: setxpos(float xposition){
	xpos=xposition;
}
void Aircraft:: setypos(float yposition){
	ypos=yposition;
	}
void Aircraft:: setzpos(float zposition){
	zpos=zposition;
	}

//updates aircraft's position. Assuming task period of 3 sec.
void Aircraft:: updatePos(){
	float NewXpos=xpos+3*xspeed; //assuming speed is per seconds
	float NewYpos=ypos+3*yspeed;
	float NewZpos=zpos+3*zspeed;
	this->setxpos(NewXpos);
	this->setypos(NewYpos);
	this->setzpos(NewZpos);
}


 void Aircraft:: disp(){
	cout<<to_string(aTime)+" | "+to_string(aId)+" | "+to_string(xpos)+" | "+to_string(ypos)
		+" | "+to_string(zpos)+" | "+to_string(xspeed)+" | "+to_string(yspeed)+" | "+to_string(zspeed)<<endl;
}

Aircraft::~Aircraft() {
	// TODO Auto-generated destructor stub
}

