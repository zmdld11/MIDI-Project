//	MIDIexample.cpp
#include "MIDIexample.h"
#include <iostream>
#include <ctime>
#include <windows.h>
#include <mmsystem.h>
#include <string>
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

double gettime(int restart) {				// ������Ĭ��ֵ�������ʾ���¼�ʱ
	// �����ۼƼ�ʱ
	const double c = 1.0/CLOCKS_PER_SEC;
	static clock_t t = clock();				// ��̬�ֲ���������һ�ε���ʱ��ȷ����ʱ���
	if(restart) t = clock();				// ����ʵ�ξ����Ƿ�����ȷ����ʱ���
	return c*(clock()-t);					// ����һ��ʱ�㵽������������ʱ��
}

DWORD MidiOutMessage(HMIDIOUT hMidi, int iStatus, int iChannel, int iFlip, int iVolume) {
	DWORD dwMessage = (iVolume << 16) | (iFlip << 8) | iStatus         | iChannel;
	//      ����      |     ����     | ״̬�ֽ�(��4λ) | ͨ��(��4λ)
	return midiOutShortMsg(hMidi, dwMessage);
}

void HelloWorld(HMIDIOUT hMidiOut) {
	MidiOutMessage(hMidiOut, 0xC0, 0, 0, 0);	// ����ͨ��0����ɫΪ����

	MidiOutMessage(hMidiOut, 0x90, 0, 60, 112);	// ��������C(С��1��c1)������112(ff)
	Sleep(500);
	MidiOutMessage(hMidiOut, 0x80, 0, 60, 127);	// ��������C(С��1��c1)���ٶ�127��������

	MidiOutMessage(hMidiOut, 0x90, 0, 64, 80);	// ����e1������80(mf)
	Sleep(500);
	MidiOutMessage(hMidiOut, 0x80, 0, 64, 127);	// ����e1

	MidiOutMessage(hMidiOut, 0x90, 0, 67, 96);	// ����g1������96(f)
	Sleep(500);
	MidiOutMessage(hMidiOut, 0x80, 0, 67, 127);	// ����g1

	MidiOutMessage(hMidiOut, 0x90, 0, 60, 80);	// ���ң�����80(mf)
	MidiOutMessage(hMidiOut, 0x90, 0, 64, 80);
	MidiOutMessage(hMidiOut, 0x90, 0, 67, 80);
	MidiOutMessage(hMidiOut, 0x90, 0, 72, 80);
	Sleep(2500);
	MidiOutMessage(hMidiOut, 0x80, 0, 60, 127);	// ����
	MidiOutMessage(hMidiOut, 0x80, 0, 64, 127);
	MidiOutMessage(hMidiOut, 0x80, 0, 67, 127);
	MidiOutMessage(hMidiOut, 0x80, 0, 72, 127);
}

// ���׹�����
class MusicList {
	public:
		int Delay=500;					// Ĭ�Ͻ����ӳ�(����/��)
		int defaultVolume = 100;		// ���Ĭ������
		vector <string> vec;			// �洢�����е�����
		// ���������
		void add(string s) { vec.push_back(s); }
		// �������
		void clear() { vec.clear(); }
		// ���ý����ӳ�
		void setDelay(int _Delay) { Delay=_Delay; }
		// ���ļ���ȡ����
		void readFile(string fileName = "") {
			clear();
			ifstream in(fileName);
			if (!in.is_open()) {
				cerr << "�޷����ļ�: " << fileName << endl;
				cout << "3����Զ��رմ��ڡ�";
				Sleep(3000);
				exit(0);
			}

			// ��ȡ��һ�У��ٶȺ�Ĭ������
			string line;
			if (getline(in, line)) {
				istringstream iss(line);
				iss >> Delay;  // ��ȡ�ٶ�
                // ���Զ�ȡĬ������
				if (iss >> defaultVolume) ;	// �ɹ���ȡ����ֵ
				else defaultVolume = 100;	// ֻ���ٶ�ֵ��ʹ��Ĭ������100
			}

			// ��ȡ��������
			while (getline(in, line)) {
				if (!line.empty() && line.back() == '\r') line.pop_back();
				if (line.empty()) continue;
				add(line);
			}
			in.close();
		}
};
bool isNumeric(string const &str) { return regex_match(str,regex("[(-|+)|][0-9]+")); }

