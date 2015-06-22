#pragma comment(lib, "ws2_32.lib")
#include "stdafx.h"
#include "MTQueue.h"
#include "ConsoleIO.h"
#include <WinSock2.h>
#include <conio.h>
#include <process.h>

#define DEFAULT_SOCKET 20248
#define MAX_RETRY 3
#define BUF_SIZE 16384
using namespace std;

string nick;
char server_addr[16];
int server_socket = DEFAULT_SOCKET;
int currentLine=0;
Queue<string> IQ; //InputQueue(main->sock)
Queue<string> OQ; //OutputQueue(sock->main)
bool running = true;


unsigned WINAPI SocketThread(void* args);

int main(int argc, char* argv[]){

	
	if (argc == 2){
		//make server
	}
	if (argc < 3){
		printf("Usage: chat <ip> <nick>");
		return -1;
	}

	string buf = ""; 
	char ch; // I/O 초기화
	clearScreen();
	setBackground(BLACK, WHITE);
	setColor(BLACK, WHITE);

	strncpy_s(server_addr, argv[1], 16);
	nick = string(argv[2]);

	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, SocketThread, NULL, 0, NULL);
	//TODO: Make thread here
	
	while (running){
		while (!OQ.isEmpty())
			writeOutput(currentLine, OQ.pop());
		while (_kbhit()){
			ch = _getche();
			if (ch == '\r'){ //사용자가 엔터를 쳤을 경우
				if (buf == "/quit")	running = false; //종료
				else if (buf.find_first_not_of(" \t\n\v\f\r") == std::string::npos){ //공백만 있을 경우

				}
				else{
					IQ.add("<" + nick + "> " + buf);
				}
				buf = ""; //인풋 큐에 버퍼를 넣어주고 버퍼 초기화
				clearCurrentLine();
				toConsoleEnd();
				cout << "> ";
			}
			else if (ch == '\b'){
				if (buf.size() <= 0){ //지울 내용이 없을 때
					cout << " ";
					continue; 
				}
				cout << ' '; //현재 커서의 글자 지움
				cout << '\b';//다시 한칸 뒤로 감
				buf = buf.substr(0, buf.size() - 1);
			}
			else{
				buf += ch; //버퍼에 방금 입력한 글자 추가
			}
		}

	}
}

unsigned WINAPI SocketThread(void* args){
	WSADATA wsa;
	SOCKET s;
	string temp;
	struct sockaddr_in server;
	const timeval tv = { 0, 0 }; //select 함수가 멈추지 않도록 합니다.
	char buffer[16384];
	DWORD tempVal;
	int retry_count = MAX_RETRY;

	int result = WSAStartup(MAKEWORD(2,2), &wsa);
	if (result != 0){
		OQ.add("[E] WSAStartup이 실패하였습니다!");
		return -1;
	}
	s = socket(
		AF_INET,    //IPv4
		SOCK_STREAM,//스트림 형식의 소켓
		0);         // TCP/IP
	OQ.add("[I] 서버에 연결중입니다...");
	if (s == INVALID_SOCKET){
		OQ.add("[E] 소켓 생성에 실패하였습니다!");
		OQ.add("[E] 이유 : " + to_string(WSAGetLastError()));
	}
	
	server.sin_addr.s_addr = inet_addr(server_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(server_socket);
	if (connect(s, (struct sockaddr*) &server, sizeof(server)) < 0){
		OQ.add("[E] 연결 오류:" + to_string(WSAGetLastError()));
		return -1;
	}

	fd_set fs;
	FD_ZERO(&fs);

	OQ.add("[I] 연결되었습니다!");
	IQ.add(nick + " connected");
	while (running){
		while (!IQ.isEmpty()){
			temp = IQ.pop();
			if (send(s, (char*)temp.c_str(), temp.length(), 0) < 0){
				OQ.add("[E] 전송에 실패하였습니다:" + GetLastError());
			}
		}
		FD_SET(s, &fs);
		Sleep(5); //CPU 과부화 방지
		switch((tempVal = select(0, &fs, NULL, NULL, &tv))){
		case 0: //읽을 수 없음
			break;
		case 1: //select 함수는 fd_set 구조체 안의 사용 가능한 소켓의 개수를 리턴
				//그러므로 1을 리턴했다면 소켓을 읽을 준비가 되었다는 의미임
			
			if ((tempVal = recv(s, buffer, BUF_SIZE - 1, 0)) == SOCKET_ERROR){
				while (retry_count--){
					OQ.add("[E] 데이터를 받아오는 데 실패하였습니다!");
					switch ((tempVal = WSAGetLastError())){
					case 10054:
						OQ.add("[E] 이유: 연결이 끊겼습니다.");
						return -1;
						break;
					case 10061:
						OQ.add("[E] 이유: 서버가 연결을 거부합니다.");
						OQ.add("[E] " + to_string(MAX_RETRY - retry_count) + "회 시도");
						break;
					default:
						wchar_t error_reason_wchar[1024];
						wstring error_reason;
						string error_reason_string;
						FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, tempVal, MAKELANGID(0x02, 0x02), error_reason_wchar, 1024, NULL);
						error_reason = wstring(error_reason_wchar);
						error_reason_string.assign(error_reason.begin(), error_reason.end());
						OQ.add(error_reason_string);
						break;
					}
				}
			}
			buffer[tempVal] = '\0';
			OQ.add(buffer);
			retry_count = MAX_RETRY;
			break;
		default:
			OQ.add("[E] 알 수 없는 오류 발생함 : " + to_string(tempVal));
			break;
		}
		Sleep(5);//CPU 과부화 방지
	}
}