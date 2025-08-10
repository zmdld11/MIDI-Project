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

double gettime(int restart) {				// 参数带默认值，非零表示重新计时
	// 否则累计计时
	const double c = 1.0/CLOCKS_PER_SEC;
	static clock_t t = clock();				// 静态局部变量。第一次调用时，确定计时起点
	if(restart) t = clock();				// 根据实参决定是否重新确定计时起点
	return c*(clock()-t);					// 从上一计时点到现在所经历的时间
}

DWORD MidiOutMessage(HMIDIOUT hMidi, int iStatus, int iChannel, int iFlip, int iVolume) {
	DWORD dwMessage = (iVolume << 16) | (iFlip << 8) | iStatus         | iChannel;
	//      音量      |     音符     | 状态字节(高4位) | 通道(低4位)
	return midiOutShortMsg(hMidi, dwMessage);
}

void HelloWorld(HMIDIOUT hMidiOut) {
	MidiOutMessage(hMidiOut, 0xC0, 0, 0, 0);	// 设置通道0的音色为钢琴

	MidiOutMessage(hMidiOut, 0x90, 0, 60, 112);	// 开音中央C(小字1组c1)，力度112(ff)
	Sleep(500);
	MidiOutMessage(hMidiOut, 0x80, 0, 60, 127);	// 关音中央C(小字1组c1)，速度127（立即）

	MidiOutMessage(hMidiOut, 0x90, 0, 64, 80);	// 开音e1，力度80(mf)
	Sleep(500);
	MidiOutMessage(hMidiOut, 0x80, 0, 64, 127);	// 关音e1

	MidiOutMessage(hMidiOut, 0x90, 0, 67, 96);	// 开音g1，力度96(f)
	Sleep(500);
	MidiOutMessage(hMidiOut, 0x80, 0, 67, 127);	// 关音g1

	MidiOutMessage(hMidiOut, 0x90, 0, 60, 80);	// 和弦，力度80(mf)
	MidiOutMessage(hMidiOut, 0x90, 0, 64, 80);
	MidiOutMessage(hMidiOut, 0x90, 0, 67, 80);
	MidiOutMessage(hMidiOut, 0x90, 0, 72, 80);
	Sleep(2500);
	MidiOutMessage(hMidiOut, 0x80, 0, 60, 127);	// 关音
	MidiOutMessage(hMidiOut, 0x80, 0, 64, 127);
	MidiOutMessage(hMidiOut, 0x80, 0, 67, 127);
	MidiOutMessage(hMidiOut, 0x80, 0, 72, 127);
}

// 乐谱管理类
class MusicList {
	public:
		int Delay=500;					// 默认节拍延迟(毫秒/拍)
		int defaultVolume = 100;		// 添加默认音量
		vector <string> vec;			// 存储乐谱行的容器
		// 添加乐谱行
		void add(string s) { vec.push_back(s); }
		// 清空乐谱
		void clear() { vec.clear(); }
		// 设置节拍延迟
		void setDelay(int _Delay) { Delay=_Delay; }
		// 从文件读取乐谱
		void readFile(string fileName = "") {
			clear();
			ifstream in(fileName);
			if (!in.is_open()) {
				cerr << "无法打开文件: " << fileName << endl;
				cout << "3秒后将自动关闭窗口…";
				Sleep(3000);
				exit(0);
			}

			// 读取第一行：速度和默认音量
			string line;
			if (getline(in, line)) {
				istringstream iss(line);
				iss >> Delay;  // 读取速度
                // 尝试读取默认音量
				if (iss >> defaultVolume) ;	// 成功读取两个值
				else defaultVolume = 100;	// 只有速度值，使用默认音量100
			}

			// 读取乐谱内容
			while (getline(in, line)) {
				if (!line.empty() && line.back() == '\r') line.pop_back();
				if (line.empty()) continue;
				add(line);
			}
			in.close();
		}
};
bool isNumeric(string const &str) { return regex_match(str,regex("[(-|+)|][0-9]+")); }

// 简谱打印类
class SimplifiedScorePrinter {
	public:
		// 将乐谱行分割成小节
		vector<string> splitIntoBars(const string& line) {
			vector<string> bars;
			string bar;
			istringstream iss(line);
			string token;
			bool inChord = false;

			while (iss >> token) {
				// 处理和弦开始/结束
				if (token == "[" || token == "{")  inChord = true;
				else if (token == "]" || token == "}")  inChord = false;

				// 小节分隔符
				if (token == "|" && !inChord) {
					if (!bar.empty()) {
						bars.push_back(bar);
						bar.clear();
					}
					continue;
				}

				// 添加空格分隔
				if (!bar.empty()) bar += " ";
				bar += token;
			}

			// 添加最后一个小节
			if (!bar.empty()) {
				bars.push_back(bar);
			}

			return bars;
		}
		// 新增：打印单个小节
		void printBar(int segIdx, int barIdx, const string& rightBar, const string& leftBar) {
			cout << "\n[段落 " << segIdx+1 << " - 小节 " << barIdx+1 << "]" << endl;
			if (!rightBar.empty()) {
				cout << "R: " << formatNotes(rightBar) << endl;
			}
			if (!leftBar.empty()) {
				cout << "L: " << formatNotes(leftBar) << endl;
			}
		}

