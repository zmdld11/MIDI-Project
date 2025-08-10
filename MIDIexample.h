#ifndef MIDI_EXAMPLE_H
#define MIDI_EXAMPLE_H
#include <windows.h>
#include <string>
#include <iostream>
#include <ctime>
#include <mmsystem.h>
#include <array>
#include <thread>
#include <conio.h>
#include <mutex>
#include <regex>
#include <string.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <assert.h>
#include <sstream>
#include <map>
using namespace std;

double gettime(int restart=0);
DWORD MidiOutMessage(HMIDIOUT hMidi, int iStatus, int iChannel, int iFlip, int iVolume);

struct Music
{
	char note, volume;
	double deltaTime, time;
};

// class SimplifiedScorePrinter {
// public:
// 	void Print(const MusicList& ml, int outputFlag = 3);
// 	vector<string> splitIntoBars(const string& line);  // 新增声明
// 	void printBar(int segIdx, int barIdx,              // 新增声明
// 				 const string& rightBar,
// 				 const string& leftBar);
// };

void PRINT_INI( int flag=3, std::string Filename="", string Filepath="");
void Test( int flag=3, std::string Filename="", string Filepath="");
void HelloWorld(HMIDIOUT hMidiOut);

#endif
