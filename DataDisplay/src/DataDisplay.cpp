#include <iostream>
#include <unistd.h>
#include <vector>
#include <math.h>
#include <sstream>
#include <thread>
#include "CUtils.h"
#include "../../Shared_mem/src/Shared_mem.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <cstring>
#include <cstdlib>

using namespace std;

struct Aircraft {
	int mId = -1;
	int mEntranceTime = -1;
	int mSpeedX = -1; //ft/s
	int mSpeedY = -1;
	int mSpeedZ = -1;
	int mPosX = -1 ; //from the bottom right corner
	int mPosY = -1;
	int mPosZ = -1;
	Aircraft()
	{

	}
	Aircraft(int i, int t, int sx, int sy, int sz, int px, int py, int pz)
	{
		mId = i;
		mEntranceTime = t;
		mSpeedX = sx;
		mSpeedY = sy;
		mSpeedZ = sz;
		mPosX = px;
		mPosY = py;
		mPosZ = pz;
	}
	int CalcExitTime()
	{
		return 25;
	}
	int CalcAOA()
	{
		return (int)(atan((float)(mSpeedZ)/(sqrt((float)(mSpeedX * mSpeedX) + (float)(mSpeedY * mSpeedY)))));
	}
	int CalcSOG()
	{
		return sqrt((float)(mSpeedX * mSpeedX) + (float)(mSpeedY * mSpeedY));
	}
};

vector<Aircraft> gvecAircraftList;
const int Y = 40;
const int X = 100;
const int MAPX = 30;
const int MAPY = 60;
//Frame starts at 2,2; map starts at 3,3
const int MAPTLX = 3;
const int MAPTLY = 3;
const int LISTTLX = 2 + MAPTLY + MAPY;
const int LISTTLY = 2;
const int LISTX = 32;
const int LISTY = 36;
char garrScreen[X][Y];

int gTime = 0;
bool gBreak = false;
int gId = -1;
int gMode = 0;
SharedMemory* sharedMem;

void StopDisplay() { gBreak = true; };

int WriteToScreen(DISP aDisplayMode);
int PrintScreen();
int LoadAircraftFromMemory();
//Current global time from Computer Module;
int DisplayMain(int aT);
int DisplayDetail(int aId, int aT);
int DisplaySplash();

//CALL BEFORE SKELETON()
//THIS FUNCTION IS OVERKILL
void ClearList();
void ListCLI();
void ListDetail(int aId);
void MapDetail(int aId); //Highlights a specific plane

void Skeleton();
void Map();
void List(bool aPopulate = true);
void Prompt();

char RoundAngle(float a, bool &n);
Aircraft FindACById(int aId);
int ProcessCommand(string aStr);
//DOES NOT MODIFY garrScreen[]!!!!!!
void ClearScreen();
void UpdateTime();
void Run();
void ChangeDisplayMode(int aMode, int aId=-1);
void CheckMode();

//Clone this and the display + cmd logic to Console instead of DataDisplay
//
int main() {
	//LoadAircraftFromMemory();
	//WriteToScreen(DISP::BLANK);
	//PrintScreen();
	//sleep(1);
	//DisplayMain(2);
	//DisplayDetail(2, 5);
	//DisplaySplash();
	int shm_fd = shm_open("/Shared_mem", O_RDWR, 0666);
	    if (shm_fd == -1) { perror("shm_open"); exit(1); }
	    sharedMem = (SharedMemory*) mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	    if (sharedMem == MAP_FAILED) { perror("mmap"); exit(1); }

	ChangeDisplayMode(0);
	thread display(Run);
	display.detach();
	for (;;)
		sleep(1);
}

//0=standard
//1=detail
void ChangeDisplayMode(int aMode, int aId)
{
	if (aMode == 0)
	{
		gId = -1;
		gMode = 0;
	}
	else if (aMode == 1)
	{
		gId = aId;
		gMode = 1;
	}
}

void CheckMode()
{
    pthread_mutex_lock(&sharedMem->displayConsoleMutex);
    gMode = (sharedMem->displayDetailId == -1) ? 0 : 1;
    gId = (sharedMem->displayDetailId == -1) ? -1 : sharedMem->displayDetailId;
    pthread_mutex_unlock(&sharedMem->displayConsoleMutex);
}

