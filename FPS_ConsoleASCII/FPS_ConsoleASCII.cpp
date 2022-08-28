#include <iostream>
#include <chrono>
using namespace std;

#include <Windows.h>

//setting console to known set of dimensions
int nScreenWidth = 120; //columns
int nScreenHeight = 40; //rows

float fPlayerX = 8.0f; //x pos, starting in mid of room (8,8)
float fPlayerY = 8.0f; //y pos
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

	//2 time points to measure duration  using chrono library
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();


	//Game Loop
	while (1)
	{
		//find how fast one frame is, using that to augment variables for movement; gives consistent movement regardless of flutuating CPU resource useage
		tp2 = chrono::system_clock::now(); //set tp2 to current time
		chrono::duration<float> elapsedTime = tp2 - tp1; //calculate duration between tp2 (current systemTime) and tp1 (previous systemTime)
		tp1 = tp2; //updating the previous time point (tp1) with current time point (tp2)
		float fElapsedTime = elapsedTime.count(); //get elapsedTime as a float to make calculations easier

		//Controls
		//Handle CCW Rotation
		//Win function to see state of any key; 0x8000 is asking "is the highest bit of that key there? if yes, key pressed"
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (0.8f) * fElapsedTime; //rotate counter-clockwise (decrease player angle); fElapsedTime for consistent movement experience

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (0.8f) * fElapsedTime; //rotate clockwise (increase player angle)

		//take prev calculated unit vetctor, multiplies it providing a magnitude (to move player), multiplied by fElapsedTime moves player consistently over frames
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime; //addition to move forward
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime; //subtraction to move backward
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
		}


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
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall); //extends unit vector to the length currently checking for
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

			//Calculate distance to ceiling and floor
			//nCeiling, subtract a proportion of the screenHeight relative to distanceToWall from the midpoint
			//as distanceToWall increases (subtraction gets smaller), thus a higher ceiling
			//nFloor is a mirror of the ceiling
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' '; 

			//as working with and in console, will be using 'extended unicode ASCII' equivalent hex values for shading symobls
			if (fDistanceToWall <= fDepth / 4.0f)		nShade = 0x2588; //player very close to wall, brigthest and most shaded character
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591; //player far from wall, least bright and least shaded character
			else										nShade = ' ';    //player too far from wall to see


			//drawing into column
			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y < nCeiling) //current cell being drawn to must be part of ceiling; shade in sky as ' '
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor) //must be wall; shade as '#' like map
					screen[y * nScreenWidth + x] = nShade;
				else //if neither ceiling nor wall, must be floor; shade as ' '
					screen[y * nScreenWidth + x] = ' ';
			}

		}

		//To Write to Screen
		screen[nScreenWidth * nScreenHeight - 1] = '\0'; //sets final char of array to esc, so it knows when to stop outputting the string 
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		//WriteConsoleOutputCharacter(handle, buffer, howManyBytes, { x,y of where text to be written, 0,0 is top left corner }, neededWinVar);
	}

	return 0;
}


