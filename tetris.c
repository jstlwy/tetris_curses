#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

int const FIELD_WIDTH = 12;
int const FIELD_HEIGHT = 18;
int const FIELD_LENGTH = FIELD_WIDTH * FIELD_HEIGHT;

// ----------------
// Piece "sprites"
// ----------------
// Based on the Super Rotation System:
// https://tetris.fandom.com/wiki/SRS
char const*const tetrominoes[7] = {
	"    IIII        ",
	"ZZ  ZZ   ",
	" SSSS    ",
	"OOOO",
	" T TTT   ",
	"  LLLL   ",
	"J  JJJ   "
};
int tetrominoLengths[7] = {16, 9, 9, 4, 9, 9, 9};
int tetrominoSideLengths[7] = {4, 3, 3, 2, 3, 3, 3};

void drawField(char field[const FIELD_LENGTH]);

void drawHUD(int const score, int const numLinesCleared, int const level);

void clearLinesFromField(char field[const FIELD_LENGTH],
	int numLinesToClear, int lowestLineToClear);

void drawPiece(char const*const piece, int const length, int const sideLength,
	int const currentX, int const currentY, int const currentRotation);

int getPieceIndexForRotation(int const numPoints, int const sideLength,
	int const x, int const y, int const rotation);

bool pieceCanFit(char field[const FIELD_LENGTH],
	char const*const piece, int const length, int const sideLength,
	int const pieceX, int const pieceY, int const rotation);

void shuffleArray(int bag[const 7]);

long getTimeDiff(struct timespec* start, struct timespec* stop);

