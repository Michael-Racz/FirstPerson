//File: Project2-Fp
//Name: Michael Racz
//Date: 01/06/2020
//Desc: With drawing a 2d map in lines ~48-63 makes a 3d space to walk around in. ALso has a map to tell you where you are in the map in the top hand screen.
//      Map size can be changed. Has score display and X,Y, and angle A display
//
//		Use A D to turn left and right, W S to go forwards and backwards. cannot move through walls.
//
#include <iostream>
#include <chrono>
#include <vector>
#include <utility>
#include <algorithm>
using namespace std;
#include <Windows.h>
#include <stdio.h>




int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 3.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4;
float fDepth = 16.0f;

int main() {

	//Create Screen Buffer?
	// screen via 2 dimensional array
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	//Regular text mode buffer
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	//Tell buffer that it is going to be our console
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.....#........#";
	map += L"#....#.........#";
	map += L"#...#..........#";
	map += L"#...........#..#";
	map += L"#..............#";
	map += L"#...........#..#";
	map += L"#..............#";
	map += L"#.....#........#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.......########";
	map += L"#..............#";
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();


	//GAME LOOP
	while (1) {

		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		//controls
		// handle ccw rotation
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) 
			fPlayerA -= (0.8f) * fElapsedTime;
		
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) 
			fPlayerA += (0.8f) * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
			}

		}		
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}
		

		//For each column, calculate the projected ray angle into world space
		for (int x = 0; x < nScreenWidth; x++) {
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			float fDistanceToWall = 0;
			bool bHitWall = false;
			bool bBoundary = false;


			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth) {

				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				//Test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					bHitWall = true;    //Just set distance to maximum depth
					fDistanceToWall = fDepth;
				}
				//else, check cells individually
				else {
					if (map[nTestY * nMapWidth + nTestX] == '#') {
						bHitWall = true;

						vector<pair<float, float>> p; // distance, dot product
						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) {
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}
						//sort pairs from closest to farthest
							sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

							float fBound = 0.01;
							if (acos(p.at(0).second) < fBound) bBoundary = true;
							if (acos(p.at(1).second) < fBound) bBoundary = true;
							

						}
					}
				}
			}
			//calculate distance to ceiling and floor
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)			 nShade = 0x2588;  // realy close
			else if (fDistanceToWall < fDepth / 3.0f)		 nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)		 nShade = 0x2592;
			else if (fDistanceToWall < fDepth)               nShade = 0x2591;
			else                                             nShade = ' '; //too far away
			

			if (bBoundary)    nShade = ' '; //black it out?

			for (int y = 0; y < nScreenHeight; y++) {
				if (y < nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if(y > nCeiling && y <= nFloor)

					screen[y * nScreenWidth + x] = nShade;
				//floor
				else
				{
					//Commented because this breaks the program. Makes the walls all black, changes NShade to a Space every time
					//shade floor based on distance
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		screen[y * nScreenWidth + x] = '#';
					else if (b < 0.5)	screen[y * nScreenWidth + x] = 'x';
					else if (b < 0.75)  screen[y * nScreenWidth + x] = '.';
					else if (b < 0.9)   screen[y * nScreenWidth + x] = '-';
					else                screen[y * nScreenWidth + x] = ' ';

					// not sure if this will do anything
					//after tests keep this commented. vvvvvvvv
					//screen[y * nScreenWidth + x] = '.';
				}
			}
		}
		//Display Stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		//Display Map
		for (int nx = 0; nx < nMapWidth; nx++) {
			for (int ny = 0; ny < nMapWidth; ny++) {
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		}

		//marker for player on the map
		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';

		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		//Writing, telling the handle,buffer, tell it how many bytes, coordinate of where to write, and a variable "we don't really need"
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}