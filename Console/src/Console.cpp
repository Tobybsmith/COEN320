#include <iostream>
#include <string>
#include <sstream>
#include <vector>
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

AircraftMessage * messageToSend = new AircraftMessage();

int SendCommandToCPU(string, long param = -1);
int SendCommandToAircraft();
void MessagePrompt();
void ProcessCommand(string);
void ProcessMessage(string);
void Prompt();

int main() {
	//Call Display.DisplaySplash()
	//Then, do Prompt() + cmd
	//Then, ONLY MAKE DISPLAY CALLS to DataDisplay
	SendCommandToCPU("splash");
	Prompt();
	string command;
	getline(cin, command);
	ProcessCommand(command);
	//await CommandProcessed == true
	//DataDisplay -> Prompt -> Update DataDisplay
	//ProcessCommand will send the command to CPU
	while (true)
	{
		//If the user is writing an aircraft message
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
	return 0;
}

//Commands the DataDisplay can process:
//refresh
//refresh <alt>
//detail <id>
//message
//splash - only used on startup and not user-accessible
int SendCommandToCPU(string command, long param)
{
	if (command == "refresh")
	{
		//called w/o alt
		if (param == -1)
		{
			cout << "Sending refresh to CPU with target DataDisplay" << endl;
		}
		else
		{
			cout << "Sending refresh(" << to_string(param) << ")  to CPU with target DataDisplay" << endl;
		}
	}
	else if (command == "detail")
	{
		cout << "Sending detail(" << param << ") to CPU with target DataDisplay" << endl;
	}
	else if (command == "message")
	{
		//maybe send pointer to aircraft_message object?
		cout << "Sending message(" << messageToSend << ") to CPU with target Comms" << endl;
	}
	else if (command == "splash")
	{
		cout << "Sending splash to CPU with target DataDisplay" << endl;
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
	SendCommandToCPU("message", (long)messageToSend);
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