void Run()
{
	//Update datadisplay every second, no need for prompting or anything
	//Check aircraft data, plot aircraft
	for(;;)
	{
		ClearScreen();
		WriteToScreen(DISP::BLANK);
		UpdateTime();
		gvecAircraftList.clear();
		LoadAircraftFromMemory();
		CheckMode();
		if (gMode == 0)
			DisplayMain(gTime);
		else
			DisplayDetail(gId, gTime);
		sleep(1);
		if (gBreak)
			break;
	}
}

void UpdateTime()
{
	//gTime = shared_mem.access(time)
    pthread_mutex_lock(&sharedMem->simClock.clockMutex);
    gTime = sharedMem->simClock.currentTimeInSeconds;
    pthread_mutex_unlock(&sharedMem->simClock.clockMutex);
}

int LoadAircraftFromMemory()
{
	pthread_mutex_lock(&sharedMem->radarDataMutex);
	SSRData input;
	Aircraft output;
	for (int i = 0; i < sharedMem->ssrDataCount; i++)
	{
		//id x y z xs ys zs
		input = sharedMem->ssrData[i];
		//id t sx sy sz x y z
		//Aircraft enters when it can be seen on the radar
		output = Aircraft(input.id, gTime, (int)input.xspeed, (int)input.yspeed, (int)input.zspeed, (int)input.x, (int)input.y, (int)input.fl);
		gvecAircraftList.push_back(output);
	}
	pthread_mutex_unlock(&sharedMem->radarDataMutex);
	return 0;
}

int DisplaySplash()
{
	//Display Program name in MAP
	//Display CLI options in side menu (Refresh Map, View Aircraft Details, Send Command)
	//Refresh Map is DisplayMain()
	//View Aircraft Details is DisplayDetail()
	//Send Command is DisplayCommand()
	//Starting screen is DisplaySplash()
	ClearScreen();
	ClearList();
	Skeleton();
	List(false);
	ListCLI();
	PrintScreen();

	Prompt();
	string command;
	getline(cin, command);
	ClearScreen();
	ProcessCommand(command);
	return 0;
}

int DisplayMain(int aT)
{
	//Consider only redrawing map and list
	gTime = aT;

	//Construct The Skeleton of the UI Elements
	ClearList();
	Skeleton();
	Map();
	List();

	PrintScreen();
	//Prompt();
	//string command;
	//getline(cin, command);
	//ClearScreen();
	//ProcessCommand(command);
	return 0;
}

int DisplayDetail(int aId, int aT)
{
	gTime = aT;

	ClearList();
	Skeleton();
	//the code gets worse every hour
	List(false);
	ListDetail(aId);
	MapDetail(aId);

	PrintScreen();
	//Prompt();
	//string command;
	//getline(cin, command);
	//ClearScreen();
	//ProcessCommand(command);
	return 0;
}

//Need to handle command entry and sending
//DataDisplay does not auto-update, and can be updated with a command from the menu
//Print menu in list space

int WriteToScreen(DISP aDisplayMode)
{
	switch (aDisplayMode)
	{
	case DISP::BLANK:
		for (int y = 0; y < Y; y++)
			for (int x = 0; x < X; x++)
				garrScreen[x][y] = ' ';
		return 1;
	case DISP::HSTR:
		for (int y = 0; y < Y; y++)
		{
			for (int x = 0; x < X; x++)
			{
				garrScreen[x][y] = (x%2==0) ? ' ' : '|';
				//garrScreen[y][x] = 'I';
			}
		}
		return 3;
	default:
		return 2;
	}
	return -1;
}

int PrintScreen()
{
	for (int y = 0; y < Y; y++)
	{
		for (int x = 0; x < X; x++)
		{
			cout << garrScreen[x][y];
		}
		cout << endl;
	}
	return 0;
}

