/*
 * PSR.h
 *
 *  Created on: Mar 24, 2025
 *      Author: ismae
 */

#ifndef PSR_SSR_H_
#define PSR_SSR_H_
#include <vector>
#include"Aircraft.h"
#include "../../Shared_mem/src/Shared_mem.h"
using namespace std;





class PSR_SSR {
public:
	vector<Aircraft*> aircrafts;
	vector<PSRData> PSRsignal();
	vector<SSRData> SSRSignal(vector<PSRData> vectorPSRData);
	PSR_SSR(vector<Aircraft*> aircrafts);
	virtual ~PSR_SSR();

};


#endif /* PSR_SSR_H_ */
