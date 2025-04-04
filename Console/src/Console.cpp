#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <unistd.h>
#include "../../Shared_mem/src/Shared_mem.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <cstring>
#include <cstdlib>
using namespace std;

//This message is sent to an aircraft to inform it to change it's speed
struct AircraftMessage {
	int mTargetId = -1;
	int mNewSpdX = -1;
	int mNewSpdY = -1;
	int mNewSpdZ = -1;
	//static int messageId = 0;
	AircraftMessage() {};
	AircraftMessage(int t, int x, int y, int z)
	{
		mTargetId = t;
		mNewSpdX = x;
		mNewSpdY = y;
		mNewSpdZ = z;
	}
};

int mode = 0; //0 = CLI, 1=aircraft CMD
SharedMemory* sharedMem;
AircraftMessage * messageToSend = new AircraftMessage();

int SendCommandToCPU(string, long param = -1);
int SendCommandToAircraft();
void MessagePrompt();
void ProcessCommand(string);
void ProcessMessage(string);
void Prompt();
void Run();

int main() {
	int shm_fd = shm_open("/Shared_mem", O_RDWR, 0666);
	    if (shm_fd == -1) { perror("shm_open"); exit(1); }
	    sharedMem = (SharedMemory*) mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	    if (sharedMem == MAP_FAILED) { perror("mmap"); exit(1); }
	thread console(Run);
	console.detach();
	//do other stuff here (like sleeping forever)
	for (;;) sleep(1);
}

//Commands:
//refresh - Sets DataDisplay to standard mode
//detail <id> - Sets DataDisplay to detailed view mode
//command - Sends a command to an aircraft to change speed
void Run()
{
	for (;;)
	{
		Prompt();
		string command;
		getline(cin, command);
		ProcessCommand(command);
		if (mode == 1)
		{
			MessagePrompt();
			string msg;
			getline(cin, msg);
			ProcessMessage(msg);
			SendCommandToAircraft();
			mode = 0;
		}
		else
		{
			//normal prompting
			Prompt();
			string command;
			getline(cin, command);
			ProcessCommand(command);
		}
	}
}

int SendCommandToCPU(string command, long param)
{
	if (command == "refresh")
	{
		cout << "Retuning Display To Big-Picture" << endl;
	    pthread_mutex_lock(&sharedMem->displayConsoleMutex);
	    sharedMem->displayDetailId = -1;
	    pthread_mutex_unlock(&sharedMem->displayConsoleMutex);
	}
	else if (command == "detail")
	{
		//set mode on datadisplay to 1, param
		cout << "Displaying details of aircraft: " << param << endl;
		pthread_mutex_lock(&sharedMem->displayConsoleMutex);
		sharedMem->displayDetailId = param;
		pthread_mutex_unlock(&sharedMem->displayConsoleMutex);
	}
	else if (command == "message")
	{
		AircraftCommand ac;
		ac.id = messageToSend->mTargetId;
		ac.xspeed = (float)messageToSend->mNewSpdX;
		ac.yspeed = (float)messageToSend->mNewSpdY;
		ac.zspeed = (float)messageToSend->mNewSpdZ;
		pthread_mutex_lock(&sharedMem->displayConsoleMutex);
		sharedMem->consoleCommand = ac;
		pthread_mutex_unlock(&sharedMem->displayConsoleMutex);
	}
	return 0;
}

void Prompt()
{
	cout << "Enter Command \t >";
}

void ProcessCommand(string aStr)
{
	//break up aStr
	stringstream ss(aStr);
	vector<string> lvecCmd;
	string tmp;
	cout << "COMMAND: " << aStr << endl;
	while(getline(ss, tmp, ' '))
	{
		lvecCmd.push_back(tmp);
	}
	//refresh, details, aircraft command
	if (lvecCmd.at(0) == "refresh")
	{
		if (lvecCmd.size() > 1)
		{
			SendCommandToCPU("refresh", stoi(lvecCmd.at(1)));
		}
		else
		{
			SendCommandToCPU("refresh");
		}
	}
	else if (lvecCmd.at(0) == "detail")
	{
		SendCommandToCPU("detail", stoi(lvecCmd.at(1)));
	}
	else if (lvecCmd.at(0) == "message")
	{
		//Update the display to be in message mode
		//SendCommandToCPU("message", 1);
		mode = 1;
		//Update the console to be in message mode
	}
}

int SendCommandToAircraft()
{
	//Send address of message
	SendCommandToCPU("message");
	//cout << (long)messageToSend << "   " << &messageToSend << endl;
	return 0;
}

void ProcessMessage(string msg)
{
	stringstream ss(msg);
	vector<string> lvecCmd;
	string tmp;
	//cout << "COMMAND: " << aStr << endl;
	while(getline(ss, tmp, ' '))
	{
		lvecCmd.push_back(tmp);
	}
	if (lvecCmd.size() != 4)
	{
		cout << "MALFORMED MESSAGE" << endl;
		return;
	}
	*messageToSend = AircraftMessage(stoi(lvecCmd.at(0)), stoi(lvecCmd.at(1)), stoi(lvecCmd.at(2)), stoi(lvecCmd.at(3)));
}

void MessagePrompt()
{
	cout << "Enter the Target ID Number and desired new speed (Space-Separated) (ID X Y Z) > ";
}
