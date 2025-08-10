//	main.cpp
#include "MIDIexample.h"
#include <iostream>
#include <conio.h>				// 因为getche函数
#include <windows.h>
#include <mmsystem.h>
#include <fstream>
using namespace std;

int main()
{
	int choice;
	// HMIDIOUT hMidiOut;							// 定义变量（句柄）
	// midiOutOpen(&hMidiOut, MIDIMAPPER, 0, 0, 0);// 打开MIDI播放设备
	//
	// HelloWorld(hMidiOut);
	// midiOutClose(hMidiOut);						// 关闭MIDI播放设备

	string Filename;
	ifstream inFile("..\\点歌.txt");
	if (inFile) {
		inFile >> Filename;
		inFile.close();
	}
	string Filepath = "..\\music\\" + Filename + ".txt";

	// freopen("COM", "r", stdin);
	char op;
	cout<<"请问是否需要预览简谱(Y/N)：";
	// cin.ignore();
	cin>>op;
	if (op=='Y' || op=='y') PRINT_INI(3,Filename,Filepath);

	while(true)
	{
		cout << "\n1 --- Right\n2 --- Left\n3 --- Both\n0 --- exit : ";
		choice = getche();
		cout << endl;
		if(choice<='0') break;
		switch(choice)
		{
		case '1':	Test(1,Filename,Filepath);	break;
		case '2': 	Test(2,Filename,Filepath);	break;
		default:	Test(3,Filename,Filepath);	break;
		}
	}
	return 0;
}