void Skeleton()
{
	//OUTER FRAME
	for (int i = 0; i < X; i++)
	{
		garrScreen[i][0] = '=';
		garrScreen[i][1] = ' ';
		garrScreen[i][Y-1] = '=';
		garrScreen[i][Y-2] = ' ';
	}
	for (int i = 0; i < Y; i++)
	{
		garrScreen[0][i] = '|';
		garrScreen[1][i] = ' ';
		garrScreen[X-1][i] = '|';
		garrScreen[X-2][i] = ' ';
	}
	garrScreen[0][0] = 'O';
	garrScreen[X-1][0] = 'O';
	garrScreen[0][Y-1] = 'O';
	garrScreen[X-1][Y-1] = 'O';

	//MAP FRAME
	for (int i = 2; i < 2 + MAPY; i++)
	{
		garrScreen[i][2] = '=';
		garrScreen[i][2 + MAPX] = '=';
	}
	for (int i = 2; i < 2 + MAPX; i++)
	{
		garrScreen[2][i] = '|';
		garrScreen[2 + MAPY][i] = '|';
	}
	garrScreen[2][2] = 'O';
	garrScreen[2 + MAPY][2] = 'O';
	garrScreen[2][2 + MAPX] = 'O';
	garrScreen[MAPY + 2][2 + MAPX] = 'O';
	garrScreen[2][MAPX + 4] = 'T';
	garrScreen[3][MAPX + 4] = 'i';
	garrScreen[4][MAPX + 4] = 'm';
	garrScreen[5][MAPX + 4] = 'e';
	garrScreen[6][MAPX + 4] = ':';
	garrScreen[7][MAPX + 4] = ' ';
	garrScreen[8][MAPX + 4] = (char)((gTime/10) + 48);
	garrScreen[9][MAPX + 4] = (char)((gTime%10) + 48);
	garrScreen[10][MAPX + 4] = 's';
}

void Map()
{
	//Fill in map background
	//Then populate with planes
	//Then add the direction arrows
	//and look at color?
	//int cY = 0;
	//int cX = 0;
	for (int y = MAPTLY; y < MAPTLY + MAPY - 1; y++)
	{
		for (int x = MAPTLX; x < MAPTLX + MAPX - 1; x++)
		{
			garrScreen[y][x] = ' ';
		}
	}

	for (auto ac : gvecAircraftList)
	{
		//Recall, the display is 2D
		//cout << ac.mPosX << " " << ac.mPosY << endl;
		int x = Remap(ac.mPosX, 100000, 0, 29, 0);
		int y = Remap(ac.mPosY, 100000, 0, 59, 0);
		garrScreen[y + 2][x + 2] = (char)(ac.mId + 48);
		//Direction Arrow
		float angle = atan2(ac.mSpeedY, ac.mSpeedX) * 180 / 3.1415926;
		bool isNeg = false;
		//cout << angle << endl;
		char v = RoundAngle(angle, isNeg);
		switch (v)
		{
		case '>':
			garrScreen[y+3][x+2] = v;
			break;
		case '/':
			if (isNeg)
			{
				garrScreen[y+3][x+1] = v;
			}
			else
			{
				garrScreen[y+1][x+3] = v;
			}
			break;
		case '^':
			garrScreen[y+2][x+1] = v;
			break;
		case '\\':
			if (isNeg)
			{
				garrScreen[y+1][x+1] = v;
			}
			else
			{
				garrScreen[y+3][x+3] = v;
			}
			break;
		case '<':
			garrScreen[y+1][x+2] = v;
			break;
		case 'V':
			garrScreen[y+2][x+3] = v;
		}
	}
}

char RoundAngle(float a, bool & neg)
{
	//Between -22 and 22
	if (abs(a) <= 22.5)
	{
		return '>';
	}
	//between 22 and 67.5
	else if (a > 22.5 && a <= 90 - 22.5)
	{
		return '\\';
	}
	else if (a > 90-22.5 && a <= 90 + 22.5)
	{
		return '^';
	}
	else if (a > 90 + 22.5 && a <= 180-22.5)
	{
		return '/';
	}
	else if (abs(a) > 135 + 22.5)
	{
		return '<';
	}
	else if (a < -22.5 && a >= -45 - 22.5)
	{
		neg = true;
		return '/';
	}
	else if (a < -45 - 22.5 && a >= -90-22.5)
	{
		neg = true;
		return 'V';
	}
	else if (a < -90-22.5 && a >= -135-22.5)
	{
		neg = true;
		return '\\';
	}
	return '?';
}

