// Based on Javidx9's version for Windows:
// https://youtu.be/8OK8_tHeCIA
// https://github.com/OneLoneCoder/Javidx9/blob/master/SimplyCode/OneLoneCoder_Tetris.cpp

#include <ncurses.h>
#include <time.h>
#include <string>
#include <array>
#include <thread>
#include <chrono>
#include <random>

constexpr int FIELD_WIDTH {12};
constexpr int FIELD_HEIGHT {18};

void drawField();
int getPieceIndexForRotation(const int x, const int y, const int r);
bool pieceDoesFit(const int shapeNum, const int rotation, const int x, const int y);

int main()
{
	// -------------------------
	// Initialize piece shapes
	// -------------------------
	std::array<std::string, 7> tetromino;
	
	// I
	tetromino[0].append("..X.");
	tetromino[0].append("..X.");
	tetromino[0].append("..X.");
	tetromino[0].append("..X.");

	// Z
	tetromino[1].append("..X.");
	tetromino[1].append(".XX.");
	tetromino[1].append(".X..");
	tetromino[1].append("....");

	// S
	tetromino[2].append(".X..");
	tetromino[2].append(".XX.");
	tetromino[2].append("..X.");
	tetromino[2].append("....");

	// O
	tetromino[3].append("....");
	tetromino[3].append(".XX.");
	tetromino[3].append(".XX.");
	tetromino[3].append("....");

	// T
	tetromino[4].append("..X.");
	tetromino[4].append(".XX.");
	tetromino[4].append("..X.");
	tetromino[4].append("....");

	// L
	tetromino[5].append("....");
	tetromino[5].append(".XX.");
	tetromino[5].append("..X.");
	tetromino[5].append("..X.");

	// J
	tetromino[6].append("....");
	tetromino[6].append(".XX.");
	tetromino[6].append(".X..");
	tetromino[6].append(".X..");

	// -------------------------
	// Initialize ncurses screen
	// -------------------------

	initscr();
	// Make user-typed characters immediately available
	cbreak();
	// Don't echo typed characters to the terminal
	noecho();
	// Enable reading of arrow keys
	keypad(stdscr, true);
	// Make getch non-blocking
	nodelay(stdscr, true);
	// Make cursor invisible
	curs_set(0);

	// Initialize random number generator
	std::random_device rd;
	std::default_random_engine randomEngine(rd());
	std::uniform_int_distribution<int> tetrominoDistribution(0, 6);

	// Game state variables
	int currentPiece {tetrominoDistribution(randomEngine)};
	int currentRotation {0};
	int numTicks {0};
	int maxTicks {20};
	int currentX {FIELD_WIDTH / 2};
	int currentY {0};
	int speed {20};

	bool shouldForceDownward {false};
	bool shouldStopRotation {true};
	int numPieces {0};
	int score {0};

	//std::string screenChars {" ABCDEFG=#"};
	
	// Ensure game begins with the screen drawn
	drawField();

	bool gameOver {false};
	while (!gameOver)
	{
		// Manage game timing
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		numTicks++;
		shouldForceDownward = (numTicks == maxTicks);

		// Get input
		const char keyInput = getch();
		switch (keyInput)
		{
		case ERR:
			continue;
		case KEY_LEFT:
			break;
		case KEY_RIGHT:
			break;
		case KEY_DOWN:
			break;
		}

		// Execute game logic
		// First, handle player movement


		// Draw screen
		drawField();
	}

	endwin();
	return 0;
}


void drawField()
{
	clear();
	for (int x = 0; x < FIELD_WIDTH; x++)
	{
		for (int y = 0; y < FIELD_HEIGHT; y++)
		{
			if (x == 0 || x == FIELD_WIDTH - 1 || y == FIELD_HEIGHT - 1)
			{
				mvprintw(y, x, "#");
			}
		}
	}
	refresh();
}


int getPieceIndexForRotation(const int x, const int y, const int r)
{
	int index {0};
	switch (r % 4)
	{
	case 0:
		// 0 degrees:
		//  0  1  2  3
		//  4  5  6  7
		//  8  9 10 11
		// 12 13 14 15
		index = (y * 4) + x;
	case 1:
		// 90 degrees:
		// 12  8  4  0
		// 13  9  5  1
		// 14 10  6  2
		// 15 11  7  3
		index = 12 + y - (x * 4);
	case 2:
		// 180 degrees
		// 15 14 13 12
		// 11 10  9  8
		//  7  6  5  4
		//  3  2  1  0
		index = 15 - (y * 4) - x;
	case 3:
		// 270 degrees
		//  3  7 11 15
		//  2  6 10 14
		//  1  5  9 13
		//  0  4  8 12
		index = 3 - y + (x * 4);
	}
	return index;
}


bool pieceDoesFit(const int shapeNum, const int rotation, const int currentX, const int currentY)
{
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			int pieceIndex = getPieceIndexForRotation(currentX, currentY, rotation);
			int fieldIndex = ;
		}
	}

	return true;
}
