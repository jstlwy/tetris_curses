#include <ncurses.h>
#include <iostream>
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

void drawField(std::array<int, FIELD_LENGTH> & field,
               std::array<char, NUM_SPRITES> & charSprites);

void drawPiece(const std::string & piece,
               const std::array<char, NUM_SPRITES> & charSprites,
               const int currentX, const int currentY, const int currentRotation);

int getPieceIndexForRotation(const std::string & piece, const int x,
                             const int y, const int rotation);

bool pieceDoesFit(const std::array<int, FIELD_LENGTH> field,
                  const std::string & piece, const int rotation,
                  const int pieceX, const int pieceY);

int getSideLength(const int pieceLength);

int main()
{
	// -------------------------
	// Initialize piece shapes
	// -------------------------
	std::array<std::string, NUM_TETROMINOES> tetromino;
	
	// Basing the strings on the Super Rotation System:
	// https://tetris.fandom.com/wiki/SRS
	
	// I
	tetromino[0].append("    ");
	tetromino[0].append("IIII");
	tetromino[0].append("    ");
	tetromino[0].append("    ");

	// Z
	tetromino[1].append("ZZ ");
	tetromino[1].append(" ZZ");
	tetromino[1].append("   ");

	// S
	tetromino[2].append(" SS");
	tetromino[2].append("SS ");
	tetromino[2].append("   ");

	// O
	tetromino[3].append("OO");
	tetromino[3].append("OO");

	// T
	tetromino[4].append(" T ");
	tetromino[4].append("TTT");
	tetromino[4].append("   ");

	// L
	tetromino[5].append("  L");
	tetromino[5].append("LLL");
	tetromino[5].append("   ");

	// J
	tetromino[6].append("J  ");
	tetromino[6].append("JJJ");
	tetromino[6].append("   ");

	// Every character that will be used to draw the screen
	std::array<char, NUM_SPRITES> charSprites {'I', 'Z', 'S', 'O', 'T', 'L', 'J', '=', '#', ' '};

	// ------------------------------
	// Initialize field status map
	// ------------------------------
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
	int currentPieceNum {tetrominoDistribution(randomEngine)};
	int currentRotation {0};
	int currentX {4};
	int currentY {1};

	int numTicks {0};
	int maxTicksPerLine {20};
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
		shouldForceDownward = (numTicks >= maxTicksPerLine);

		std::string currentPiece = tetromino[currentPieceNum];

		// Get input
		const int keyInput = getch();
		int newRotation;
		shouldStopRotation = true;
		bool canGoDown;
		switch (keyInput)
		{
		case 'h':
		case 'H':
		case KEY_LEFT:
			currentX -= pieceDoesFit(field, currentPiece, currentRotation, currentX - 1, currentY) ? 1 : 0;
			break;
		case 'l':
		case 'L':
		case KEY_RIGHT:
			currentX += pieceDoesFit(field, currentPiece, currentRotation, currentX + 1, currentY) ? 1 : 0;
			break;
		case 'j':
		case 'J':
		case KEY_DOWN:
			canGoDown = pieceDoesFit(field, currentPiece, currentRotation, currentX, currentY + 1);
			currentY += canGoDown ? 1 : 0;
			numTicks = canGoDown ? 0 : numTicks;
			break;
		case 'a':
		case 'A':
			// Rotate 90 degrees counterclockwise
			newRotation = (currentRotation == 0) ? 3 : currentRotation - 1;
			currentRotation = pieceDoesFit(field, currentPiece, newRotation, currentX, currentY) ? newRotation : currentRotation;
			shouldStopRotation = false;
			break;
		case 's':
		case 'S':
			// Rotate 90 degrees clockwise
			newRotation = (currentRotation == 3) ? 0 : currentRotation + 1;
			currentRotation = pieceDoesFit(field, currentPiece, newRotation, currentX, currentY) ? newRotation : currentRotation;
			shouldStopRotation = false;
			break;
		}

		if (shouldForceDownward)
		{
			if (numTicks > 0)
			{
				currentY += pieceDoesFit(field, currentPiece, currentRotation, currentX, currentY + 1) ? 1 : 0;
				numTicks = 0;
			}
			shouldForceDownward = false;
		}

		// Draw screen
		drawField(field, charSprites);
		drawPiece(currentPiece, charSprites, currentX, currentY, currentRotation);
	}

	endwin();
	return 0;
}


void drawField(std::array<int, FIELD_LENGTH> & field,
               std::array<char, NUM_SPRITES> & charSprites)
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


void drawPiece(const std::string & piece,
               const std::array<char, NUM_SPRITES> & charSprites,
               const int currentX, const int currentY, const int currentRotation)
{
	const int sideLength = getSideLength(piece.length());

	for (int y = 0; y < sideLength; y++)
	{
		const int drawY = currentY + y;
		for (int x = 0; x < sideLength; x++)
		{
			const int pieceIndex = getPieceIndexForRotation(piece, x, y, currentRotation);
			//std::cout << "x: " << x << ", y: " << y << ", pieceIndex: " << pieceIndex << "\n";
			const char charSprite = piece.at(pieceIndex);
			const int drawX = currentX + x;
			mvaddch(drawY, drawX, charSprite);
		}
	}

	refresh();
}
			

int getPieceIndexForRotation(const std::string & piece,
                             const int x, const int y,
                             const int rotation)
{
	int index {0};

	const int sideLength = getSideLength(piece.length());

	// "O" block rotations are irrelevant
	if (sideLength < 3)
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
		
	switch (rotation)
	{
	case 0:
		index = (y * sideLength) + x;
		break;
	case 1:
		index = (sideLength * (sideLength - 1)) + y - (x * sideLength);
		break;
	case 2:
		index = ((sideLength * sideLength) - 1) - (y * sideLength) - x;
		break;
	case 3:
		index = (sideLength - 1) - y + (x * sideLength);
		break;
	}

	//std::cout << "sideLength: " << sideLength << ", x: " << x << ", y: " << y << ", rotation: " << rotation << ", index: " << index << "\n";
	return index;
}


bool pieceDoesFit(const std::array<int, FIELD_LENGTH> field,
                  const std::string & piece, const int rotation,
                  const int pieceX, const int pieceY)
{
	const int sideLength = getSideLength(piece.length());

	for (int y = 0; y < sideLength; y++)
	{
		const int fieldYOffset = pieceY + (y * FIELD_WIDTH);
		for (int x = 0; x < sideLength; x++)
		{
			const int pieceIndex = getPieceIndexForRotation(piece, x, y, rotation);
			if (piece.at(pieceIndex) != ' ')
			{
				const bool isOutsideField = (pieceX + x < 1 || pieceX + x >= FIELD_WIDTH || pieceY + y >= FIELD_HEIGHT);
				const int fieldIndex = fieldYOffset + (pieceX + x);
				if (isOutsideField || field.at(fieldIndex) != 9)
				{
					return false;
				}
			}
		}
	}

	return true;
}


int getSideLength(const int pieceLength)
{
	int sideLength {0};
	switch (pieceLength)
	{
	case 4:
		sideLength = 2;
		break;
	case 9:
		sideLength = 3;
		break;
	case 16:
		sideLength = 4;
		break;
	}
	return sideLength;
}