		// 将数字音符转换为音名表示
		string convertToNoteName(const string& note) {
			if (note.empty()) return "";

			// 处理休止符
			if (note == "0") return "R";

			// 基础音高映射
			static const map<char, string> baseNotes = {
				{'1', "C"}, {'2', "D"}, {'3', "E"},
				{'4', "F"}, {'5', "G"}, {'6', "A"}, {'7', "B"}
			};

			char base = note[0];
			if (baseNotes.find(base) == baseNotes.end()) return note;

			string result = baseNotes.at(base);

			// 处理升号
			if (note.find('#') != string::npos) {
				result += "#";
			}

			// 计算八度 - 修正八度表示
			int octave = 3; // C3开始（中央C是C4）
			for (char c : note) {
				if (c == ',') octave--;  // 逗号降低八度
				else if (c == '^') octave++; // 脱字符升高八度
			}

			// 添加八度信息
			result += to_string(octave);
			return result;
		}

		// 格式化音符显示，正确处理和弦
		string formatNotes(const string& notes) {
			string formatted;
			string currentNote;  // 当前正在处理的音符

			for (size_t i = 0; i < notes.length(); i++) {
				char c = notes[i];

				// 如果是数字（音符开始）
				if (isdigit(c)) {
					// 如果有未处理的音符，先处理它
					if (!currentNote.empty()) {
						formatted += convertToNoteName(currentNote);
						currentNote.clear();
					}
					currentNote += c;
					continue;
				}

				// 如果是音符修饰符（#、^、,）并且当前正在处理音符
				if (!currentNote.empty() && (c == '#' || c == '^' || c == ',')) {
					currentNote += c;
					continue;
				}

				// 如果不是数字也不是音符修饰符，或者当前没有处理音符
				if (!currentNote.empty()) {
					formatted += convertToNoteName(currentNote);
					currentNote.clear();
				}

				// 添加当前字符
				formatted += c;
			}

			// 处理最后一个音符（如果有）
			if (!currentNote.empty()) {
				formatted += convertToNoteName(currentNote);
			}

			return formatted;
		}

