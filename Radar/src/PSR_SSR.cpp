/*
 * PSR.cpp
 *
 *  Created on: Mar 24, 2025
 *      Author: ismae
 */

#include "PSR_SSR.h"

#include "Aircraft.h"
#include <vector>
#include <array>
#include "AircraftUtils.h"
#include <iostream>

PSR_SSR::PSR_SSR(vector<Aircraft*> aircrafts) {
	this->aircrafts=aircrafts;
}

PSR_SSR::~PSR_SSR() {
	// TODO Auto-generated destructor stub
}
//returns a vector of arrays containing the x,y,z positions of the aircrafts located in the range of the psr radar
vector<PSRData> PSR_SSR::PSRsignal(){
	PSRData psrdata;
	vector<PSRData> vectorPSRData;
	for(int i=0;i<aircrafts.size();i++){
		float xpos=aircrafts[i]->getxpos();
		float ypos=aircrafts[i]->getypos();
		float zpos=aircrafts[i]->getzpos();
		int id=aircrafts[i]->getId();
		if(xpos>= 0 && xpos<=10000 && ypos>= 0 && ypos<=10000 && zpos>= 15000 && zpos<=40000){
			psrdata.id=id;
			psrdata.x=xpos;
			psrdata.y=ypos;
			psrdata.z=zpos;

			vectorPSRData.push_back(psrdata);
		}
	}
	return vectorPSRData;
}

vector<SSRData> PSR_SSR::SSRSignal(vector<PSRData> vectorPSRData){
	SSRData ssrdata;
	vector<SSRData> vectorSSRData;
	int id;
	float xpos;
	float ypos;
	float Fl;
	float xspeed;
	float yspeed;
	float zspeed;
	Aircraft* aircraft;

	for(int i=0;i<vectorPSRData.size();i++){

		id=vectorPSRData[i].id;
		aircraft=getAircraftById(aircrafts, id);

		xpos=aircraft->getxpos();
		ypos=aircraft->getypos();
		Fl=aircraft->getzpos();

		xspeed=aircraft->getxspeed();
		yspeed=aircraft->getyspeed();
		zspeed=aircraft->getzspeed();

		ssrdata.x=xpos;
		ssrdata.y=ypos;
		ssrdata.fl=Fl;
		ssrdata.xspeed=xspeed;
		ssrdata.yspeed=yspeed;
		ssrdata.zspeed=zspeed;
		ssrdata.id=id;
		vectorSSRData.push_back(ssrdata);
	}
	return vectorSSRData;
}