// ���״�ӡ��
class SimplifiedScorePrinter {
	public:
		// �������зָ��С��
		vector<string> splitIntoBars(const string& line) {
			vector<string> bars;
			string bar;
			istringstream iss(line);
			string token;
			bool inChord = false;

			while (iss >> token) {
				// ������ҿ�ʼ/����
				if (token == "[" || token == "{")  inChord = true;
				else if (token == "]" || token == "}")  inChord = false;

				// С�ڷָ���
				if (token == "|" && !inChord) {
					if (!bar.empty()) {
						bars.push_back(bar);
						bar.clear();
					}
					continue;
				}

				// ��ӿո�ָ�
				if (!bar.empty()) bar += " ";
				bar += token;
			}

			// ������һ��С��
			if (!bar.empty()) {
				bars.push_back(bar);
			}

			return bars;
		}
		// ��������ӡ����С��
		void printBar(int segIdx, int barIdx, const string& rightBar, const string& leftBar) {
			cout << "\n[���� " << segIdx+1 << " - С�� " << barIdx+1 << "]" << endl;
			if (!rightBar.empty()) {
				cout << "R: " << formatNotes(rightBar) << endl;
			}
			if (!leftBar.empty()) {
				cout << "L: " << formatNotes(leftBar) << endl;
			}
		}

		// ����������ת��Ϊ������ʾ
		string convertToNoteName(const string& note) {
			if (note.empty()) return "";

			// ������ֹ��
			if (note == "0") return "R";

			// ��������ӳ��
			static const map<char, string> baseNotes = {
				{'1', "C"}, {'2', "D"}, {'3', "E"},
				{'4', "F"}, {'5', "G"}, {'6', "A"}, {'7', "B"}
			};

			char base = note[0];
			if (baseNotes.find(base) == baseNotes.end()) return note;

			string result = baseNotes.at(base);

			// ��������
			if (note.find('#') != string::npos) {
				result += "#";
			}

			// ����˶� - �����˶ȱ�ʾ
			int octave = 3; // C3��ʼ������C��C4��
			for (char c : note) {
				if (c == ',') octave--;  // ���Ž��Ͱ˶�
				else if (c == '^') octave++; // ���ַ����߰˶�
			}

			// ��Ӱ˶���Ϣ
			result += to_string(octave);
			return result;
		}

		// ��ʽ��������ʾ����ȷ�������
		string formatNotes(const string& notes) {
			string formatted;
			string currentNote;  // ��ǰ���ڴ��������

			for (size_t i = 0; i < notes.length(); i++) {
				char c = notes[i];

				// ��������֣�������ʼ��
				if (isdigit(c)) {
					// �����δ������������ȴ�����
					if (!currentNote.empty()) {
						formatted += convertToNoteName(currentNote);
						currentNote.clear();
					}
					currentNote += c;
					continue;
				}

				// ������������η���#��^��,�����ҵ�ǰ���ڴ�������
				if (!currentNote.empty() && (c == '#' || c == '^' || c == ',')) {
					currentNote += c;
					continue;
				}

				// �����������Ҳ�����������η������ߵ�ǰû�д�������
				if (!currentNote.empty()) {
					formatted += convertToNoteName(currentNote);
					currentNote.clear();
				}

				// ��ӵ�ǰ�ַ�
				formatted += c;
			}

			// �������һ������������У�
			if (!currentNote.empty()) {
				formatted += convertToNoteName(currentNote);
			}

			return formatted;
		}