void List(bool aPopulate)
{

	garrScreen[LISTTLX][LISTTLY] = 'G';
	for (int i = LISTTLX; i < LISTTLX + LISTX; i++)
	{
		garrScreen[i][LISTTLY] = '=';
		garrScreen[i][LISTTLY + LISTY - 1] = '=';
	}
	for (int i = LISTTLY; i < LISTTLY + LISTY; i++)
	{
		garrScreen[LISTTLX][i] = '|';
		garrScreen[LISTTLX + LISTX - 1][i] = '|';
	}
	garrScreen[LISTTLX][LISTTLY] = 'O';
	garrScreen[LISTTLX + LISTX - 1][LISTTLY] = 'O';
	garrScreen[LISTTLX][LISTTLY + LISTY - 1] = 'O';
	garrScreen[LISTTLX + LISTX - 1][LISTTLY + LISTY - 1] = 'O';

	//cout << (aPopulate ? "true" : "false") << endl;

	if (!aPopulate)
	{
		return;
	}

	//Now Populate List
	int count = 0;
	for (auto ac : gvecAircraftList)
	{
		ListEntry l;
		l.id = ac.mId;
		l.xpos = ac.mPosX;
		l.ypos = ac.mPosY;
		l.timetoexit = 10;
		//cout << l.Stringify() << endl;
		std::string banner = "ID | POS (10k ft) | T_EXIT         ";
		for (int x = LISTTLX + 2; x < l.Stringify().size() + LISTTLX + 2; x++)
		{
			if (count == 0)
			{
				//first pass
				garrScreen[x][LISTTLY + 2 * count + 1] = banner[x - 2 - LISTTLX];
			}
			garrScreen[x][LISTTLY + 2 * count + 3] = l.Stringify()[x - 2 - LISTTLX];
		}
		count += 1;
	}
}

Aircraft FindACById(int aId)
{
	for (auto ac : gvecAircraftList)
	{
		//cout << "Aircraft: " << ac.mId << " : " << ac.mPosX << endl;
		//cout << "Searching For: " << aId << endl;
		if (ac.mId == aId)
		{
			return ac;
		}
	}
	return Aircraft();
}


void ListCLI()
{
	//First Line:
	int count = 0;
	vector<string> message = {"refresh - Refresh Map", "detail <id> - Show Details" , "message - Open Console"};
		for (int i = 0; i < message.size(); i++)
		{
			std::string banner = "CLI OPTIONS                ";
			for (int x = LISTTLX + 2; x < message.at(i).size() + LISTTLX + 2; x++)
			{
				if (count == 0)
				{
					//first pass
					garrScreen[x][LISTTLY + 2 * count + 1] = banner[x - 2 - LISTTLX];
				}
				garrScreen[x][LISTTLY + 2 * count + 3] = message.at(i)[x - 2 - LISTTLX];
			}
			count += 1;
		}
}

void SendCommandToComputer(int cmd)
{
	if (cmd == 0)
	{
		//cout << "Sending 'refresh' to CPU with target DataDisplay" << endl;
		DisplayMain(gTime);
	}
	else if (cmd == 2)
	{
		//cout << "Sending 'opencommand' to CPU with target DataDisplay" << endl;
	}
	else if (cmd % 4 == 0)
	{
		//cout << "Sending 'detail' " + to_string(cmd/4) + " to CPU with target DataDisplay" << endl;
		DisplayDetail(cmd/4, gTime);
	}
	else
	{
		//cout << "EPIC FAIL" << endl;
	}
}

void Prompt()
{
	cout << "Enter Command \t >";
}

int ProcessCommand(string aStr)
{
	//break up aStr
	stringstream ss(aStr);
	vector<string> lvecCmd;
	string tmp;
	//cout << "COMMAND: " << aStr << endl;
	while(getline(ss, tmp, ' '))
	{
		lvecCmd.push_back(tmp);
	}
	//refresh, details, aircraft command
	if (lvecCmd.at(0) == "refresh")
	{
		SendCommandToComputer(0);
	}
	else if (lvecCmd.at(0) == "detail")
	{
		SendCommandToComputer(4 * stoi(lvecCmd.at(1)));
	}
	else if (lvecCmd.at(0) == "command")
	{
		SendCommandToComputer(2);
	}
	else
	{
		return -1;
	}
	return 0;
}

