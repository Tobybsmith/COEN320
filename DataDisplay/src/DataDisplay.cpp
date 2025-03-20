#include <iostream>
#include <unistd.h>
#include <vector>
#include <math.h>
#include "CUtils.h"

using namespace std;

struct Aircraft {
	int mId;
	int mEntranceTime;
	int mSpeedX; //ft/s
	int mSpeedY;
	int mSpeedZ;
	int mPosX; //from the bottom right corner
	int mPosY;
	int mPosZ;
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

int gTime = -1;

int WriteToScreen(DISP aDisplayMode);
int PrintScreen();
int LoadAircraftFromMemory();
//Current global time from Computer Module;
int RefreshDisplay(int aT);

void Skeleton();
void Map();
void List();

char RoundAngle(float a, bool &n);

int main() {
	LoadAircraftFromMemory();
	WriteToScreen(DISP::BLANK);
	//PrintScreen();
	//sleep(1);
	RefreshDisplay(2);
	return 0;
}

int LoadAircraftFromMemory()
{
	gvecAircraftList.push_back(Aircraft(0, 5, 50, 0, 3, 20000, 20000, 10));
	gvecAircraftList.push_back(Aircraft(1, 5, -50, 0, 3, 50000, 50000, 10));
	gvecAircraftList.push_back(Aircraft(0, 5, 50, -21, 3, 40000, 20000, 10));
	gvecAircraftList.push_back(Aircraft(1, 5, -50, 13, 3, 35000, 80000, 10));
	gvecAircraftList.push_back(Aircraft(1, 5, -2, 8, 3, 70000, 18000, 10));
	gvecAircraftList.push_back(Aircraft(2, 5, 0, -10, 3, 99999, 99999, 10));
	return 0;
}

int RefreshDisplay(int aT)
{
	//Consider only redrawing map and list
	gTime = aT;

	//Construct The Skeleton of the UI Elements
	Skeleton();
	Map();
	List();

	PrintScreen();
	return 0;
}

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

	//Construct RADAR Map
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
		garrScreen[y + 2][x + 2] = 'A';
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

void List()
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
		int dog = 0;
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
