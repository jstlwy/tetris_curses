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
int getPieceIndexForRotation(const std::string & piece, const int x, const int y, const int r);
bool pieceDoesFit(const int shapeNum, const int rotation, const int x, const int y);

int main()
{
	// -------------------------
	// Initialize piece shapes
	// -------------------------
	std::array<std::string, 7> tetromino;
	
	// Basing the strings on the Super Rotation System:
	// https://tetris.fandom.com/wiki/SRS
	
	// I
	tetromino[0].append("....");
	tetromino[0].append("XXXX");
	tetromino[0].append("....");
	tetromino[0].append("....");

	// Z
	tetromino[1].append("XX.");
	tetromino[1].append(".XX");
	tetromino[1].append("...");

	// S
	tetromino[2].append(".XX");
	tetromino[2].append("XX.");
	tetromino[2].append("...");

	// O
	tetromino[3].append("XX");
	tetromino[3].append("XX");

	// T
	tetromino[4].append(".X.");
	tetromino[4].append("XXX.");
	tetromino[4].append("...");

	// L
	tetromino[5].append("..X");
	tetromino[5].append("XXX");
	tetromino[5].append("...");

	// J
	tetromino[6].append("X..");
	tetromino[6].append("XXX");
	tetromino[6].append("...");

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


int getPieceIndexForRotation(const std::string & piece, const int x, const int y, const int r)
{
	int index {0};

	const int len = piece.length();

	if (len < 3)
		return index;

	// For 3x3 shapes:
	//
	// 0 degrees:
	// 0 1 2
	// 3 4 5
	// 6 7 8
	//
	// 90 degrees:
	// 6 3 0
	// 7 4 1
	// 8 5 2
	//
	// 180 degrees
	// 8 7 6
	// 5 4 3
	// 2 1 0
	//
	// 270 degrees
	// 2 5 8
	// 1 4 7
	// 0 3 6

	// For 4x4 shapes:
	//
	// 0 degrees:
	//  0  1  2  3
	//  4  5  6  7
	//  8  9 10 11
	// 12 13 14 15
	//
	// 90 degrees:
	// 12  8  4  0
	// 13  9  5  1
	// 14 10  6  2
	// 15 11  7  3
	//
	// 180 degrees
	// 15 14 13 12
	// 11 10  9  8
	//  7  6  5  4
	//  3  2  1  0
	//
	// 270 degrees
	//  3  7 11 15
	//  2  6 10 14
	//  1  5  9 13
	//  0  4  8 12
		
	switch (r % 4)
	{
	case 0:
		index = (y * len) + x;
		break;
	case 1:
		index = (len * (len - 1)) + y - (x * len);
		break;
	case 2:
		index = ((len * len) - 1) - (y * len) - x;
		break;
	case 3:
		index = (len - 1) - y + (x * len);
		break;
	}

	return index;
}


bool pieceDoesFit(const int shapeNum, const int rotation, const int currentX, const int currentY)
{
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			//int pieceIndex = getPieceIndexForRotation(currentX, currentY, rotation);
			//int fieldIndex = ;
		}
	}

	return true;
}
