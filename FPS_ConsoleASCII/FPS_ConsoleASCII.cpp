#include <iostream>
using namespace std;

#include <Windows.h>

//setting console to known set of dimensions
int nScreenWidth = 120; //columns
int nScreenHeight = 40; //rows

//cant use int for player location, would cause clucky movement from tile to tile, with no progression
float fPlayerX = 0.0f; //x pos
float fPlayerY = 0.0f; //y pos
float fPlayerA = 0.0f; //angle player is looking at 

//Map Constants, 2D array; hash symbol for wall and period for an empty space; allows game to infer as it doesn't know about polygons
int nMapHeight = 16;
int nMapWidth = 16;

int main()
{
	//Create Screen Buffer (creates a screen of wchar_t type(?), trying to use only unicode)
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight]; //2D Array
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL); //handle to console, is a textmode_buffer (?)
	SetConsoleActiveScreenBuffer(hConsole); //tell buffer it'll be the target of the console
	DWORD dwBytesWritten = 0; //(?) & CreateConsoleScreenBuffer(?)

	//Game Loop
	while (1)
	{
		//To Write to Screen
		screen[nScreenWidth * nScreenHeight - 1] = '\0'; //sets final char of array to esc, so it knows when to stop outputting the string 
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); //WriteConsoleOutputCharacter(handle, buffer, howManyBytes, { x,y of where text to be written, 0,0 is top left corner }, neededWinVar);
	}

	return 0;
}