		void Print(const MusicList& ml, int outputFlag = 3) {  // 添加outputFlag参数
			cout << "\n===== 简谱 =====";
			cout << "\n速度: " << ml.Delay << " 毫秒/拍\n";
			cout << "----------------\n";

			vector<vector<string>> rightHandBars;
			vector<vector<string>> leftHandBars;

			// 解析乐谱
			for (size_t i = 0; i < ml.vec.size(); i++) {
				if (ml.vec[i].empty() || isNumeric(ml.vec[i])) continue;

				if (i % 2 == 0) {
					rightHandBars.push_back(splitIntoBars(ml.vec[i]));
				} else {
					leftHandBars.push_back(splitIntoBars(ml.vec[i]));
				}
			}

			// 对齐小节数
			size_t maxSegments = max(rightHandBars.size(), leftHandBars.size());
			rightHandBars.resize(maxSegments);
			leftHandBars.resize(maxSegments);

			// 打印乐谱
			for (size_t seg = 0; seg < maxSegments; seg++) {
				cout << "\n[段落 " << seg + 1 << "]\n";

				size_t maxBars = max(
				                     rightHandBars[seg].size(),
				                     leftHandBars[seg].size()
				                 );

				for (size_t bar = 0; bar < maxBars; bar++) {
					cout << "\n*小节 " << bar + 1 << "*\n";
					// 根据outputFlag决定是否输出右手部分
					if (outputFlag == 1 || outputFlag == 3) {
						cout << "R: ";
						if (bar < rightHandBars[seg].size()) cout << formatNotes(rightHandBars[seg][bar]);
						else cout << "(休止)";
						cout << "\n";
					}
					// 根据outputFlag决定是否输出左手部分
					if (outputFlag == 2 || outputFlag == 3) {
						cout << "L: ";
						if (bar < leftHandBars[seg].size()) cout << formatNotes(leftHandBars[seg][bar]);
						else cout << "(休止)";
						cout << "\n";
					}
				}
			}
			cout << "\n===== 结束 =====\n";
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
		// 音量映射表
		map<string, int> volumeMap = {
			{"ppp", -30}, {"pp", -20}, {"p", -10}, {"mp", -5},
			{"mf", 5}, {"f", 10}, {"ff", 20}, {"fff", 30}
		};
		HMIDIOUT handle;
		int Delay=500;
		int defaultVolume = 100;  // 默认音量
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
			int baseVolume = defaultVolume;  // 基础音量
			int currentVolume = defaultVolume;  // 当前音符音量
			bool isM=0;
			bool isChord=0;
			nbuf.clear();
			int st=clock(),tick=0;
			for (int i=0; i<n; ++i) {
				if (ENDMUSIC) break;
				char c=s[i];
				switch (c) {
					case '[': case '{': { // 和弦开始
						assert(isChord==0);
						isChord = true;
						// 保存当前音量作为和弦基础音量
						baseVolume = currentVolume;
						// cout<<"baseVolume: "<<baseVolume<<endl;
						break;
					}
					case ']': case '}': { // 和弦结束
						assert(isChord==1);
						isChord = false;
						// 恢复和弦前的基础音量
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
						// 重置为基本音量（除非在和弦内）
						// cout<<currentVolume<<endl;
						currentVolume = baseVolume;
						break;
					}

					// 音长控制
					case '|': break;
					case '_': { ctn/=2; break; }
					case '.': { ctn*=1.5; break; }
					case '-': { ctn+=32*21; break; }

					// 音强控制
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
					// 休止符号
					case '0': { nbuf.push_back(Rest); break; }
					//数字，输出声音
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

						// 和弦内重置为和弦基础音量
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
				// 添加短暂延迟避免忙等待
				this_thread::sleep_for(chrono::milliseconds(1));
			}
		}
		void playList(MusicList &m,int flag) {
			Delay = m.Delay;
			defaultVolume = m.defaultVolume;  // 设置从文件读取的默认音量
			ENDMUSIC = false;
			for (int i=0; i<(int)m.vec.size() && !ENDMUSIC; ++i) {
				while (i<(int)m.vec.size() && (m.vec[i]=="" || isNumeric(m.vec[i]))) {
					if (isNumeric(m.vec[i])) setDelay(stoi(m.vec[i]));
					i++;
				}
				if (i >= (int)m.vec.size()) break;
				string s1=m.vec[i],s2="";
				if (i<(int)m.vec.size()-1 && m.vec[i+1]!="") s2=m.vec[i+1],i++;
				if (flag==1) s2="";			//只播放右手
				else if (flag==2) s1="";	//只播放左手
				play(s1,s2);			//播放整个段落
			}
		}
		void playByBar(MusicList &m, int flag, SimplifiedScorePrinter& printer) {
			Delay = m.Delay;
			defaultVolume = m.defaultVolume;
			ENDMUSIC = false;

			// 存储分段小节
			vector<pair<vector<string>, vector<string>>> segments;
			vector<string> currentRight, currentLeft;
			bool inRightHand = true;

			// 解析乐谱为小节
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

			// 逐小节播放
			for (int segIdx = 0; segIdx < segments.size() && !ENDMUSIC; segIdx++) {
				auto& [rightBars, leftBars] = segments[segIdx];
				int maxBars = max(rightBars.size(), leftBars.size());

				for (int barIdx = 0; barIdx < maxBars && !ENDMUSIC; barIdx++) {
					string rightBar = barIdx < rightBars.size() ? rightBars[barIdx] : "";
					string leftBar = barIdx < leftBars.size() ? leftBars[barIdx] : "";

					// 打印当前小节
					printer.printBar(segIdx, barIdx,
									(flag & 1) ? rightBar : "",
									(flag & 2) ? leftBar : "");

					// 播放当前小节
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
	// 在播放前打印简谱
	SimplifiedScorePrinter printer;
	cout<<"***—————————————————简谱预览—————————————————***"<<endl;
	printer.Print(music, flag);  // 传递flag参数
	printf("\n\n");
}

void Test(int flag, string Filename, string Filepath) {		// 党啊，亲爱的妈妈（钢琴伴奏前2小节）
	MusicPlayer mp;
	MusicList music;
	music.readFile(Filepath);
	cout << "\n\t\t"<<"正在为您播放："<<Filename<<"(钢琴伴奏)" << endl;
	// 在播放前打印简谱
	SimplifiedScorePrinter printer;
	gettime(1);
	mp.playByBar(music, flag, printer);  // 使用新的逐小节播放函数
	cout << "总时长: " << gettime() << "秒" << endl;
}
