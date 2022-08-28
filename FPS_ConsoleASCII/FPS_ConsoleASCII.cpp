using namespace std;

#include <iostream>
#include <chrono>
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <utility>
#include <stdio.h>

//setting console to known set of dimensions
int nScreenWidth = 120; //columns
int nScreenHeight = 40; //rows

//Map dimensions, 2D array; hash symbol for wall and period for an empty space
int nMapHeight = 16;
int nMapWidth = 16;

float fPlayerX = 7.0f; //x pos, starting in mid of room (8,8)
float fPlayerY = 6.0f; //y pos
float fPlayerA = 0.0f; //angle player is looking at 

float pi = 3.14159;
float fFOV = pi / 4.0;
float fDepth = 16.0f; 
float fSpeed = 5.0f;

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
	map += L"#########.......";
	map += L"#...............";
	map += L"#.......########";
	map += L"#.......##.....#";
	map += L"#.....####.....#";
	map += L"#..............#";
	map += L"###..........###";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.......########";
	map += L"#..............#";
	map += L"####...........#";
	map += L"#..............#";
	map += L"#......#########";
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
			fPlayerA -= (fSpeed * 0.75f) * fElapsedTime; //rotate counter-clockwise (decrease player angle); fElapsedTime for consistent movement experience

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (fSpeed * 0.75f) * fElapsedTime; //rotate clockwise (increase player angle)

		//take prev calculated unit vetctor, multiplies it providing a magnitude (to move player), multiplied by fElapsedTime moves player consistently over frames
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += cosf(fPlayerA) * fSpeed * fElapsedTime;; //addition to move forward
			fPlayerY += sinf(fPlayerA) * fSpeed * fElapsedTime;; //2nd ; means an "empty expression"

			//collision detection
			//converts player current coordinates into integer space and tests on map array; 1.0f == player in top left cell of map (cell 1)
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX -= cosf(fPlayerA) * fSpeed * fElapsedTime;; //"undoes" movement input (stops player) if hits a wall
				fPlayerY -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= cosf(fPlayerA) * fSpeed * fElapsedTime;; //subtraction to move backward
			fPlayerY -= sinf(fPlayerA) * fSpeed * fElapsedTime;;

			//collision detection
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += cosf(fPlayerA) * fSpeed * fElapsedTime;; //"undoes" movement input (stops player) if hits a wall
				fPlayerY += sinf(fPlayerA) * fSpeed * fElapsedTime;;
			}
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
			float fStepSize = 0.1f;	   //increment size for ray casting, decrease to increase
			float fDistanceToWall = 0; //resolution

			bool bHitWall = false;
			bool bBoundary = false;

			//to calculate the test point
			float fEyeX = cosf(fRayAngle); //Unit Vector for ray in player space, representing direction player is looking in
			float fEyeY = sinf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth) //sets boundaries to avoid never hitting a wall
			{
				fDistanceToWall += fStepSize;

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
					if (map.c_str()[nTestX * nMapWidth + nTestY] == '#') //converts 2D system into 1D for array; y coor * mapWidth + X, if contains # then have hit wall
					{
						bHitWall = true; //fDistanceToWall will contain last value it had, as this loop will exit

						//boundary detection; vector which accumlates all 4 corners of cell
						vector<pair<float, float>> p; //distance to 'perfect corner', dot product (angle between the 2 vectors) to sort based on distance

						for (int tx = 0; tx < 2; tx++) //4 corners per cell to test, so 2 tightly nested loops to give offsets
							for (int ty = 0; ty < 2; ty++)
							{
								//creates vector from perfectCorner (int corners offset from player pos)
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								//calculate magnitue of vector^ to know how far corner is from player
								float d = sqrt(vx*vx + vy*vy);
								//calculate dot product (representation of angle between ray being cast and vector of perfectCorner)
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								//assemble into pair, push into vector
								p.push_back(make_pair(d, dot));
							}

						//Sort Pairs from closest to farthest; sort(from beginning to end); lambda takes the 2 pairs and compares, sorting from closest point to furthest 
						sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) { return left.first < right.first; });
												
						//takes inverse cos of 2nd part of pair (dot product), gives angle between the 2 rays
						//if less than fBound, can assume ray hit boundary of cell
						//only need to test 2 or 3 as you'll never see all 4 corners of a cell in a "normal Cartesian space" projected normally
						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;

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

			if (bBoundary)	nShade = ' '; //Black it out

			//drawing into column
			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y <= nCeiling) //current cell being drawn to must be part of ceiling; shade in sky as ' '
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor) //must be wall; shade as '#' like map
					screen[y * nScreenWidth + x] = nShade;
				else //if neither ceiling nor wall, must be floor; shade as ' '
				{
					//Shade floor based on distance
					//distance to floor remains constant, shading constantly based on proportion of how far floor can be seen
					//using symbols to differentiate walls from floor, less confusing than using the same symbols
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade = '#'; //more "full and vibrant" symbol of set
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-'; //least "full and vibrant" symbol of set before floor cannot be seen
					else				nShade = ' ';
					screen[y * nScreenWidth + x] = nShade;
				}
			}

		}

		//Display Stats (default at 0,0); FPS (frequency=1/time)
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		//Display Map; goes through coordinates of map, directly puts them into screenBuffer; offest by 1 to not overwrite stats
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny + 1)*nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		//Player marker; based on int version of player coordinates
		//checks fPlayerA when looking the cardinal directions: down = 0, pi/2 = left, pi = up, and (3pi)/2 = right
		//checks for what fPlayerA is closest to between four direction four values^
		short playerMarker = ' ';
		if (cosf(fPlayerA) > sinf(fPlayerA) && cosf(fPlayerA) > sinf(fPlayerA + pi)) playerMarker = 0x2190; //left
		else if (sinf(fPlayerA + pi) > cosf(fPlayerA) && sinf(fPlayerA + pi) > cosf(fPlayerA + pi)) playerMarker = 0x2191; //up
		else if (cosf(fPlayerA + pi) > sinf(fPlayerA + pi) && cosf(fPlayerA + pi) > sinf(fPlayerA)) playerMarker = 0x2192; //right
		else playerMarker = 0x2193; //down

		screen[((int)fPlayerY + 1) * nScreenWidth + (int)(nMapWidth - fPlayerX)] = playerMarker;

		//To Write to Screen
		screen[nScreenWidth * nScreenHeight - 1] = '\0'; //sets final char of array to esc, so it knows when to stop outputting the string 
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		//WriteConsoleOutputCharacter(handle, buffer, howManyBytes, { x,y of where text to be written, 0,0 is top left corner }, neededWinVar);
	}

	return 0;
}


