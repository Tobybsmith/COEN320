#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <array>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "CUtils.h"
#include "Aircraft.h"
#include "PSR_SSR.h"
#include "../../Shared_mem/src/Shared_mem.h"

using namespace std;

// Shared data
vector<Aircraft*> aircraftList;
SharedMemory* sharedMem;

mutex aircraftMutex;

// Synchronization flags
atomic<bool> clockReady(false);
atomic<bool> aircraftsReady(false);
atomic<bool> psrReady(false);

void ClockThread() {
    while (true) {
        pthread_mutex_lock(&sharedMem->simClock.clockMutex);
        sharedMem->simClock.currentTimeInSeconds++;
        int t = sharedMem->simClock.currentTimeInSeconds;
        pthread_mutex_unlock(&sharedMem->simClock.clockMutex);

        if (t == 1) {
            clockReady = true;
        }

        cout << "--- Simulation clock: " << t << "s" << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void AircraftLogic(Aircraft* aircraft) {
    while (!clockReady) {}  // Wait for clock to be ready

    static atomic<int> aircraftStarted(0);
    if (++aircraftStarted == aircraftList.size()) {
        aircraftsReady = true;
    }

    while (true) {
        pthread_mutex_lock(&sharedMem->simClock.clockMutex);
        int clock = sharedMem->simClock.currentTimeInSeconds;
        pthread_mutex_unlock(&sharedMem->simClock.clockMutex);

        if (clock < aircraft->getTime()) {
            this_thread::sleep_for(chrono::milliseconds(200));
            continue;
        }

        {
            lock_guard<mutex> lock(aircraftMutex);
            aircraft->updatePos();
        }

        this_thread::sleep_for(chrono::milliseconds(1100));
    }
}

void PSRThread(PSR_SSR& psr_ssr) {
    while (!aircraftsReady) {}  // Wait for aircrafts to be ready
    psrReady = true;

    while (true) {
        vector<PSRData> detected;
        {
            lock_guard<mutex> lock(aircraftMutex);
            detected = psr_ssr.PSRsignal();
        }

        pthread_mutex_lock(&sharedMem->radarDataMutex);
        sharedMem->psrDataCount = 0;
        for (const auto& p : detected) {
            sharedMem->psrData[sharedMem->psrDataCount++] = p;
            cout << "From PSR ==> ID: " << p.id << " | x = " << p.x << " | y = " << p.y << " | z = " << p.z << endl;
        }
        pthread_mutex_unlock(&sharedMem->radarDataMutex);

        this_thread::sleep_for(chrono::milliseconds(2000));
    }
}

void SSRThread(PSR_SSR& psr_ssr) {
    while (!psrReady) {}  // Wait for PSR to be ready

    while (true) {
        vector<PSRData> input;

        pthread_mutex_lock(&sharedMem->radarDataMutex);
        for (int i = 0; i < sharedMem->psrDataCount; i++) {
            input.push_back(sharedMem->psrData[i]);
        }
        pthread_mutex_unlock(&sharedMem->radarDataMutex);

        auto results = psr_ssr.SSRSignal(input);

        pthread_mutex_lock(&sharedMem->radarDataMutex);
        sharedMem->ssrDataCount = 0;
        for (const auto& s : results) {
            sharedMem->ssrData[sharedMem->ssrDataCount++] = s;
            cout << "From SSR ==> ID: " << s.id << " | x = " << s.x << " | y = " << s.y << " | z = " << s.fl << endl;
        }
        pthread_mutex_unlock(&sharedMem->radarDataMutex);

        this_thread::sleep_for(chrono::milliseconds(3000));
    }
}

vector<Aircraft*> initplanes(const string& fileName) {
    ifstream file("/tmp/" + fileName);
    vector<Aircraft*> list;
    string line;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        stringstream ss(line);
        int t, id; float x, y, z, xs, ys, zs;
        if (ss >> t >> id >> x >> y >> z >> xs >> ys >> zs) {
            list.push_back(new Aircraft(t, id, x, y, z, xs, ys, zs));
        }
    }
    return list;
}

int main() {
    int shm_fd = shm_open("/Shared_mem", O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open"); exit(1); }
    sharedMem = (SharedMemory*) mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedMem == MAP_FAILED) { perror("mmap"); exit(1); }

    aircraftList = initplanes("test_1.txt");
    for (auto* a : aircraftList) a->disp();
    PSR_SSR psr_ssr(aircraftList);

    thread clockT(ClockThread);
    vector<thread> threads;
    for (auto* a : aircraftList) threads.emplace_back(AircraftLogic, a);
    thread psrT(PSRThread, ref(psr_ssr));
    thread ssrT(SSRThread, ref(psr_ssr));

    clockT.join(); for (auto& t : threads) t.join(); psrT.join(); ssrT.join();
    return 0;
}


//vector<Aircraft*> initplanes(string FileName);
//
//int main() {
//
//	// Shared memory not finished
////	int shm_fd = shm_open("/Shared_mem", O_RDWR, 0666);
////	if (shm_fd == -1) {
////	    perror("shm_open");
////	    exit(1);
////	}
////
////	SharedMemory* sharedMem = (SharedMemory*) mmap(
////	    NULL,
////	    sizeof(SharedMemory),
////	    PROT_READ | PROT_WRITE,
////	    MAP_SHARED,
////	    shm_fd,
////	    0
////	);
////
////	if (sharedMem == MAP_FAILED) {
////	    perror("mmap");
////	    exit(1);
////	}
//
//	//opens input file, reads the file, and initialize aircrafts
//	string FileName="test_1.txt";
//	vector<Aircraft*> aircraftList = initplanes(FileName);
//	for (int i=0; i<aircraftList.size();i++){
//		aircraftList[i]->disp(); //display aircrafts info
//
//	}
//
//
//	PSR_SSR psr_ssr(aircraftList);
//
//	//PSR signal store the position of aircrafts that are in range of the radar
//	vector <PSRData>psrData=psr_ssr.PSRsignal();
//	for (int i=0;i<psrData.size();i++){
//		cout<<"From PSR ==> ID: "<<psrData[i].id <<" | x = "<<psrData[i].x<<" |  y= "<<psrData[i].y<<" | z = "<<psrData[i].z<<endl;
//	}
//
//	//From the PSR vector, SSR radar communicate with each aircrafts in range to get their positions
//	vector <SSRData> ssrData=psr_ssr.SSRSignal(psrData);
//	for (int i=0;i<ssrData.size();i++){
//			cout<<"From SSR ==> ID: "<<ssrData[i].id <<" | x = "<<ssrData[i].x<<" |  y= "<<ssrData[i].y<<" | Fl = "<<ssrData[i].fl<<" | xspeed = "<< ssrData[i].xspeed<<" | yspeed = "<< ssrData[i].yspeed<<" | zspeed = "<< ssrData[i].zspeed<< endl;
//		}
//
//
//	return 0;
//}
//
////open input file + initialize aircrafts
//vector<Aircraft*> initplanes(string FileName){
//	ifstream InputFile;
//	InputFile.open("/tmp/resources/"+FileName);
//
//	if(!InputFile.is_open()){
//			cout<<"Failed to open file "+FileName<<endl;
//			return {};
//	}
//
//	else{
//	vector<Aircraft*> aircraftList;
//	string line;
//	while(getline(InputFile,line)){
//		if(line.empty() || line[0]=='#')
//			continue;
//		if(!line.empty() && line.back()=='\r'){
//			line.erase(line.size());
//		}
//
//		stringstream ss(line);
//		int time, id;
//		float xpos, ypos, zpos, xspeed, yspeed, zspeed;
//
//		if(ss>>time>>id>>xpos>>ypos>>zpos>>xspeed>>yspeed>>zspeed){
//			Aircraft* aircraft = new Aircraft(time,id,xpos,ypos,zpos,xspeed,yspeed,zspeed);
//			aircraftList.push_back(aircraft);
//		}
//		else
//			cout<<"Invalid line format: "<<line<<endl;
//	}
////	cout<<"Success"<<endl;
//	return aircraftList;
//	}
//
//}
//
////vector<array<float,4>> SSR(vector<Aircraft*>aircraftList){
////	for(int i=0; i<aircraftList.size();i++){
////		if(aircraftList[i]->getxpos()	)
////	}
////
////
////}





