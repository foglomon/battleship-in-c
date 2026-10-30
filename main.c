/*
     A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 1  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 2  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 ...

Hit - X
Miss - O
Ship - S
Water -

Battleship Modes:

1. Bot vs Player
2. Player vs Player

Input format: <row><col>  e.g. 1A, 5C, 12O
Extra chance on successful hit.
Screen cleared after every move.
Grid displayed with separator lines.
Bot ships placed randomly.
*/



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>

#define SIZE 10
#define SHIP_COUNT 5

#define WATER ' '
#define SHIP  'S'
#define HIT   'X'
#define MISS  'O'

const int shipSizes[SHIP_COUNT] = {5, 4, 3, 3, 2};

const char *shipNames[SHIP_COUNT] = {
    "Carrier",
    "Battleship",
    "Cruiser",
    "Submarine",
    "Destroyer"
};

struct Ship {
    int size;
    int health;
    int row;
    int col;
    char orientation;  // H = horizontal, V = vertical
};

struct Player {
    char name[20];
    char board[SIZE][SIZE];
    struct Ship ships[SHIP_COUNT];
};

/* Function declarations */
void initializePlayer(struct Player *p, const char *name);
void clearScreen(void);
void delay(int ms);
void displayReferenceGrid(void);
void displayBoard(const struct Player *p, int hideShips);
int  parseCoordinate(const char *input, int *row, int *col);
int  placeShipAt(struct Player *p, int index,
                 int row, int col, char orientation);
void placePlayerShips(struct Player *p);
void placeBotShips(struct Player *bot);
int  shipContains(const struct Ship *s, int row, int col);
void performAttack(struct Player *attacker,
                   struct Player *defender, int row, int col);
int  playerAttack(struct Player *player, struct Player *bot);
int  botAttack(struct Player *bot, struct Player *player);
int  allShipsSunk(const struct Player *p);

/* Clear the console screen */
void clearScreen(void)
{
    system("cls");
}

/* Pause for a given number of milliseconds */
void delay(int ms)
{
    Sleep(ms);
}

/* Initialize the board and ship information */
void initializePlayer(struct Player *p, const char *name)
{
    int r, c, i;

    snprintf(p->name, sizeof(p->name), "%s", name);

    for (r = 0; r < SIZE; r++) {
        for (c = 0; c < SIZE; c++) {
            p->board[r][c] = WATER;
        }
    }

    for (i = 0; i < SHIP_COUNT; i++) {
        p->ships[i].size = shipSizes[i];
        p->ships[i].health = shipSizes[i];
        p->ships[i].row = -1;
        p->ships[i].col = -1;
        p->ships[i].orientation = 'H';
    }
}

/* Display an empty reference grid at game start */
void displayReferenceGrid(void)
{
    int r, c;

    printf("\n=== REFERENCE GRID ===\n\n");
    printf("     ");
    for (c = 0; c < SIZE; c++) {
        printf("  %c ", 'A' + c);
    }
    printf("\n");
    printf("    +");
    for (c = 0; c < SIZE; c++) {
        printf("---+");
    }
    printf("\n");

    for (r = 0; r < SIZE; r++) {
        printf(" %2d |", r + 1);
        for (c = 0; c < SIZE; c++) {
            printf("   |");
        }
        printf("\n");
        printf("    +");
        for (c = 0; c < SIZE; c++) {
            printf("---+");
        }
        printf("\n");
    }

    printf("\nRows: 1-%d   Columns: A-%c\n", SIZE, 'A' + SIZE - 1);
    printf("Enter coordinates as: <row><col>  e.g. 1A, 5C, 12O\n");
}

/* Display a board.
   hideShips = 1 hides ships that have not been hit. */
void displayBoard(const struct Player *p, int hideShips)
{
    int r, c;
    char cell;

    printf("\n%s's board\n", p->name);

    /* Column headers */
    printf("     ");
    for (c = 0; c < SIZE; c++) {
        printf("  %c ", 'A' + c);
    }
    printf("\n");

    /* Top border */
    printf("    +");
    for (c = 0; c < SIZE; c++) {
        printf("---+");
    }
    printf("\n");

    for (r = 0; r < SIZE; r++) {
        printf(" %2d |", r + 1);

        for (c = 0; c < SIZE; c++) {
            cell = p->board[r][c];

            if (hideShips && cell == SHIP) {
                cell = WATER;
            }

            printf(" %c |", cell);
        }

        printf("\n");

        /* Row separator */
        printf("    +");
        for (c = 0; c < SIZE; c++) {
            printf("---+");
        }
        printf("\n");
    }

    printf("Legend:   = water, S = ship, X = hit, O = miss\n");
}