void ClearScreen()
{
	for (int i = 0; i < 64; i++)
	{
		cout << endl;
	}
}

void ListDetail(int aId)
{
	Aircraft ac = FindACById(aId);
	if (ac.mId == -1)
	{
		cout << "NO AIRCRAFT WITH ID: " << aId << endl;
		return;
	}
	int count = 0;
		vector<string> message = {
				"POS X: " + to_string(ac.mPosX).substr(0,2) + "k ft",
				"POS Y: " + to_string(ac.mPosY).substr(0,2) + "k ft",
				"POS Z: " + to_string(ac.mPosZ).substr(0,2) + "k ft",
				"",
				"SPD X: " + to_string(ac.mSpeedX) + "mph",
				"SPD Y: " + to_string(ac.mSpeedY) + "mph",
				"SPD Z: " + to_string(ac.mSpeedZ) + "mph",
				"",
				"ENTRY: " + to_string(ac.mEntranceTime) + "s",
				"PROJ. EXIT: " + to_string(ac.CalcExitTime()) + "s",
				"",
				"AOA: " + to_string(ac.CalcAOA()) + "deg",
				"SOG: " + to_string(ac.CalcSOG()) + "mph"

		};
			for (int i = 0; i < message.size(); i++)
			{
				std::string banner = "DETAILS OF AIRCRAFT: " + to_string(aId);
				if (count == 0)
				{
					for (int x = LISTTLX + 2; x < banner.size() + LISTTLX + 2; x++)
					{
						garrScreen[x][LISTTLY + 2 * count + 1] = banner[x - 2 - LISTTLX];
					}
				}
				for (int x = LISTTLX + 2; x < message.at(i).size() + LISTTLX + 2; x++)
				{
					garrScreen[x][LISTTLY + 2 * count + 3] = message.at(i)[x - 2 - LISTTLX];
				}
				count += 1;
			}
	//Details:
	//Position, Speed, Entrance Time, Exit Time

}

void ClearList()
{
	for (int i = LISTTLY; i < LISTTLY + LISTY; i++)
	{
		for (int x = LISTTLX; x < LISTX + LISTTLX; x++)
		{
			garrScreen[x][i] = ' ';
		}
	}
}

void MapDetail(int aId)
{
	for (int y = MAPTLY; y < MAPTLY + MAPY - 1; y++)
		{
			for (int x = MAPTLX; x < MAPTLX + MAPX - 1; x++)
			{
				garrScreen[y][x] = ' ';
			}
		}

		for (auto ac : gvecAircraftList)
		{
			//Recall, the display is 2D
			//cout << ac.mPosX << " " << ac.mPosY << endl;
			int x = Remap(ac.mPosX, 100000, 0, 29, 0);
			int y = Remap(ac.mPosY, 100000, 0, 59, 0);
			if (ac.mId == aId)
			{
				garrScreen[y + 2][x + 2] = 'A';
			}
			else
			{
				garrScreen[y + 2][x + 2] = '0';
			}

			//Direction Arrow
			float angle = atan2(ac.mSpeedY, ac.mSpeedX) * 180 / 3.1415926;
			bool isNeg = false;
			//cout << angle << endl;
			char v = RoundAngle(angle, isNeg);
			switch (v)
			{
			case '>':
				garrScreen[y+3][x+2] = v;
				break;
			case '/':
				if (isNeg)
				{
					garrScreen[y+3][x+1] = v;
				}
				else
				{
					garrScreen[y+1][x+3] = v;
				}
				break;
			case '^':
				garrScreen[y+2][x+1] = v;
				break;
			case '\\':
				if (isNeg)
				{
					garrScreen[y+1][x+1] = v;
				}
				else
				{
					garrScreen[y+3][x+3] = v;
				}
				break;
			case '<':
				garrScreen[y+1][x+2] = v;
				break;
			case 'V':
				garrScreen[y+2][x+3] = v;
			}
		}
}
