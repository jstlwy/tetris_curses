#include <ncurses.h>
#include <iostream>
#include <time.h>
#include <string>
#include <array>
#include <thread>
#include <chrono>
#include <algorithm>
#include <random>

constexpr int FIELD_WIDTH {12};
constexpr int FIELD_HEIGHT {18};
constexpr int FIELD_LENGTH {FIELD_WIDTH * FIELD_HEIGHT};

class Tetromino {
public:
	int tnum {};
	int x {4};
	int y {1};
	int rot {0};
	int sidelen;

	Tetromino(int tnum)
		: tnum{tnum}, sprite{tetrominoes.at(tnum)}
	{
		sidelen = tetrominoSideLengths.at(tnum);
	}

	void reset(int tnum)
	{
		x = 4;
		y = 1;
		rot = 0;
		sidelen = tetrominoSideLengths.at(tnum);
		sprite = tetrominoes.at(tnum);
	}

	char getSpriteChar(int i) const
	{
		return sprite.at(i);
	}

	char getSpriteLen() const
	{
		return sprite.size();
	}

private:
	std::string sprite;

	// Piece "sprites"
	// Based on the Super Rotation System:
	// https://tetris.fandom.com/wiki/SRS
	inline static std::array<std::string, 7> tetrominoes = {
		"    IIII        ",
		"ZZ  ZZ   ",
		" SSSS    ",
		"OOOO",
		" T TTT   ",
		"  LLLL   ",
		"J  JJJ   "
	};
	static constexpr std::array<int, 7> tetrominoSideLengths {{4, 3, 3, 2, 3, 3, 3}};
};

void drawField(const std::array<char, FIELD_LENGTH>& field);

void drawHUD(const int score, const int numLinesCleared, const int level);

void clearLinesFromField(std::array<char, FIELD_LENGTH>& field,
	int numLinesToClear, int lowestLineToClear);

void drawPiece(Tetromino& t);

//=================
// ROTATION TABLES
//=================
// For 3x3 shapes:
std::array<std::array<std::array<int, 3>, 3>, 4> threeRot {{
	// 0 degrees:
	{{ {{0, 1, 2}},
	   {{3, 4, 5}},
	   {{6, 7, 8}} }},
	// 90 degrees:
	{{ {{6, 3, 0}},
	   {{7, 4, 1}},
	   {{8, 5, 2}} }},
	// 180 degrees:
	{{ {{8, 7, 6}},
	   {{5, 4, 3}},
	   {{2, 1, 0}} }},
	// 270 degrees:
	{{ {{2, 5, 8}},
	   {{1, 4, 7}},
	   {{0, 3, 6}} }}
}};
// For 4x4 shapes:
const std::array<std::array<std::array<int, 4>, 4>, 4> fourRot {{
	// 0 degrees:
	{{ {{ 0,  1,  2,  3}},
	   {{ 4,  5,  6,  7}},
	   {{ 8,  9, 10, 11}},
	   {{12, 13, 14, 15}} }},
	// 90 degrees:
	{{ {{12,  8,  4,  0}},
	   {{13,  9,  5,  1}},
	   {{14, 10,  6,  2}},
	   {{15, 11,  7,  3}} }},
	// 180 degrees:
	{{ {{15, 14, 13, 12}},
	   {{11, 10,  9,  8}},
	   {{ 7,  6,  5,  4}},
	   {{ 3,  2,  1,  0}} }},
	// 270 degrees:
	{{ {{ 3,  7, 11, 15}},
	   {{ 2,  6, 10, 14}},
	   {{ 1,  5,  9, 13}},
	   {{ 0,  4,  8, 12}} }}
}};
int getPieceIndexForRotation(Tetromino& t, int const x, int const y);

bool pieceCanFit(const std::array<char, FIELD_LENGTH>& field, Tetromino& t);

