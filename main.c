/*
   01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
A  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
B  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
C  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
D  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
E  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
F  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
G  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
H  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
I  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
J  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
K  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
L  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
M  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
N  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □
O  □  □  □  □  □  □  □  □  □  □  □  □  □  □  □

Hit - X
Miss - O
Ship - S
Water - ~

Battleship Modes:

1. Bot vs Player
2. Player vs Player

Points

print grid
sleep
col abc
extra chance on hitting
window clear
border





*/



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h> 

#define SIZE 15
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
void displayBoard(const struct Player *p, int hideShips);
int placeShipAt(struct Player *p, int index,
                int row, int col, char orientation);
void placePlayerShips(struct Player *p);
void placeBotShips(struct Player *bot);
int shipContains(const struct Ship *s, int row, int col);
void performAttack(struct Player *attacker,
                   struct Player *defender, int row, int col);
void playerAttack(struct Player *player, struct Player *bot);
void botAttack(struct Player *bot, struct Player *player);
int allShipsSunk(const struct Player *p);

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

/* Display a board.
   hideShips = 1 hides ships that have not been hit. */
void displayBoard(const struct Player *p, int hideShips)
{
    int r, c;
    char cell;

    printf("\n%s's board\n", p->name);
    printf("    ");

    for (c = 1; c <= SIZE; c++) {
        printf("%2d ", c);
    }
    printf("\n");

    for (r = 0; r < SIZE; r++) {
        printf("%2d  ", r + 1);

        for (c = 0; c < SIZE; c++) {
            cell = p->board[r][c];

            if (hideShips && cell == SHIP) {
                cell = WATER;
            }

            printf(" %c ", cell);
        }

        printf("\n");
    }

    printf("Legend:  = water, S = ship, X = hit, O = miss\n");
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

    printf("\nPlace your ships on the %d x %d board.\n", SIZE, SIZE);
    printf("Enter starting row, starting column, and orientation.\n");
    printf("Rows and columns are numbered 1 to %d.\n", SIZE);
    printf("Orientation: H = horizontal, V = vertical.\n");

    for (i = 0; i < SHIP_COUNT; i++) {
        while (1) {
            printf("\n%s (length %d)\n",
                   shipNames[i], p->ships[i].size);

            printf("Starting row: ");
            if (scanf("%d", &row) != 1) {
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF) {}
                printf("Please enter a number.\n");
                continue;
            }

            printf("Starting column: ");
            if (scanf("%d", &col) != 1) {
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF) {}
                printf("Please enter a number.\n");
                continue;
            }

            printf("Orientation (H/V): ");
            if (scanf(" %c", &orientation) != 1) {
                printf("Invalid orientation input.\n");
                continue;
            }

            orientation = (char)toupper((unsigned char)orientation);

            if (placeShipAt(p, i, row - 1, col - 1, orientation)) {
                displayBoard(p, 0);
                break;
            }

            printf("Invalid placement! Check the board boundaries, "
                   "orientation, and overlapping ships.\n");
        }
    }
}

/* Programmer-defined Bot ship positions.
   All coordinates below are zero-based.
   Change these values to change the Bot's fleet alignment. */
void placeBotShips(struct Player *bot)
{
    /* Carrier: horizontal, row 2, columns 2-6 */
    placeShipAt(bot, 0, 1, 1, 'H');

    /* Battleship: vertical, column 9, rows 4-7 */
    placeShipAt(bot, 1, 3, 8, 'V');

    /* Cruiser: horizontal, row 7, columns 3-5 */
    placeShipAt(bot, 2, 6, 2, 'H');

    /* Submarine: vertical, column 7, rows 10-12 */
    placeShipAt(bot, 3, 9, 6, 'V');

    /* Destroyer: horizontal, row 13, columns 12-13 */
    placeShipAt(bot, 4, 12, 11, 'H');
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

    printf("\n%s attacks row %d, column %d.\n",
           attacker->name, row + 1, col + 1);

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

/* Human player's turn. */
void playerAttack(struct Player *player, struct Player *bot)
{
    int row, col;
    int ch;

    while (1) {
        printf("\nYour board:");
        displayBoard(player, 0);

        printf("\nEnemy board:");
        displayBoard(bot, 1);

        printf("\nEnter attack row (1-%d): ", SIZE);

        if (scanf("%d", &row) != 1) {
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf("Please enter a number.\n");
            continue;
        }

        printf("Enter attack column (1-%d): ", SIZE);

        if (scanf("%d", &col) != 1) {
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf("Please enter a number.\n");
            continue;
        }

        if (row < 1 || row > SIZE || col < 1 || col > SIZE) {
            printf("Coordinates are outside the board.\n");
            continue;
        }

        row--;
        col--;

        if (bot->board[row][col] == HIT ||
            bot->board[row][col] == MISS) {
            printf("You already attacked that position. Try again.\n");
            continue;
        }

        performAttack(player, bot, row, col);
        break;
    }
}

/* Bot attacks a random position that has not been attacked before. */
void botAttack(struct Player *bot, struct Player *player)
{
    int row, col;

    do {
        row = rand() % SIZE;
        col = rand() % SIZE;
    } while (player->board[row][col] == HIT ||
             player->board[row][col] == MISS);

    performAttack(bot, player, row, col);
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

    srand((unsigned int)time(NULL));

    initializePlayer(&player, "Player");
    initializePlayer(&bot, "Bot");

    printf("====================================\n");
    printf("       BATTLESHIP: PLAYER vs BOT\n");
    printf("====================================\n");

    placePlayerShips(&player);
    placeBotShips(&bot);

    printf("\nAll ships are placed. Let the battle begin!\n");

    while (1) {
        printf("\n========== YOUR TURN ==========\n");
        playerAttack(&player, &bot);

        if (allShipsSunk(&bot)) {
            printf("\nCongratulations! You defeated the Bot!\n");
            break;
        }

        printf("\n========== BOT'S TURN ==========\n");
        botAttack(&bot, &player);

        if (allShipsSunk(&player)) {
            printf("\nThe Bot has sunk all your ships. "
                   "The Bot wins!\n");
            break;
        }
    }

    printf("\n========== FINAL BOARDS ==========\n");

    displayBoard(&player, 0);

    /* Reveal the Bot's ships after the game ends. */
    displayBoard(&bot, 0);

    printf("\nGame over. Thanks for playing!\n");

    return 0;
}



