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
	char ch; // I/O �ʱ�ȭ
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
			if (ch == '\r'){ //����ڰ� ���͸� ���� ���
				if (buf == "/quit")	running = false; //����
				else if (buf.find_first_not_of(" \t\n\v\f\r") == std::string::npos){ //���鸸 ���� ���

				}
				else{
					IQ.add("<" + nick + "> " + buf);
				}
				buf = ""; //��ǲ ť�� ���۸� �־��ְ� ���� �ʱ�ȭ
				clearCurrentLine();
				toConsoleEnd();
				cout << "> ";
			}
			else if (ch == '\b'){
				if (buf.size() <= 0){ //���� ������ ���� ��
					cout << " ";
					continue; 
				}
				cout << ' '; //���� Ŀ���� ���� ����
				cout << '\b';//�ٽ� ��ĭ �ڷ� ��
				buf = buf.substr(0, buf.size() - 1);
			}
			else{
				buf += ch; //���ۿ� ��� �Է��� ���� �߰�
			}
		}

	}
}

unsigned WINAPI SocketThread(void* args){
	WSADATA wsa;
	SOCKET s;
	string temp;
	struct sockaddr_in server;
	const timeval tv = { 0, 0 }; //select �Լ��� ������ �ʵ��� �մϴ�.
	char buffer[16384];
	DWORD tempVal;
	int retry_count = MAX_RETRY;

	int result = WSAStartup(MAKEWORD(2,2), &wsa);
	if (result != 0){
		OQ.add("[E] WSAStartup�� �����Ͽ����ϴ�!");
		return -1;
	}
	s = socket(
		AF_INET,    //IPv4
		SOCK_STREAM,//��Ʈ�� ������ ����
		0);         // TCP/IP
	OQ.add("[I] ������ �������Դϴ�...");
	if (s == INVALID_SOCKET){
		OQ.add("[E] ���� ������ �����Ͽ����ϴ�!");
		OQ.add("[E] ���� : " + to_string(WSAGetLastError()));
	}
	
	server.sin_addr.s_addr = inet_addr(server_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(server_socket);
	if (connect(s, (struct sockaddr*) &server, sizeof(server)) < 0){
		OQ.add("[E] ���� ����:" + to_string(WSAGetLastError()));
		return -1;
	}

	fd_set fs;
	FD_ZERO(&fs);

	OQ.add("[I] ����Ǿ����ϴ�!");
	IQ.add(nick + " connected");
	while (running){
		while (!IQ.isEmpty()){
			temp = IQ.pop();
			if (send(s, (char*)temp.c_str(), temp.length(), 0) < 0){
				OQ.add("[E] ���ۿ� �����Ͽ����ϴ�:" + GetLastError());
			}
		}
		FD_SET(s, &fs);
		Sleep(5); //CPU ����ȭ ����
		switch((tempVal = select(0, &fs, NULL, NULL, &tv))){
		case 0: //���� �� ����
			break;
		case 1: //select �Լ��� fd_set ����ü ���� ��� ������ ������ ������ ����
				//�׷��Ƿ� 1�� �����ߴٸ� ������ ���� �غ� �Ǿ��ٴ� �ǹ���
			
			if ((tempVal = recv(s, buffer, BUF_SIZE - 1, 0)) == SOCKET_ERROR){
				while (retry_count--){
					OQ.add("[E] �����͸� �޾ƿ��� �� �����Ͽ����ϴ�!");
					switch ((tempVal = WSAGetLastError())){
					case 10054:
						OQ.add("[E] ����: ������ ������ϴ�.");
						return -1;
						break;
					case 10061:
						OQ.add("[E] ����: ������ ������ �ź��մϴ�.");
						OQ.add("[E] " + to_string(MAX_RETRY - retry_count) + "ȸ �õ�");
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
			OQ.add("[E] �� �� ���� ���� �߻��� : " + to_string(tempVal));
			break;
		}
		Sleep(5);//CPU ����ȭ ����
	}
}