int main()
{
	// -------------------------
	// Initialize field map
	// -------------------------
	std::array<char, FIELD_LENGTH> field;
	for (int y = 0; y < FIELD_HEIGHT; y++)
	{
		const int fieldRow = y * FIELD_WIDTH;
		for (int x = 0; x < FIELD_WIDTH; x++)
		{
			const int i = fieldRow + x;
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

	// Initialize random number generator
	std::random_device rd;
	std::default_random_engine randomEngine(rd());

	// Initialize the array to hold tetromino sequence
	std::array<int, 7> pieceBag = {0, 1, 2, 3, 4, 5, 6};
	std::shuffle(pieceBag.begin(), pieceBag.end(), randomEngine);

	// --------------------
	// Game state variables
	// --------------------
	int currentBagIndex {0};
	int currentPieceNum {pieceBag.at(currentBagIndex)};
	Tetromino t = Tetromino{currentPieceNum};

	bool shouldForceDownward {false};

	unsigned int totalNumLinesCleared {0};
	unsigned int score {0};
	unsigned int level {0};
	unsigned int tenLineCounter {0};

	// Timing
	int numTicks {0};
	int maxTicksPerLine {48};
	const auto usPerFrame {std::chrono::microseconds(16667)};
	
	// Ensure game begins with the screen drawn
	drawField(field);
	drawHUD(score, totalNumLinesCleared, level);

	bool gameOver {false};
	while (!gameOver)
	{
		const auto timeStart = std::chrono::system_clock::now();
		shouldForceDownward = (numTicks >= maxTicksPerLine);

		// Process input
		const int keyInput = getch();
		int newRotation {t.rot};
		switch (keyInput)
		{
		case 'h':
		case 'H':
		case KEY_LEFT:
			t.x--;
			if (!pieceCanFit(field, t))
				t.x++;
			break;
		case 'l':
		case 'L':
		case KEY_RIGHT:
			t.x++;
			if (!pieceCanFit(field, t))
				t.x--;
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

		if (newRotation != t.rot)
		{
			const int currentRotation = t.rot;
			t.rot = newRotation;
			if (!pieceCanFit(field, t))
				t.rot = currentRotation;
		}

		bool shouldFixInPlace {false};
		if (shouldForceDownward)
		{
			t.y++;
			if (!pieceCanFit(field, t))
			{
				t.y--;
				shouldFixInPlace = true;
			}
			numTicks = 0;
			shouldForceDownward = false;
		}

		int numLinesToClear {0};
		int lowestLineToClear {0};

		if (!shouldFixInPlace)
		{
			drawField(field);
			drawPiece(t);
		}
		else if (t.y <= 1)
		{
			gameOver = true;
		}
		else
		{
			// Add piece to field map
			for (int y = 0; y < t.sidelen; y++)
			{
				const int fieldYOffset = (t.y + y) * FIELD_WIDTH;
				for (int x = 0; x < t.sidelen; x++)
				{
					const int pieceIndex = getPieceIndexForRotation(t, x, y);
					const char charSprite = t.getSpriteChar(pieceIndex);
					if (charSprite == ' ')
						continue;
					const int fieldIndex = fieldYOffset + (t.x + x);
					field.at(fieldIndex) = charSprite;
				}
			}

			// Check if any lines should be cleared
			for (int y = 0; y < t.sidelen; y++)
			{
				const int screenRow = t.y + y;
				// Stop if going outside the boundaries
				if (screenRow >= FIELD_HEIGHT - 1)
					break;

				// Begin with the assumption that the line is full of blocks
				bool lineIsFull {true};
				const int fieldRow = screenRow * FIELD_WIDTH;

				// Check whether there are any empty spaces in the line
				for (int x = 1; x < FIELD_WIDTH - 1; x++)
				{
					const int fieldIndex = fieldRow + x;
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
						const int fieldIndex = fieldRow + x;
						field.at(fieldIndex) = '=';
					}

					// Save the location of this line so it can be cleared later
					lowestLineToClear = screenRow;
					numLinesToClear++;
				}
			}

			// Update field
			drawField(field);

			// Update game state
			currentBagIndex++;
			if (currentBagIndex >= pieceBag.size())
			{
				currentBagIndex = 0;
				std::shuffle(pieceBag.begin(), pieceBag.end(), randomEngine);
			}
			currentPieceNum = pieceBag.at(currentBagIndex);
			t.reset(currentPieceNum);
		}

		if (numLinesToClear > 0)
		{
			// Must draw the screen once again
			// to show the lines disappearing.

			// First, wait for a short duration
			// so the player can see the effect.
			std::this_thread::sleep_for(std::chrono::milliseconds(600));

			// Keep track of player progress
			totalNumLinesCleared += numLinesToClear;

			// Scoring system similar to original Nintendo system
			const int scoringLevel = level + 1;
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
		const auto timeEnd = std::chrono::system_clock::now();
		const auto usElapsed = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart);
		if (usElapsed < usPerFrame)
			std::this_thread::sleep_for(usPerFrame - usElapsed);
	}

	endwin();
	std::cout << "Final score: " << score << "\n";
	return 0;
}


void drawField(const std::array<char, FIELD_LENGTH>& field)
{
	for (int y = 0; y < FIELD_HEIGHT; y++)
	{
		const int fieldRow = y * FIELD_WIDTH;
		for (int x = 0; x < FIELD_WIDTH; x++)
		{
			const int fieldIndex = fieldRow + x;
			const char charSprite = field.at(fieldIndex);
			mvaddch(y, x, charSprite);
		}
	}
	refresh();
}


void drawHUD(const int score, const int numLinesCleared, const int level)
{
	mvprintw(1, FIELD_WIDTH + 2, "SCORE:");
	mvprintw(2, FIELD_WIDTH + 2, "%d", score);
	mvprintw(4, FIELD_WIDTH + 2, "LINES:");
	mvprintw(5, FIELD_WIDTH + 2, "%d", numLinesCleared);
	mvprintw(7, FIELD_WIDTH + 2, "LEVEL:");
	mvprintw(8, FIELD_WIDTH + 2, "%d", level);
	refresh();
}


void clearLinesFromField(std::array<char, FIELD_LENGTH>& field,
	int numLinesToClear, int lowestLineToClear)
{
	while (numLinesToClear > 0)
	{
		// Get number of lines to move down
		int numFullContiguousLines {1};
		const int charAboveIndex = ((lowestLineToClear - 1) * FIELD_WIDTH) + 1;
		for (int i = charAboveIndex; field.at(i) == '='; i -= FIELD_WIDTH)
		{
			numFullContiguousLines++;
		}
		
		// This offset designates how far away in the field array
		// the old elements that must be moved down/ahead are
		const int oldYOffset = numFullContiguousLines * FIELD_WIDTH;

		// Move everything in the field array down
		for (int y = lowestLineToClear; y >= 0; y--)
		{
			const int fieldYOffset = y * FIELD_WIDTH;
			for (int x = 1; x < FIELD_WIDTH - 1; x++)
			{
				const int newFieldIndex = fieldYOffset + x;
				if (y <= numFullContiguousLines)
				{
					field.at(newFieldIndex) = ' ';
				}
				else
				{
					const int oldFieldIndex = newFieldIndex - oldYOffset;
					field.at(newFieldIndex) = field.at(oldFieldIndex);
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
			while (field.at(fieldIndex) != '=');
		}
	}
}


void drawPiece(Tetromino& t)
{
	for (int y = 0; y < t.sidelen; y++)
	{
		const int drawY = t.y + y;
		for (int x = 0; x < t.sidelen; x++)
		{
			const int pieceIndex = getPieceIndexForRotation(t, x, y);
			const char charSprite = t.getSpriteChar(pieceIndex);
			if (charSprite == ' ')
				continue;
			const int drawX = t.x + x;
			mvaddch(drawY, drawX, charSprite);
		}
	}

	refresh();
}


int getPieceIndexForRotation(Tetromino& t, int const x, int const y)
{
	int index {0};
	
	/*
	// Old method using arithmetic:
	// The "O" tetromino's rotation is irrelevant
	if (t.sidelen < 3)
		return index;
	switch (t.rot)
	{
	case 0:
		index = (y * t.sidelen) + x;
		break;
	case 1:
		index = (t.getSpriteLen() - t.sidelen) + y - (x * t.sidelen);
		break;
	case 2:
		index = (t.getSpriteLen() - 1) - (y * t.sidelen) - x;
		break;
	case 3:
		index = (t.sidelen - 1) - y + (x * t.sidelen);
		break;
	}
	*/

	// New method using tables:
	switch (t.sidelen)
	{
	case 3:
		index = threeRot.at(t.rot).at(y).at(x);
		break;
	case 4:
		index = fourRot.at(t.rot).at(y).at(x);
		break;
	default:
		break;
	}

	return index;
}


bool pieceCanFit(const std::array<char, FIELD_LENGTH>& field, Tetromino& t)
{
	for (int y = 0; y < t.sidelen; y++)
	{
		const int screenRow = t.y + y;
		const int fieldRow = screenRow * FIELD_WIDTH;
		for (int x = 0; x < t.sidelen; x++)
		{
			const int pieceIndex = getPieceIndexForRotation(t, x, y);
			if (t.getSpriteChar(pieceIndex) == ' ')
				continue;
			const int screenCol = t.x + x;
			if (screenCol < 1 ||
				screenCol >= FIELD_WIDTH ||
				screenRow >= FIELD_HEIGHT ||
				field.at(fieldRow + screenCol) != ' ')
			{
				return false;
			}
		}
	}
	return true;
}