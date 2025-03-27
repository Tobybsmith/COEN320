/*
 * AircraftUtils.cpp
 *
 *  Created on: Mar 26, 2025
 *      Author: ismae
 */

#include "AircraftUtils.h"

Aircraft* getAircraftById(vector<Aircraft*> aircraftList,int aId){
	 for (int i=0; i<aircraftList.size();i++){
		 if(aircraftList[i]->getId()==aId)
			 return aircraftList[i];
	 }
	 throw runtime_error("Aircraft not found");
 }
