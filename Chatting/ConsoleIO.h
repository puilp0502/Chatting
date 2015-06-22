#include "stdafx.h"

#pragma region console_color
#define BLACK 0
#define DARK_BLUE 1
#define DARK_GREEN 2
#define CYAN 3
#define DARK_RED 4
#define PURPLE 5
#define GOLD 6
#define GREY 7
#define DARK_GREY 8
#define BLUE 9
#define GREEN 10
#define AQUA 11
#define RED 12
#define PINK 13
#define YELLOW 14
#define WHITE 15
#pragma endregion

void clearCurrentLine();
void gotoxy(short x, short y){
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x, y });
}
void gotoxy(COORD c){
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
void setColor(int color, int bgcolor){
	color &= 0xf;
	bgcolor &= 0xf;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (bgcolor << 4) | color);
}
void setBackground(int color, int bgcolor){
	DWORD num;
	color &= 0xf;
	bgcolor &= 0xf;
	FillConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
		(bgcolor << 4) | color,
		0xfffffffe,
		{ 0, 0 },
		&num);

}

COORD getCursorPos(){
	CONSOLE_SCREEN_BUFFER_INFO csinfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csinfo);
	return csinfo.dwCursorPosition;
}
COORD getConsoleSize(){
	CONSOLE_SCREEN_BUFFER_INFO csinfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csinfo);
	COORD ConsoleSize = {
		csinfo.srWindow.Right - csinfo.srWindow.Left,
		csinfo.srWindow.Bottom - csinfo.srWindow.Top
	};
	return ConsoleSize;
}
void toConsoleEnd(){
	CONSOLE_SCREEN_BUFFER_INFO csinfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csinfo);
	gotoxy(0, csinfo.srWindow.Bottom);
	
}
void writeOutput(int& currentLine, std::string str){
	CONSOLE_SCREEN_BUFFER_INFO csinfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csinfo);
	gotoxy(0, currentLine);
	int lines = str.size() / getConsoleSize().X + 1;
	if (currentLine >= getConsoleSize().Y){
		WORD cnt;
		for (cnt = 0; cnt < lines; cnt++){
			clearCurrentLine();
			printf("\n");
			currentLine--;
		}
	}
	if (str.find("[I]") == 0){
		setColor(GREEN, WHITE);
	}
	else if (str.find("[E]") == 0){
		setColor(RED, WHITE);
	}
	std::cout << str << std::endl;
	currentLine = (getCursorPos().Y);
	setColor(BLACK, WHITE);
	toConsoleEnd();
	std::cout << "> ";
	gotoxy(2, csinfo.srWindow.Bottom);
}
void clearCurrentLine(){
	short i;
	gotoxy(0, getCursorPos().Y);
	for (i = 0; i < getConsoleSize().X-1; i++)
		std::cout << " ";
}
void clearScreen(){
	system("cls");
}

