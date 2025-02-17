// Minesweeper
// by LeonardoTheMutant
//
// Basic minesweeper game in C
//
// Version 1

// KNOWN BUG: Some cells might not show the correct number of mines around them, see G_CountMinesAround()

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

//#define DEBUG

#define CELL_EMPTY 0
#define CELL_MINE 1
#define CELL_CHECKED 2
#define CELL_FLAGGED 4

typedef enum {
    GS_INIT,
    GS_GAME,
    GS_END_GOOD,
    GS_END_BAD
} gamestate_t;

const char welcomeText[] = "Welcome to Minesweeper!\n";
const char quitText[] = "Press CTRL+C to quit";
const char pauseText[] = "Press SPACE+ENTER to continue...";
#ifdef _WIN32
const char CMDclear[] = "cls";
#else
const char CMDclear[] = "clear";
#endif

//symbols
const char mineChar = '*';
const char flagChar = '!';

//function prototypes
//Data Handling
uint8_t D_Char2Hex(char c);
uint8_t D_IsValidInput(char c);

//Menu/User Prompt
uint8_t M_PromptFieldSize(void);
uint8_t M_PromptNumMines(uint8_t);
void M_PromptCell(uint8_t **);

//Game
void G_PlaceMines(uint8_t **, uint8_t);
uint8_t G_CountMinesAround(uint8_t **, uint8_t, uint8_t);
void G_RevealNearbyEmptyCells(uint8_t **, uint8_t, uint8_t);
void G_FinishGameIfComplete(uint8_t **);

//Drawing
void V_DrawField(uint8_t **, uint8_t);

//variables
uint8_t fieldSize;
uint8_t minesNum;
uint8_t minesNum_max;
uint8_t flagsNum;
gamestate_t gamestate;

//main program
int main(int argc, char* argv[])
{
    srand(time(0));
    puts("Welcome to Minesweeper");
    system(CMDclear);

    fieldSize = M_PromptFieldSize();
    minesNum_max = (fieldSize * fieldSize) - ((fieldSize * fieldSize) >> 2);
    flagsNum = minesNum = M_PromptNumMines(minesNum_max);

    while (getchar() != '\n'); //clear the input buffer

    //initialize the minefield
    uint8_t **minefield = calloc(fieldSize, sizeof(uint8_t *)); //rows
    if (!minefield) {
        puts("Failed to allocate memory for rows");
        return -1;
    }

    for (uint8_t i = 0; i < fieldSize; i++) {
        minefield[i] = calloc(fieldSize, sizeof(uint8_t)); //columns
        if (!minefield[i]) {
            puts("Failed to allocate memory for one of the columns");
            return -1;
        }
    }

    gamestate = GS_INIT;
    system(CMDclear);
    V_DrawField(minefield, 0);
    M_PromptCell(minefield);

    do {
        system(CMDclear);
        V_DrawField(minefield, 0);
        M_PromptCell(minefield);
        if (gamestate == GS_GAME) G_FinishGameIfComplete(minefield);
    } while (gamestate == GS_GAME);

    system(CMDclear);
    switch (gamestate) {
        case GS_END_GOOD:
            V_DrawField(minefield, 0);
            puts("Good job! You marked all the mines!");
            break;
        case GS_END_BAD:
            V_DrawField(minefield, 1);
            puts("Stomped on a mine! Game Over");
            break;
    }

    return 0;
}

uint8_t D_Char2Hex(char c) {
    if ((c >= '0') && (c <= '9')) return c - '0';
    else if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
    else if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
}

uint8_t D_IsValidInput(char c) {
    return ((c >= '1') && (c <= '9')) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f'));
}

uint8_t M_PromptFieldSize(void) {
    uint8_t fs = 0;
    while (1) {
        puts("Please select the field size:\n\t1 - 9x9\n\t2 - 10x10\n\t3 - 11x11\n\t4 - 12x12\n\t5 - 13x13\n\t6 - 14x14\n\t7 - 15x15");

        if (scanf("%hhu", &fs) != 1 || fs < 1 || fs > 7)
            puts("Invalid input! Please type a number from 1 to 7.");
        else {
            fs += 8;
            return fs;
        }
    }
}

uint8_t M_PromptNumMines(uint8_t maxMines) {
    uint8_t num = 0;
    while (1) {
        printf("Please select the number of mines (1 - %hu):\n", maxMines);

        if (scanf("%hhu", &num) != 1 || num < 1 || num > maxMines)
            printf("Invalid input! Please enter a number between 1 and %hu.\n", maxMines);
        else
            return num;
    }
}


