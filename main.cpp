#include <ncurses.h>
#include <iostream>
#include <time.h>
#include <string>
#include <array>
#include <vector>
#include <thread>
#include <chrono>
#include <random>

constexpr int FIELD_WIDTH {12};
constexpr int FIELD_HEIGHT {18};
constexpr int FIELD_LENGTH {FIELD_WIDTH * FIELD_HEIGHT};
constexpr int NUM_TETROMINOES {7};

void drawField(std::array<char, FIELD_LENGTH> & field);

void drawPiece(const std::string & piece,
               const int currentX, const int currentY,
               const int currentRotation);

int getPieceIndexForRotation(const std::string & piece,
                             const int x, const int y,
                             const int rotation);

bool pieceDoesFit(const std::array<char, FIELD_LENGTH> field,
                  const std::string & piece,
                  const int pieceX, const int pieceY,
				  const int rotation);

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

	// -------------------------
	// Initialize field map
	// -------------------------
	std::array<char, FIELD_LENGTH> field;
	for (int y = 0; y < FIELD_HEIGHT; y++)
	{
		const int currentLineNum = y * FIELD_WIDTH;
		for (int x = 0; x < FIELD_WIDTH; x++)
		{
			const int i = currentLineNum + x;
			field[i] = (x == 0 || x == FIELD_WIDTH - 1 || y == FIELD_HEIGHT - 1) ? '#' : ' ';
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
	std::string currentPiece {tetromino[currentPieceNum]};
	int currentRotation {0};
	int currentX {4};
	int currentY {1};

	int numTicks {0};
	int maxTicksPerLine {20};
	int speed {20};

	bool shouldForceDownward {false};
	bool shouldStopRotation {true};
	std::vector<int> linesToClear;

	int numLinesCleared {0};
	int score {0};
	
	// Ensure game begins with the screen drawn
	drawField(field);

	bool gameOver {false};
	while (!gameOver)
	{
		// Manage game timing
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		numTicks++;
		shouldForceDownward = (numTicks >= maxTicksPerLine);

		// Process input
		const int keyInput = getch();
		int newRotation {currentRotation};
		shouldStopRotation = true;
		bool shouldFixInPlace {false};
		switch (keyInput)
		{
		case 'h':
		case 'H':
		case KEY_LEFT:
			currentX -= pieceDoesFit(field, currentPiece, currentX - 1, currentY, currentRotation) ? 1 : 0;
			break;
		case 'l':
		case 'L':
		case KEY_RIGHT:
			currentX += pieceDoesFit(field, currentPiece, currentX + 1, currentY, currentRotation) ? 1 : 0;
			break;
		case 'j':
		case 'J':
		case KEY_DOWN:
			shouldFixInPlace = !pieceDoesFit(field, currentPiece, currentX, currentY + 1, currentRotation);
			currentY += shouldFixInPlace ? 0 : 1;
			shouldForceDownward = false;
			numTicks = 0;
			break;
		case 'a':
		case 'A':
		case 's':
		case 'S':
			if (keyInput == 'a' || keyInput == 'A')
			{
				// Rotate 90 degrees counterclockwise
				newRotation = (currentRotation == 0) ? 3 : currentRotation - 1;
			}
			else
			{
				// Rotate 90 degrees clockwise
				newRotation = (currentRotation == 3) ? 0 : currentRotation + 1;
			}
			currentRotation = pieceDoesFit(field, currentPiece, currentX, currentY, newRotation) ? newRotation : currentRotation;
			shouldStopRotation = false;
			break;
		}

		if (shouldForceDownward)
		{
			if (pieceDoesFit(field, currentPiece, currentX, currentY + 1, currentRotation))
			{
				currentY++;
			}
			else
			{
				shouldFixInPlace = true;
			}
			numTicks = 0;
			shouldForceDownward = false;
		}

		if (shouldFixInPlace)
		{
			if (currentY <= 1)
			{
				gameOver = true;
			}
			else
			{
				// Add piece to field map
				const int sideLength = getSideLength(currentPiece.length());
				for (int y = 0; y < sideLength; y++)
				{
					const int fieldYOffset = (currentY + y) * FIELD_WIDTH;
					for (int x = 0; x < sideLength; x++)
					{
						const int pieceIndex = getPieceIndexForRotation(currentPiece, x, y, currentRotation);
						const char charSprite = currentPiece.at(pieceIndex);
						if (charSprite != ' ')
						{
							const int fieldIndex = fieldYOffset + (x + currentX);
							field.at(fieldIndex) = charSprite;
						}
					}
				}

				// Check if any lines should be cleared
				for (int y = 0; y < sideLength; y++)
				{
					// Stop if going outside the boundaries
					if (currentY + y >= FIELD_HEIGHT - 1)
					{
						break;
					}

					// Begin with the assumption that the line is full of blocks
					bool lineIsFull {true};
					const int fieldYOffset = (currentY + y) * FIELD_WIDTH;

					// Check whether there are any empty spaces in the line
					for (int x = 1; x < FIELD_WIDTH - 1; x++)
					{
						const int fieldIndex = fieldYOffset + x;
						if (field.at(fieldIndex) == ' ')
						{
							lineIsFull = false;
							break;
						}
					}

					if (lineIsFull)
					{
						// Rewrite all the characters with '='
						for (int x = 1; x < FIELD_WIDTH - 1; x++)
						{
							const int fieldIndex = fieldYOffset + x;
							field.at(fieldIndex) = '=';
						}
						// Save the location of this line so it can be cleared later
						linesToClear.add(currentY + y);
					}
				}

				// Update game state
				currentPieceNum = tetrominoDistribution(randomEngine);
				currentPiece = tetromino[currentPieceNum];
				currentRotation = 0;
				currentX = 4;
				currentY = 1;
			}
		}

		// Draw screen
		drawField(field);

		// Draw any floating pieces
		if (!shouldFixInPlace)
		{
			drawPiece(currentPiece, currentX, currentY, currentRotation);
		}

		// If there are any lines that need to disappear,
		// alter the field map array and redraw the field
		if (!linesToClear.empty())
		{
			// Wait for a short duration to allow the player
			// to see the blocks disappearing
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			// Move everything in the field array down
			for (const int lineNum : linesToClear)
			{
						
			}

			drawField(field);
		}
	}

	endwin();
	return 0;
}


void drawField(std::array<char, FIELD_LENGTH> & field)
{
	clear();
	for (int y = 0; y < FIELD_HEIGHT; y++)
	{
		const int yOffset = y * FIELD_WIDTH;
		for (int x = 0; x < FIELD_WIDTH; x++)
		{
			const int fieldIndex = yOffset + x;
			const char charSprite = field.at(fieldIndex);
			mvaddch(y, x, charSprite);
		}
	}
	refresh();
}


void drawPiece(const std::string & piece,
               const int currentX, const int currentY,
               const int currentRotation)
{
	const int sideLength = getSideLength(piece.length());

	for (int y = 0; y < sideLength; y++)
	{
		const int drawY = currentY + y;
		for (int x = 0; x < sideLength; x++)
		{
			const int pieceIndex = getPieceIndexForRotation(piece, x, y, currentRotation);
			const char charSprite = piece.at(pieceIndex);
			if (charSprite != ' ')
			{
				const int drawX = currentX + x;
				mvaddch(drawY, drawX, charSprite);
			}
		}
	}

	refresh();
}
			

int getPieceIndexForRotation(const std::string & piece,
                             const int x, const int y,
                             const int rotation)
{
	int index {0};

	const int numPoints = piece.length();
	const int sideLength = getSideLength(numPoints);

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
		index = (numPoints - sideLength) + y - (x * sideLength);
		break;
	case 2:
		index = (numPoints - 1) - (y * sideLength) - x;
		break;
	case 3:
		index = (sideLength - 1) - y + (x * sideLength);
		break;
	}

	//std::cout << "sideLength: " << sideLength << ", x: " << x << ", y: " << y << ", rotation: " << rotation << ", index: " << index << "\n";
	return index;
}


bool pieceDoesFit(const std::array<char, FIELD_LENGTH> field,
                  const std::string & piece,
                  const int pieceX, const int pieceY,
                  const int rotation)
{
	const int sideLength = getSideLength(piece.length());

	for (int y = 0; y < sideLength; y++)
	{
		const int fieldYOffset = (pieceY + y) * FIELD_WIDTH;
		for (int x = 0; x < sideLength; x++)
		{
			const int pieceIndex = getPieceIndexForRotation(piece, x, y, rotation);
			if (piece.at(pieceIndex) != ' ')
			{
				const bool isOutsideField = (pieceX + x < 1 || pieceX + x >= FIELD_WIDTH || pieceY + y >= FIELD_HEIGHT);
				const int fieldIndex = fieldYOffset + (pieceX + x);
				if (isOutsideField || field.at(fieldIndex) != ' ')
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

