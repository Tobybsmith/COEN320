#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "CUtils.h"
#include "Aircraft.h"

using namespace std;

vector<string> mvecFileContent;
vector<Aircraft> mvecAircraft;

RETVAL LoadFile(string aFileName);
RETVAL InitPlaneList();
RETVAL InitPlaneFromLine(string aLine);

int main() {
	if (LoadFile("resources/test_1.atc") != RETVAL::OK)
	{
		cout << "UNABLE TO OPEN ATC FILE" << endl;
		return 1;
	}
	if (InitPlaneList() != RETVAL::OK)
	{
		cout << "UNABLE TO INITALIZE PLANE LIST" << endl;
		return 1;
	}
}

//DO NOT VALIDATE INPUT HERE, NOT EVEN CUTTING COMMENTS
RETVAL LoadFile(string aFileName)
{
	ifstream file;
	file.open(aFileName);
	if (!file.is_open())
	{
		return RETVAL::IO;
	}
	string temp;
	while (getline(file, temp))
	{
		mvecFileContent.push_back(temp);
	}
	return RETVAL::OK;
}

//SANITIZE FILE CONTENTS HERE
RETVAL InitPlaneList() {
	int lines = mvecFileContent.size();
	try
	{
		mvecAircraft.resize(lines);
	}
	catch (...)
	{
		return RETVAL::MEM;
	}
	for (auto line : mvecFileContent)
	{
		if (line[0] == '#')
		{
			continue;
		}
		InitPlaneFromLine(line);
	}
	return RETVAL::OK;
}

RETVAL InitPlaneFromLine(string aLine)
{
	//TODO
	return RETVAL::MISC;
}
