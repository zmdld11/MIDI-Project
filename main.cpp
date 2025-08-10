//	main.cpp
#include "MIDIexample.h"
#include <iostream>
#include <conio.h>				// ��Ϊgetche����
#include <windows.h>
#include <mmsystem.h>
#include <fstream>
using namespace std;

int main()
{
	int choice;
	// HMIDIOUT hMidiOut;							// ��������������
	// midiOutOpen(&hMidiOut, MIDIMAPPER, 0, 0, 0);// ��MIDI�����豸
	//
	// HelloWorld(hMidiOut);
	// midiOutClose(hMidiOut);						// �ر�MIDI�����豸

	string Filename;
	ifstream inFile("..\\���.txt");
	if (inFile) {
		inFile >> Filename;
		inFile.close();
	}
	string Filepath = "..\\music\\" + Filename + ".txt";

	// freopen("COM", "r", stdin);
	char op;
	cout<<"�����Ƿ���ҪԤ������(Y/N)��";
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
