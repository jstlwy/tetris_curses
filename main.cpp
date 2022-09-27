#include <ncurses.h>
#include <time.h>
#include <string>
#include <array>
#include <thread>
#include <chrono>
#include <random>

constexpr int FIELD_WIDTH {12};
constexpr int FIELD_HEIGHT {18};
constexpr int FIELD_LENGTH {FIELD_WIDTH * FIELD_HEIGHT};
constexpr int NUM_TETROMINOES {7};
constexpr int NUM_SPRITES {10};

void drawField(std::array<int, FIELD_LENGTH> & field, std::array<char, NUM_SPRITES> & charSprites);

int getPieceIndexForRotation(const std::string & piece, const int x, const int y, const int r);

bool pieceDoesFit(const std::array<int, FIELD_LENGTH> field, const std::string & piece,
                  const int rotation, const int pieceX, const int pieceY);

int main()
{
	// -------------------------
	// Initialize piece shapes
	// -------------------------
	std::array<std::string, NUM_TETROMINOES> tetromino;
	
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

	// Every character that will be used to draw the screen
	std::array<char, NUM_SPRITES> charSprites {'I', 'Z', 'S', 'O', 'T', 'L', 'J', '=', '#', ' '};

	// -------------------------
	// Initialize field tracker
	// -------------------------
	std::array<int, FIELD_LENGTH> field;
	for (int y = 0; y < FIELD_HEIGHT; y++)
	{
		const int currentLineNum = y * FIELD_WIDTH;
		for (int x = 0; x < FIELD_WIDTH; x++)
		{
			const int i = currentLineNum + x;
			field[i] = (x == 0 || x == FIELD_WIDTH - 1 || y == FIELD_HEIGHT - 1) ? 8 : 9;
		}
	}

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
	int maxTicksPerLine {20};
	int currentX {4};
	int currentY {1};
	int speed {20};

	bool shouldForceDownward {false};
	bool shouldStopRotation {true};
	int numPieces {0};
	int score {0};

	
	// Ensure game begins with the screen drawn
	drawField(field, charSprites);

	bool gameOver {false};
	while (!gameOver)
	{
		// Manage game timing
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		numTicks++;
		shouldForceDownward = (numTicks == maxTicksPerLine);

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
		drawField(field, charSprites);
	}

	endwin();
	return 0;
}


void drawField(std::array<int, FIELD_LENGTH> & field, std::array<char, NUM_SPRITES> & charSprites)
{
	clear();
	for (int y = 0; y < FIELD_HEIGHT; y++)
	{
		const int currentLineNum = y * FIELD_WIDTH;
		for (int x = 0; x < FIELD_WIDTH; x++)
		{
			const int index = currentLineNum + x;
			const int charSpriteNum = field.at(index);
			const char charSprite = charSprites.at(charSpriteNum);
			mvaddch(y, x, charSprite);
		}
	}
	refresh();
}


int getPieceIndexForRotation(const std::string & piece, const int x, const int y, const int r)
{
	int index {0};

	const int len = piece.length();

	// "O" block rotations are irrelevant
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
	// 180 degrees:
	// 8 7 6
	// 5 4 3
	// 2 1 0
	//
	// 270 degrees:
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
	// 180 degrees:
	// 15 14 13 12
	// 11 10  9  8
	//  7  6  5  4
	//  3  2  1  0
	//
	// 270 degrees:
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


bool pieceDoesFit(const std::array<int, FIELD_LENGTH> field, const std::string & piece,
                  const int rotation, const int pieceX, const int pieceY)
{
	for (int y = 0; y < piece.length(); y++)
	{
		const int currentLineNum = (pieceY + y) * FIELD_WIDTH;
		for (int x = 0; x < piece.length(); x++)
		{
			// Make sure not to go out of bounds
			if (pieceX + x < 1 || pieceX + x >= FIELD_WIDTH || pieceY + y >= FIELD_HEIGHT)
			{
				continue;
			}

			const int pieceIndex = getPieceIndexForRotation(piece, pieceX, pieceY, rotation);
			const int fieldIndex = currentLineNum + (pieceX + x);

			if (piece.at(pieceIndex) == 'X' && field.at(fieldIndex) != 0)
			{
				return false;
			}
		}
	}

	return true;
}