		void Print(const MusicList& ml, int outputFlag = 3) {  // ���outputFlag����
			cout << "\n===== ���� =====";
			cout << "\n�ٶ�: " << ml.Delay << " ����/��\n";
			cout << "----------------\n";

			vector<vector<string>> rightHandBars;
			vector<vector<string>> leftHandBars;

			// ��������
			for (size_t i = 0; i < ml.vec.size(); i++) {
				if (ml.vec[i].empty() || isNumeric(ml.vec[i])) continue;

				if (i % 2 == 0) {
					rightHandBars.push_back(splitIntoBars(ml.vec[i]));
				} else {
					leftHandBars.push_back(splitIntoBars(ml.vec[i]));
				}
			}

			// ����С����
			size_t maxSegments = max(rightHandBars.size(), leftHandBars.size());
			rightHandBars.resize(maxSegments);
			leftHandBars.resize(maxSegments);

			// ��ӡ����
			for (size_t seg = 0; seg < maxSegments; seg++) {
				cout << "\n[���� " << seg + 1 << "]\n";

				size_t maxBars = max(
				                     rightHandBars[seg].size(),
				                     leftHandBars[seg].size()
				                 );

				for (size_t bar = 0; bar < maxBars; bar++) {
					cout << "\n*С�� " << bar + 1 << "*\n";
					// ����outputFlag�����Ƿ�������ֲ���
					if (outputFlag == 1 || outputFlag == 3) {
						cout << "R: ";
						if (bar < rightHandBars[seg].size()) cout << formatNotes(rightHandBars[seg][bar]);
						else cout << "(��ֹ)";
						cout << "\n";
					}
					// ����outputFlag�����Ƿ�������ֲ���
					if (outputFlag == 2 || outputFlag == 3) {
						cout << "L: ";
						if (bar < leftHandBars[seg].size()) cout << formatNotes(leftHandBars[seg][bar]);
						else cout << "(��ֹ)";
						cout << "\n";
					}
				}
			}
			cout << "\n===== ���� =====\n";
		}
};