/*
 * Parse a coordinate string like "1A" or "12O".
 * Returns 1 on success, filling in zero-based *row and *col.
 * Returns 0 on invalid input.
 */
int parseCoordinate(const char *input, int *row, int *col)
{
    int r = 0;
    int i = 0;
    char letter;

    /* Parse leading digits (row number) */
    while (input[i] >= '0' && input[i] <= '9') {
        r = r * 10 + (input[i] - '0');
        i++;
    }

    if (i == 0 || r < 1 || r > SIZE) {
        return 0;  /* No digits or row out of range */
    }

    /* Parse letter (column) */
    letter = (char)toupper((unsigned char)input[i]);
    if (letter < 'A' || letter > ('A' + SIZE - 1)) {
        return 0;
    }

    /* Check nothing else follows */
    if (input[i + 1] != '\0' && input[i + 1] != '\n' && input[i + 1] != '\r') {
        return 0;
    }

    *row = r - 1;           /* zero-based */
    *col = letter - 'A';    /* zero-based */
    return 1;
}

/* Place a ship at a specified starting position.
   row and col are zero-based inside the program. */
int placeShipAt(struct Player *p, int index,
                int row, int col, char orientation)
{
    int k, r, c;
    int length = p->ships[index].size;

    orientation = (char)toupper((unsigned char)orientation);

    if (orientation != 'H' && orientation != 'V') {
        return 0;
    }

    if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) {
        return 0;
    }

    /* Check that the ship fits and does not overlap another ship. */
    for (k = 0; k < length; k++) {
        r = row + (orientation == 'V' ? k : 0);
        c = col + (orientation == 'H' ? k : 0);

        if (r >= SIZE || c >= SIZE) {
            return 0;
        }

        if (p->board[r][c] != WATER) {
            return 0;
        }
    }

    /* Place the ship after all checks pass. */
    for (k = 0; k < length; k++) {
        r = row + (orientation == 'V' ? k : 0);
        c = col + (orientation == 'H' ? k : 0);
        p->board[r][c] = SHIP;
    }

    p->ships[index].row = row;
    p->ships[index].col = col;
    p->ships[index].orientation = orientation;

    return 1;
}

/* Let the human player position their own ships. */
void placePlayerShips(struct Player *p)
{
    int i, row, col;
    char orientation;
    char coordInput[10];

    clearScreen();
    displayReferenceGrid();
    printf("\nPlace your ships on the %d x %d board.\n", SIZE, SIZE);
    printf("Enter starting position (e.g. 1A) and orientation (H/V).\n");

    for (i = 0; i < SHIP_COUNT; i++) {
        while (1) {
            printf("\n%s (length %d)\n",
                   shipNames[i], p->ships[i].size);

            printf("Starting position (e.g. 3B): ");
            if (scanf("%9s", coordInput) != 1) {
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF) {}
                printf("Invalid input.\n");
                continue;
            }

            if (!parseCoordinate(coordInput, &row, &col)) {
                printf("Invalid coordinate. Use format like 1A, 5C, 12O.\n");
                continue;
            }

            printf("Orientation (H/V): ");
            if (scanf(" %c", &orientation) != 1) {
                printf("Invalid orientation input.\n");
                continue;
            }

            orientation = (char)toupper((unsigned char)orientation);

            if (placeShipAt(p, i, row, col, orientation)) {
                clearScreen();
                displayBoard(p, 0);
                break;
            }

            printf("Invalid placement! Check boundaries, orientation, "
                   "and overlapping ships.\n");
        }
    }
}

/* Randomly place all Bot ships. */
void placeBotShips(struct Player *bot)
{
    int i, row, col;
    char orientation;

    for (i = 0; i < SHIP_COUNT; i++) {
        do {
            row = rand() % SIZE;
            col = rand() % SIZE;
            orientation = (rand() % 2 == 0) ? 'H' : 'V';
        } while (!placeShipAt(bot, i, row, col, orientation));
    }
}

/* Check whether a coordinate belongs to a particular ship. */
int shipContains(const struct Ship *s, int row, int col)
{
    int k;
    int r, c;

    if (s->row < 0 || s->col < 0) {
        return 0;
    }

    for (k = 0; k < s->size; k++) {
        r = s->row + (s->orientation == 'V' ? k : 0);
        c = s->col + (s->orientation == 'H' ? k : 0);

        if (r == row && c == col) {
            return 1;
        }
    }

    return 0;
}

