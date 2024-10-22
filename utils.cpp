#include <iostream>
#include <sstream>
#include <windows.h>
#include <ctime>
#include "header/utils.h"

using namespace std;

string getCurrentTimestamp() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%m/%d/%Y, %I:%M:%S %p", &tstruct);
    return buf;
}

void setColor(unsigned char color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

pair<string, string> parseCommand(const string& input) {
	istringstream stream(input);
	string cmd, argument;
	stream >> cmd;
	getline(stream, argument);
	if (!argument.empty() && argument[0] == ' ') {
		argument = argument.substr(1);
	}
	return {cmd, argument};
}
