#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "CUtils.h"
#include "Aircraft.h"
#include <sstream>
#include <array>
#include "../../Shared_mem/src/Shared_mem.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "PSR_SSR.h"

using namespace std;




vector<Aircraft*> initplanes(string FileName);

int main() {

	// Shared memory not finished
//	int shm_fd = shm_open("/Shared_mem", O_RDWR, 0666);
//	if (shm_fd == -1) {
//	    perror("shm_open");
//	    exit(1);
//	}
//
//	SharedMemory* sharedMem = (SharedMemory*) mmap(
//	    NULL,
//	    sizeof(SharedMemory),
//	    PROT_READ | PROT_WRITE,
//	    MAP_SHARED,
//	    shm_fd,
//	    0
//	);
//
//	if (sharedMem == MAP_FAILED) {
//	    perror("mmap");
//	    exit(1);
//	}

	//opens input file, reads the file, and initialize aircrafts
	string FileName="test_1.txt";
	vector<Aircraft*> aircraftList = initplanes(FileName);
	for (int i=0; i<aircraftList.size();i++){
		aircraftList[i]->disp(); //display aircrafts info

	}


	PSR_SSR psr_ssr(aircraftList);

	//PSR signal store the position of aircrafts that are in range of the radar
	vector <PSRData>psrData=psr_ssr.PSRsignal();
	for (int i=0;i<psrData.size();i++){
		cout<<"From PSR ==> ID: "<<psrData[i].id <<" | x = "<<psrData[i].x<<" |  y= "<<psrData[i].y<<" | z = "<<psrData[i].z<<endl;
	}

	//From the PSR vector, SSR radar communicate with each aircrafts in range to get their positions
	vector <SSRData> ssrData=psr_ssr.SSRSignal(psrData);
	for (int i=0;i<ssrData.size();i++){
			cout<<"From SSR ==> ID: "<<ssrData[i].id <<" | x = "<<ssrData[i].x<<" |  y= "<<ssrData[i].y<<" | Fl = "<<ssrData[i].fl<<" | xspeed = "<< ssrData[i].xspeed<<" | yspeed = "<< ssrData[i].yspeed<<" | zspeed = "<< ssrData[i].zspeed<< endl;
		}


	return 0;
}

//open input file + initialize aircrafts
vector<Aircraft*> initplanes(string FileName){
	ifstream InputFile;
	InputFile.open("/tmp/resources/"+FileName);

	if(!InputFile.is_open()){
			cout<<"Failed to open file "+FileName<<endl;
			return {};
	}

	else{
	vector<Aircraft*> aircraftList;
	string line;
	while(getline(InputFile,line)){
		if(line.empty() || line[0]=='#')
			continue;
		if(!line.empty() && line.back()=='\r'){
			line.erase(line.size());
		}

		stringstream ss(line);
		int time, id;
		float xpos, ypos, zpos, xspeed, yspeed, zspeed;

		if(ss>>time>>id>>xpos>>ypos>>zpos>>xspeed>>yspeed>>zspeed){
			Aircraft* aircraft = new Aircraft(time,id,xpos,ypos,zpos,xspeed,yspeed,zspeed);
			aircraftList.push_back(aircraft);
		}
		else
			cout<<"Invalid line format: "<<line<<endl;
	}
//	cout<<"Success"<<endl;
	return aircraftList;
	}

}

//vector<array<float,4>> SSR(vector<Aircraft*>aircraftList){
//	for(int i=0; i<aircraftList.size();i++){
//		if(aircraftList[i]->getxpos()	)
//	}
//
//
//}