/* Apply an attack to the defender's board. */
void performAttack(struct Player *attacker,
                   struct Player *defender, int row, int col)
{
    int i;

    printf("\n%s attacks %d%c.\n",
           attacker->name, row + 1, 'A' + col);

    if (defender->board[row][col] == SHIP) {
        defender->board[row][col] = HIT;
        printf("HIT!\n");

        for (i = 0; i < SHIP_COUNT; i++) {
            if (shipContains(&defender->ships[i], row, col)) {
                defender->ships[i].health--;

                if (defender->ships[i].health == 0) {
                    printf("%s's %s has been sunk!\n",
                           defender->name, shipNames[i]);
                }

                break;
            }
        }
    } else {
        defender->board[row][col] = MISS;
        printf("MISS!\n");
    }
}

/* Human player's turn.
   Returns 1 if the attack was a HIT, 0 if MISS. */
int playerAttack(struct Player *player, struct Player *bot)
{
    int row, col;
    char coordInput[10];

    while (1) {
        printf("\nYour board:");
        displayBoard(player, 0);

        printf("\nEnemy board:");
        displayBoard(bot, 1);

        printf("\nEnter attack coordinate (e.g. 3B): ");

        if (scanf("%9s", coordInput) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf("Invalid input.\n");
            continue;
        }

        if (!parseCoordinate(coordInput, &row, &col)) {
            printf("Invalid coordinate. Use format like 1A, 5C, 12O.\n");
            continue;
        }

        if (bot->board[row][col] == HIT ||
            bot->board[row][col] == MISS) {
            printf("You already attacked that position. Try again.\n");
            continue;
        }

        performAttack(player, bot, row, col);
        delay(1500);

        return (bot->board[row][col] == HIT) ? 1 : 0;
    }
}

/* Bot attacks a random position that has not been attacked before.
   Returns 1 if the attack was a HIT, 0 if MISS. */
int botAttack(struct Player *bot, struct Player *player)
{
    int row, col;

    do {
        row = rand() % SIZE;
        col = rand() % SIZE;
    } while (player->board[row][col] == HIT ||
             player->board[row][col] == MISS);

    performAttack(bot, player, row, col);
    delay(1500);

    return (player->board[row][col] == HIT) ? 1 : 0;
}

/* Return 1 when all ships have been sunk. */
int allShipsSunk(const struct Player *p)
{
    int i;

    for (i = 0; i < SHIP_COUNT; i++) {
        if (p->ships[i].health > 0) {
            return 0;
        }
    }

    return 1;
}

int main(void)
{
    struct Player player;
    struct Player bot;
    int wasHit;

    srand((unsigned int)time(NULL));

    initializePlayer(&player, "Player");
    initializePlayer(&bot, "Bot");

    clearScreen();
    printf("====================================\n");
    printf("       BATTLESHIP: PLAYER vs BOT\n");
    printf("====================================\n");

    placePlayerShips(&player);
    placeBotShips(&bot);

    printf("\nAll ships are placed. Let the battle begin!\n");
    delay(1500);

    while (1) {
        /* --- Player turn (with extra turns on hit) --- */
        do {
            clearScreen();
            printf("\n========== YOUR TURN ==========\n");
            wasHit = playerAttack(&player, &bot);

            if (allShipsSunk(&bot)) {
                clearScreen();
                printf("\nCongratulations! You defeated the Bot!\n");
                goto game_over;
            }

            if (wasHit) {
                printf("Direct hit! You get another turn!\n");
                delay(1000);
            }
        } while (wasHit);

        /* --- Bot turn (with extra turns on hit) --- */
        do {
            clearScreen();
            printf("\n========== BOT'S TURN ==========\n");
            wasHit = botAttack(&bot, &player);

            if (allShipsSunk(&player)) {
                clearScreen();
                printf("\nThe Bot has sunk all your ships. "
                       "The Bot wins!\n");
                goto game_over;
            }

            if (wasHit) {
                printf("Bot scored a hit! Bot attacks again!\n");
                delay(1000);
            }
        } while (wasHit);
    }

game_over:
    printf("\n========== FINAL BOARDS ==========\n");

    displayBoard(&player, 0);

    /* Reveal the Bot's ships after the game ends. */
    displayBoard(&bot, 0);

    printf("\nGame over. Thanks for playing!\n");

    return 0;
}