void M_PromptCell(uint8_t **field) {
    char input[4];
    uint8_t x, y, flag;
    uint8_t prompting = 1;
    while (prompting) {
        printf("Enter the XY coordinates (e.g., A1, B2), \'!\' prefix will toggle flag on the cell: ");
        input[0] = getchar();
        flag = (input[0] == '!') ? 1 : 0;
        input[1] = getchar();
        if (flag)
            input[2] = getchar();


        if ((D_IsValidInput(input[0 + flag]) && ((D_Char2Hex(input[0 + flag]) - 1) < fieldSize)) && (D_IsValidInput(input[1 + flag]) && ((D_Char2Hex(input[1 + flag]) - 1) < fieldSize))) {
            x = D_Char2Hex(input[0 + flag]) - 1;
            y = D_Char2Hex(input[1 + flag]) - 1;

            switch (field[y][x]) {
                case CELL_EMPTY:
                    if (flag) {
                        if (flagsNum) {
                            field[y][x] = CELL_FLAGGED;
                            flagsNum--;
                            prompting = 0;
                        }
                        else
                            puts("You are out of flags, remove the flag from another cell");
                    }
                    else {
                        G_RevealNearbyEmptyCells(field, x, y);
                        prompting = 0;
                    }
                    break;
                case CELL_MINE:
                    if (flag) {
                        if (flagsNum) {
                            field[y][x] |= CELL_FLAGGED;
                            flagsNum--;
                            prompting = 0;
                        }
                        else
                            puts("You are out of flags, remove the flag from another cell");
                    }
                    else {
                        gamestate = GS_END_BAD;
                        prompting = 0;
                    }
                    break;
                case CELL_CHECKED:
                    puts("This cell is already revealed, please choose another cell");
                    break;
                case CELL_FLAGGED:
                case CELL_MINE|CELL_FLAGGED:
                    if (flag) {
                        if (field[y][x] & CELL_FLAGGED) {
                            field[y][x] &= ~CELL_FLAGGED;
                            flagsNum++;
                            prompting = 0;
                        } else {
                            if (flagsNum) {
                                field[y][x] |= CELL_FLAGGED;
                                flagsNum--;
                                prompting = 0;
                            }
                            else
                                puts("This cell is already revealed, please choose another cell");
                        }
                    }
                    else
                        puts("This cell is flagged, please choose another cell. Give the flag toggle command (\'!\' prefix) to remove the flag");
                    break;
            }
        }
        else
            puts("Invalid cell range. Please enter two hexadecimal digits (from 1-9 or A-F range)");

        while (getchar() != '\n');
    }
}

void G_PlaceMines(uint8_t **field, uint8_t minesNum) {
    uint8_t minesPlaced = 0;
    do {
        uint8_t x = rand() % fieldSize;
        uint8_t y = rand() % fieldSize;

        if (field[y][x] == CELL_EMPTY) {
            field[y][x] = CELL_MINE;
            minesPlaced++;
        }
    } while (minesPlaced < minesNum);
}

//Known bug: The amount of mines around a given cell may be incorrect (it shows that there are less mines than really is)
uint8_t G_CountMinesAround(uint8_t **field, uint8_t x, uint8_t y) {
    uint8_t mines = 0;

    for (char dy = -1; dy <= 1; dy++) {
        for (char dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue; // Skip the center cell

            char nx = x + dx;
            char ny = y + dy;

            if ((nx >= 0) && (ny >= 0) && (nx < fieldSize) && (ny < fieldSize))
                if (field[ny][nx] & CELL_MINE)
                    mines++;
        }
    }

    return mines;
}

void G_RevealNearbyEmptyCells(uint8_t **field, uint8_t x, uint8_t y)
{
    if (x >= fieldSize || y >= fieldSize || field[y][x] != CELL_EMPTY)
        return;

    if (gamestate == GS_INIT) {
        G_PlaceMines(field, minesNum);
        gamestate = GS_GAME;
    }

    // BFS queue setup
    uint8_t queueX[fieldSize * fieldSize];
    uint8_t queueY[fieldSize * fieldSize];

    uint16_t front = 0, rear = 0; // Queue pointers

    // Enqueue the starting cell
    queueX[rear] = x;
    queueY[rear] = y;
    rear++;

    // Mark the starting cell as checked immediately
    field[y][x] = CELL_CHECKED;

    while (front < rear) { // While queue is not empty
        uint8_t px = queueX[front];
        uint8_t py = queueY[front];
        front++;

        if (G_CountMinesAround(field, px, py)) continue;

        // Expand to all 8 neighbors
        for (char dy = -1; dy <= 1; dy++) {
            for (char dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue; // Skip itself
                uint8_t nx = px + dx;
                uint8_t ny = py + dy;

                if (nx < fieldSize && ny < fieldSize) {
                    if (field[ny][nx] == CELL_EMPTY) {
                        // Enqueue this cell for processing
                        queueX[rear] = nx;
                        queueY[rear] = ny;
                        rear++;

                        field[ny][nx] = CELL_CHECKED;
                    }
                }
            }
        }
    }
}

void G_FinishGameIfComplete(uint8_t **field) {
    uint8_t flaggedMines = 0;
    for (uint8_t y = 0; y < fieldSize; y++)
        for (uint8_t x = 0; x < fieldSize; x++)
            if (field[y][x] == (CELL_MINE|CELL_FLAGGED))
                flaggedMines++;

    gamestate = (flaggedMines == minesNum) ? GS_END_GOOD : GS_GAME;
}

void V_DrawField(uint8_t **field, uint8_t showall) {
    uint8_t x = 0;

    printf("   ");
    while (x < fieldSize)
        printf("%X ", ++x);

    printf("   Flags: %d\n\n", flagsNum);

    for (uint8_t y = 0; y < fieldSize; y++) {
        printf("%X  ", y + 1);
        for (x = 0; x < fieldSize; x++) {
#ifdef DEBUG
            putchar(field[y][x] + '0');
            putchar(32); //space
#else
            switch (field[y][x]) {
                case CELL_EMPTY:
                    putchar(32); //space
                    break;
                case CELL_MINE:
                    if (showall)
                        putchar(mineChar);
                    else
                        putchar(32); //space
                    break;
                case CELL_CHECKED:
                    putchar(G_CountMinesAround(field, x, y) + '0');
                    break;
                case CELL_FLAGGED:
                case CELL_MINE|CELL_FLAGGED:
                    if (showall && (field[y][x] & CELL_MINE))
                        putchar(mineChar);
                    else
                        putchar(flagChar);
                    break;
            }
            putchar(32); //space
#endif
        }
        putchar('\n');
    }
}