class MusicPlayer {
	private:
		enum scale {
		    Rest=0,
		    C8=108,
		    B7=107,A7s=106,A7=105,G7s=104,G7=103,F7s=102,F7=101,E7=100,D7s=99, D7=98, C7s=97, C7=96,
		    B6=95, A6s=94, A6=93, G6s=92, G6=91, F6s=90, F6=89, E6=88, D6s=87, D6=86, C6s=85, C6=84,
		    B5=83, A5s=82, A5=81, G5s=80, G5=79, F5s=78, F5=77, E5=76, D5s=75, D5=74, C5s=73, C5=72,
		    B4=71, A4s=70, A4=69, G4s=68, G4=67, F4s=66, F4=65, E4=64, D4s=63, D4=62, C4s=61, C4=60,
		    B3=59, A3s=58, A3=57, G3s=56, G3=55, F3s=54, F3=53, E3=52, D3s=51, D3=50, C3s=49, C3=48,
		    B2=47, A2s=46, A2=45, G2s=44, G2=43, F2s=42, F2=41, E2=40, D2s=39, D2=38, C2s=37, C2=36,
		    B1=35, A1s=34, A1=33, G1s=32, G1=31, F1s=30, F1=29, E1=28, D1s=27, D1=26, C1s=25, C1=24,
		    B0=23, A0s=22, A0=21
		};
		const int C_Scale[7][7]= {{C1,D1,E1,F1,G1,A1,B1},
			{C2,D2,E2,F2,G2,A2,B2},
			{C3,D3,E3,F3,G3,A3,B3},
			{C4,D4,E4,F4,G4,A4,B4},
			{C5,D5,E5,F5,G5,A5,B5},
			{C6,D6,E6,F6,G6,A6,B6},
			{C7,D7,E7,F7,G7,A7,B7}
		};
		const int C_Scale_s[7][7]= {{C1s,D1s,-1,F1s,G1s,A1s,-1},
			{C2s,D2s,-1,F2s,G2s,A2s,-1},
			{C3s,D3s,-1,F3s,G3s,A3s,-1},
			{C4s,D4s,-1,F4s,G4s,A4s,-1},
			{C5s,D5s,-1,F5s,G5s,A5s,-1},
			{C6s,D6s,-1,F6s,G6s,A6s,-1},
			{C7s,D7s,-1,F7s,G7s,A7s,-1}
		};
		// ����ӳ���
		map<string, int> volumeMap = {
			{"ppp", -30}, {"pp", -20}, {"p", -10}, {"mp", -5},
			{"mf", 5}, {"f", 10}, {"ff", 20}, {"fff", 30}
		};
		HMIDIOUT handle;
		int Delay=500;
		int defaultVolume = 100;  // Ĭ������
	public:
		bool ENDMUSIC=0;
		int STOP;
		MusicPlayer() {
			midiOutOpen(&handle,0,0,0,CALLBACK_NULL);
		}
		~MusicPlayer() {
			midiOutClose(handle);
		}
		void setDefaultVolume(int vol) {
			defaultVolume = vol;
		}
		void setDelay(int _Delay) {
			Delay=_Delay;
		}
		mutex mu;
		int ttag=0;
		int tick1,tick2;
		void play_single(string s,bool isMain) {
			vector <int> nbuf;
			s=s+' ';
			int n=s.size();
			int ctn=32*21;
			int baseVolume = defaultVolume;  // ��������
			int currentVolume = defaultVolume;  // ��ǰ��������
			bool isM=0;
			bool isChord=0;
			nbuf.clear();
			int st=clock(),tick=0;
			for (int i=0; i<n; ++i) {
				if (ENDMUSIC) break;
				char c=s[i];
				switch (c) {
					case '[': case '{': { // ���ҿ�ʼ
						assert(isChord==0);
						isChord = true;
						// ���浱ǰ������Ϊ���һ�������
						baseVolume = currentVolume;
						// cout<<"baseVolume: "<<baseVolume<<endl;
						break;
					}
					case ']': case '}': { // ���ҽ���
						assert(isChord==1);
						isChord = false;
						// �ָ�����ǰ�Ļ�������
						baseVolume = defaultVolume;
						break;
					}
					case ' ': {
						if (!isChord) {
							if (!nbuf.empty()) {
								for (int i=0; i<(int)nbuf.size(); ++i) if (nbuf[i]!=0) midiOutShortMsg(handle,nbuf[i]);
								nbuf.clear();
								while ((clock()-st)*1000.0/CLOCKS_PER_SEC<Delay/32.0/21*(tick+ctn));
								tick+=ctn;
								ctn=32*21;
							}
						}
						// ����Ϊ���������������ں����ڣ�
						// cout<<currentVolume<<endl;
						currentVolume = baseVolume;
						break;
					}

					// ��������
					case '|': break;
					case '_': { ctn/=2; break; }
					case '.': { ctn*=1.5; break; }
					case '-': { ctn+=32*21; break; }

					// ��ǿ����
					case 'm': case 'M': { isM=1; break;}
					case 'p': case 'P': {
						if (isM) currentVolume -= 5;
						else currentVolume -=10;
						isM=0;
						break;
					}
					case 'f': case 'F': {
						if (isM) currentVolume += 5;
						else currentVolume +=10;
						isM=0;
						break;
					}
					// ��ֹ����
					case '0': { nbuf.push_back(Rest); break; }
					//���֣��������
					default: {
						assert(c>='1' && c<='7');
						int x=(int)c-49,lvl=3;
						bool isSharp=0;
						for (int j=i+1; j<n; ++j) {
							if (s[j]=='^') lvl++;
							else if (s[j]==',') lvl--;
							else if (s[j]=='#') isSharp=1;
							else break;
							i++;
						}
						int noteValue;
						if (isSharp)
							noteValue = C_Scale_s[lvl][x];
						else
							noteValue = C_Scale[lvl][x];
						// cout<<currentVolume<<" "<<noteValue<<endl;
						if (noteValue != -1)
							nbuf.push_back((currentVolume << 16) + (noteValue << 8) + 0x90);

						// ����������Ϊ���һ�������
						currentVolume = baseVolume;
						break;
					}
				}
				// cout<<endl;
			}
			if (isMain) tick1=tick;
			else tick2=tick;
			mu.lock();
			STOP++;
			mu.unlock();
			return;
		}
		void play(string s1,string s2="") {
			STOP=0;
			tick1=0;
			tick2=0;
			thread tune1(&MusicPlayer::play_single,this,s1,1); tune1.detach();
			thread tune2(&MusicPlayer::play_single,this,s2,0); tune2.detach();
			while (STOP<2) {
				// ��Ӷ����ӳٱ���æ�ȴ�
				this_thread::sleep_for(chrono::milliseconds(1));
			}
		}
		void playList(MusicList &m,int flag) {
			Delay = m.Delay;
			defaultVolume = m.defaultVolume;  // ���ô��ļ���ȡ��Ĭ������
			ENDMUSIC = false;
			for (int i=0; i<(int)m.vec.size() && !ENDMUSIC; ++i) {
				while (i<(int)m.vec.size() && (m.vec[i]=="" || isNumeric(m.vec[i]))) {
					if (isNumeric(m.vec[i])) setDelay(stoi(m.vec[i]));
					i++;
				}
				if (i >= (int)m.vec.size()) break;
				string s1=m.vec[i],s2="";
				if (i<(int)m.vec.size()-1 && m.vec[i+1]!="") s2=m.vec[i+1],i++;
				if (flag==1) s2="";			//ֻ��������
				else if (flag==2) s1="";	//ֻ��������
				play(s1,s2);			//������������
			}
		}
		void playByBar(MusicList &m, int flag, SimplifiedScorePrinter& printer) {
			Delay = m.Delay;
			defaultVolume = m.defaultVolume;
			ENDMUSIC = false;

			// �洢�ֶ�С��
			vector<pair<vector<string>, vector<string>>> segments;
			vector<string> currentRight, currentLeft;
			bool inRightHand = true;

			// ��������ΪС��
			for (const string& line : m.vec) {
				if (line.empty() || isNumeric(line)) continue;

				vector<string> bars = printer.splitIntoBars(line);
				if (inRightHand) {
					currentRight = bars;
				} else {
					currentLeft = bars;
					segments.emplace_back(currentRight, currentLeft);
					currentRight.clear();
					currentLeft.clear();
				}
				inRightHand = !inRightHand;
			}
			if (!currentRight.empty()) {
				segments.emplace_back(currentRight, vector<string>());
			}

			// ��С�ڲ���
			for (int segIdx = 0; segIdx < segments.size() && !ENDMUSIC; segIdx++) {
				auto& [rightBars, leftBars] = segments[segIdx];
				int maxBars = max(rightBars.size(), leftBars.size());

				for (int barIdx = 0; barIdx < maxBars && !ENDMUSIC; barIdx++) {
					string rightBar = barIdx < rightBars.size() ? rightBars[barIdx] : "";
					string leftBar = barIdx < leftBars.size() ? leftBars[barIdx] : "";

					// ��ӡ��ǰС��
					printer.printBar(segIdx, barIdx,
									(flag & 1) ? rightBar : "",
									(flag & 2) ? leftBar : "");

					// ���ŵ�ǰС��
					play((flag & 1) ? rightBar : "",
						 (flag & 2) ? leftBar : "");
				}
			}
		}
};

void PRINT_INI(int flag, string Filename, string Filepath){
	MusicPlayer mp;
	MusicList music;
	music.readFile(Filepath);
	// �ڲ���ǰ��ӡ����
	SimplifiedScorePrinter printer;
	cout<<"***��������������������������������������Ԥ������������������������������������***"<<endl;
	printer.Print(music, flag);  // ����flag����
	printf("\n\n");
}

void Test(int flag, string Filename, string Filepath) {		// �������װ������裨���ٰ���ǰ2С�ڣ�
	MusicPlayer mp;
	MusicList music;
	music.readFile(Filepath);
	cout << "\n\t\t"<<"����Ϊ�����ţ�"<<Filename<<"(���ٰ���)" << endl;
	// �ڲ���ǰ��ӡ����
	SimplifiedScorePrinter printer;
	gettime(1);
	mp.playByBar(music, flag, printer);  // ʹ���µ���С�ڲ��ź���
	cout << "��ʱ��: " << gettime() << "��" << endl;
}
