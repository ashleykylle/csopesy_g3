#ifndef UTILS_H
#define UTILS_H

#include <string>

using namespace std;

string getCurrentTimestamp();
void setColor(unsigned char color);
pair<string, string> parseCommand(const string& input);

#endif
