#include <iostream>
using namespace std;

#include <Windows.h>

//setting console to known set of dimensions
int nScreenWidth = 120; //columns
int nScreenHeight = 40; //rows

float fPlayerX = 0.0f; //x pos
float fPlayerY = 0.0f; //y pos
float fPlayerA = 0.0f; //angle player is looking at 

//Map dimensions, 2D array; hash symbol for wall and period for an empty space
int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f; 

int main()
{
	//Create Screen Buffer (creates a screen of wchar_t type, trying to use only unicode)
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight]; //2D Array
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL); //handle to console, standard textmode_buffer
	SetConsoleActiveScreenBuffer(hConsole); //tell buffer it'll be the target of the console
	DWORD dwBytesWritten = 0;

	//Map (wstring bc unicode)
	wstring map;

	//using append symbol, can draw map line by line; easier to visualize and debug
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	//Game Loop
	while (1)
	{
		//as not considered with looking up and down, really a 2D problem, thus 2D arrays
		//only need to be concerned with one axis, whichever is going across the screen

		//tiles, player has limited FOV, mostly looking fwd; 
		//algo takes each column of console (x), and relating that to a ray cast within the FOV; 120 columns, 120 rays cast into scene
		//interested in how far does ray travel before it hits a surface (block tile); does this 120x for full FOV, end up w/ array full of distances
		for (int x = 0; x < nScreenWidth; x++) //will do a computation for each column on the screen for this reason^
		{
			//For each column, calculate the projected ray angle into world space
			//(fPlayerA - fFOV / 2.0f) takes playerAngle and tries to find startingAngle for FOV; divides FOV by 2 as player looking fwd in middle of FOV, 1/2 on either side of playerSight
			//((float)x / (float)nScreenWidth) chops into bits, here 120 as is width of screen 
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			//ray tracing by measuring incremental small distances from player until ray collides with wall block tile (increment "lands inside a wall" tile)
			float fDistanceToWall = 0;
			bool bHitWall = false;

			//to calculate the test point
			float fEyeX = sinf(fRayAngle); //Unit Vector for ray in player space, representing direction player is looking in
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth) //sets boundaries to avoid never hitting a wall
			{
				fDistanceToWall += 0.1f;

				//creating a line of a given distance, using unit vector^
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall); //extends unit vector to the length that we're currently checking for
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall); //can use int as edges of walls will be int; ex. cell at 1.5, we know it's 1, so can truncate

				//Test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true; 
					fDistanceToWall = fDepth; //just set distance to maxium depth
				}
				else
				{
					//Ray is inbounds so test to see if the ray cell is a wall block, individually
					if (map[nTestY * nMapWidth + nTestX] == '#') //converts 2D system into 1D for array; y coor * mapWidth + X, if contains # then have hit wall
					{
						bHitWall = true; //fDistanceToWall will contain last value it had, as this loop will exit
					}
				}
			}
		}

		//To Write to Screen
		screen[nScreenWidth * nScreenHeight - 1] = '\0'; //sets final char of array to esc, so it knows when to stop outputting the string 
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		//WriteConsoleOutputCharacter(handle, buffer, howManyBytes, { x,y of where text to be written, 0,0 is top left corner }, neededWinVar);
	}

	return 0;
}