int main(void)
{
	// -------------------------
	// Initialize field map
	// -------------------------
	char field[FIELD_LENGTH];
	for (int y = 0; y < FIELD_HEIGHT; y++)
	{
		int const fieldRow = y * FIELD_WIDTH;
		for (int x = 0; x < FIELD_WIDTH; x++)
		{
			int const i = fieldRow + x;
			if (x == 0 || x == FIELD_WIDTH - 1 || y == FIELD_HEIGHT - 1)
				field[i] = '#';
			else
				field[i] = ' ';
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

	// Initialize the array to hold tetromino sequence
	int pieceBag[7] = {0, 1, 2, 3, 4, 5, 6};
	shuffleArray(pieceBag);

	// --------------------
	// Game state variables
	// --------------------
	
	int currentBagIndex = 0;
	int currentPieceNum = pieceBag[currentBagIndex];
	char const* currentPiece = tetrominoes[currentPieceNum];
	int currentLength = tetrominoLengths[currentPieceNum];
	int currentSideLength = tetrominoSideLengths[currentPieceNum];
	int currentRotation = 0;
	int currentX = 4;
	int currentY = 1;

	bool shouldForceDownward = false;

	unsigned int totalNumLinesCleared = 0;
	unsigned int score = 0;
	unsigned int level = 0;
	unsigned int tenLineCounter =0;

	// Timing
	int numTicks = 0;
	int maxTicksPerLine = 48;
	struct timespec start;
	struct timespec stop;
	long const nsPerFrame = 16666667;
	
	// Ensure game begins with the screen drawn
	drawField(field);
	drawHUD(score, totalNumLinesCleared, level);

	bool gameOver = false;
	while (!gameOver)
	{
		clock_gettime(CLOCK_MONOTONIC, &start);
		shouldForceDownward = (numTicks >= maxTicksPerLine);

		// Process input
		int const keyInput = getch();
		int newRotation = currentRotation;
		bool shouldFixInPlace = false;
		switch (keyInput)
		{
		case 'h':
		case 'H':
		case KEY_LEFT:
			if (pieceCanFit(
				field,
				currentPiece,
				currentLength,
				currentSideLength,
				currentX-1,
				currentY,
				currentRotation))
			{
				currentX--;
			}
			break;
		case 'l':
		case 'L':
		case KEY_RIGHT:
			if (pieceCanFit(
				field,
				currentPiece,
				currentLength,
				currentSideLength,
				currentX+1,
				currentY,
				currentRotation))
			{
				currentX++;
			}
			break;
		case 'j':
		case 'J':
		case KEY_DOWN:
			shouldForceDownward = true;
			break;
		case 'a':
		case 'A':
			// Rotate 90 degrees counterclockwise
			newRotation = (newRotation == 0) ? 3 : newRotation - 1;
			break;
		case 's':
		case 'S':
			// Rotate 90 degrees clockwise
			newRotation = (newRotation == 3) ? 0 : newRotation + 1;
			break;
		default:
			break;
		}

		if (newRotation != currentRotation &&
		    pieceCanFit(
			    field,
			    currentPiece,
			    currentLength,
			    currentSideLength,
			    currentX,
			    currentY,
			    newRotation))
		{
			currentRotation = newRotation;
		}

		if (shouldForceDownward)
		{
			if (pieceCanFit(
				field,
				currentPiece,
				currentLength,
				currentSideLength,
				currentX,
				currentY+1,
				currentRotation))
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

		int numLinesToClear = 0;
		int lowestLineToClear = 0;

		if (!shouldFixInPlace)
		{
			drawField(field);
			drawPiece(
				currentPiece,
				currentLength,
				currentSideLength,
				currentX,
				currentY,
				currentRotation
			);
		}
		else if (currentY <= 1)
		{
			gameOver = true;
		}
		else
		{
			// Add piece to field map
			for (int y = 0; y < currentSideLength; y++)
			{
				int const fieldYOffset = (currentY + y) * FIELD_WIDTH;
				for (int x = 0; x < currentSideLength; x++)
				{
					int const pieceIndex = getPieceIndexForRotation(
						currentLength,
						currentSideLength,
						x,
						y,
						currentRotation
					);
					char const charSprite = currentPiece[pieceIndex];
					if (charSprite == ' ')
						continue;
					int const fieldIndex = fieldYOffset + (x + currentX);
					field[fieldIndex] = charSprite;
				}
			}

			// Check if any lines should be cleared
			for (int y = 0; y < currentSideLength; y++)
			{
				int const screenRow = currentY + y;
				// Stop if going outside the boundaries
				if (screenRow >= FIELD_HEIGHT - 1)
					break;

				// Begin with the assumption that the line is full of blocks
				bool lineIsFull = true;
				int const fieldRow = screenRow * FIELD_WIDTH;

				// Check whether there are any empty spaces in the line
				for (int x = 1; x < FIELD_WIDTH - 1; x++)
				{
					int const fieldIndex = fieldRow + x;
					if (field[fieldIndex] == ' ')
					{
						lineIsFull = false;
						break;
					}
				}

				if (!lineIsFull)
					continue;

				// Rewrite all the characters with '='
				for (int x = 1; x < FIELD_WIDTH - 1; x++)
				{
					int const fieldIndex = fieldRow + x;
					field[fieldIndex] = '=';
				}

				// Save the location of this line so it can be cleared later
				lowestLineToClear = screenRow;
				numLinesToClear++;
			}

			// Update field
			drawField(field);

			// Update game state
			currentBagIndex++;
			if (currentBagIndex >= 7)
			{
				currentBagIndex = 0;
				shuffleArray(pieceBag);
			}
			currentPieceNum = pieceBag[currentBagIndex];
			currentPiece = tetrominoes[currentPieceNum];
			currentLength = tetrominoLengths[currentPieceNum];
			currentSideLength = tetrominoSideLengths[currentPieceNum];
			currentRotation = 0;
			currentX = 4;
			currentY = 1;
		}

		if (numLinesToClear > 0)
		{
			// Must draw the screen once again
			// to show the lines disappearing.

			// First, wait for a short duration
			// so the player can see the effect.
			struct timespec sleepTime = {0, 600000000};
			nanosleep(&sleepTime, &sleepTime);

			// Keep track of player progress
			totalNumLinesCleared += numLinesToClear;

			// Scoring system similar to original Nintendo system
			int const scoringLevel = level + 1;
			switch (numLinesToClear)
			{
			case 1:
				score += 40 * scoringLevel;
				break;
			case 2:
				score += 100 * scoringLevel;
				break;
			case 3:
				score += 300 * scoringLevel;
				break;
			case 4:
				score += 1200 * scoringLevel;
				break;
			}

			// Check if level should advance
			tenLineCounter += numLinesToClear;
			if (tenLineCounter >= 10)
			{
				level++;
				tenLineCounter -= 10;
				// Adjust timing
				if (level < 8 && maxTicksPerLine > 5)
					maxTicksPerLine -= 5;
				else if (maxTicksPerLine > 1)
					maxTicksPerLine--;
			}

			clearLinesFromField(field, numLinesToClear, lowestLineToClear);
			drawField(field);
			drawHUD(score, totalNumLinesCleared, level);
		}
 
		numTicks++;
		// Wait if necessary to maintain roughly 60 loops per second
		clock_gettime(CLOCK_MONOTONIC, &stop);
		long const nsElapsed = getTimeDiff(&start, &stop);
		if (nsElapsed < nsPerFrame)
		{
			struct timespec sleepTime = {0, nsPerFrame - nsElapsed};
			nanosleep(&sleepTime, &sleepTime);
		}
	}

	endwin();
	printf("Final score: %d\n", score);
	return EXIT_SUCCESS;
}


void drawField(char field[FIELD_LENGTH])
{
	for (int y = 0; y < FIELD_HEIGHT; y++)
	{
		int const fieldRow = y * FIELD_WIDTH;
		for (int x = 0; x < FIELD_WIDTH; x++)
		{
			int const fieldIndex = fieldRow + x;
			char const charSprite = field[fieldIndex];
			mvaddch(y, x, charSprite);
		}
	}
	refresh();
}


void drawHUD(int const score, int const numLinesCleared, int const level)
{
	mvprintw(1, FIELD_WIDTH + 2, "SCORE:");
	mvprintw(2, FIELD_WIDTH + 2, "%d", score);
	mvprintw(4, FIELD_WIDTH + 2, "LINES:");
	mvprintw(5, FIELD_WIDTH + 2, "%d", numLinesCleared);
	mvprintw(7, FIELD_WIDTH + 2, "LEVEL:");
	mvprintw(8, FIELD_WIDTH + 2, "%d", level);
	refresh();
}


void clearLinesFromField(char field[FIELD_LENGTH],
	int numLinesToClear, int lowestLineToClear)
{
	while (numLinesToClear > 0)
	{
		// Get number of lines to move down
		int numFullContiguousLines = 1;
		int const charAboveIndex = ((lowestLineToClear - 1) * FIELD_WIDTH) + 1;
		for (int i = charAboveIndex; field[i] == '='; i -= FIELD_WIDTH)
		{
			numFullContiguousLines++;
		}
		
		// This offset designates how far away in the field array
		// the old elements that must be moved down/ahead are
		int const oldYOffset = numFullContiguousLines * FIELD_WIDTH;

		// Move everything in the field array down
		for (int y = lowestLineToClear; y >= 0; y--)
		{
			int const fieldYOffset = y * FIELD_WIDTH;
			for (int x = 1; x < FIELD_WIDTH - 1; x++)
			{
				int const newFieldIndex = fieldYOffset + x;
				if (y <= numFullContiguousLines)
				{
					field[newFieldIndex] = ' ';
				}
				else
				{
					int const oldFieldIndex = newFieldIndex - oldYOffset;
					field[newFieldIndex] = field[oldFieldIndex];
				}
			}
		}

		numLinesToClear -= numFullContiguousLines;
		if (numLinesToClear > 0)
		{
			// Find the next line that needs to be cleared
			int fieldIndex;
			do
			{
				lowestLineToClear--;
				fieldIndex = (lowestLineToClear * FIELD_WIDTH) + 1;
			}
			while (field[fieldIndex] != '=');
		}
	}
}


void drawPiece(char const*const piece, int const length, int const sideLength,
	int const currentX, int const currentY, int const currentRotation)
{
	for (int y = 0; y < sideLength; y++)
	{
		int const drawY = currentY + y;
		for (int x = 0; x < sideLength; x++)
		{
			int const pieceIndex = getPieceIndexForRotation(
				length,
				sideLength,
				x,
				y,
				currentRotation
			);
			char const charSprite = piece[pieceIndex];
			if (charSprite == ' ')
				continue;
			int const drawX = currentX + x;
			mvaddch(drawY, drawX, charSprite);
		}
	}
	refresh();
}


int getPieceIndexForRotation(int const numPoints, int const sideLength,
	int const x, int const y, int const rotation)
{
	int index = 0;
	// The "O" tetromino's rotation is irrelevant
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

	return index;
}


bool pieceCanFit(char field[const FIELD_LENGTH],
	char const*const piece, int const length, int const sideLength,
	int const pieceX, int const pieceY, int const rotation)
{
	for (int y = 0; y < sideLength; y++)
	{
		int const screenRow = pieceY + y;
		int const fieldRow = screenRow * FIELD_WIDTH;
		for (int x = 0; x < sideLength; x++)
		{
			int const pieceIndex = getPieceIndexForRotation(
				length,
				sideLength,
				x,
				y,
				rotation
			);
			if (piece[pieceIndex] == ' ')
				continue;
			int const screenCol = pieceX + x;
			if (screenCol < 1 ||
			    screenCol >= FIELD_WIDTH ||
			    screenRow >= FIELD_HEIGHT ||
			    field[fieldRow + screenCol] != ' ')
			{
				return false;
			}
		}
	}
	return true;
}


void shuffleArray(int bag[7])
{
	// Fisher-Yates shuffle
	for (int i = 6; i >= 1; i--)
	{
		int const j = arc4random_uniform(i + 1);
		int const temp = bag[i];
		bag[i] = bag[j];
		bag[j] = temp;
	}
}


long const ns_per_s = 1000000000;
long getTimeDiff(struct timespec* start, struct timespec* stop)
{
	long const start_nsec = start->tv_nsec + (start->tv_sec * ns_per_s);
	long const stop_nsec = stop->tv_nsec + (stop->tv_sec * ns_per_s);
	return stop_nsec - start_nsec;
}
