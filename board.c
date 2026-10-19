/* ============================================================
 * PART 1: board.c  --  Board setup and display
 * ============================================================ */
#include <stdio.h>
#include <ctype.h>
#include "board.h"

void board_init(Game *g)
{
    const char *start[SIZE] = {
        "rnbqkbnr",
        "pppppppp",
        "........",
        "........",
        "........",
        "........",
        "PPPPPPPP",
        "RNBQKBNR"
    };

    for (int r = 0; r < SIZE; r++)
        for (int c = 0; c < SIZE; c++)
            g->sq[r][c] = start[r][c];

    g->turn = WHITE;
    g->castle_wk = g->castle_wq = 1;
    g->castle_bk = g->castle_bq = 1;
    g->ep_r = g->ep_c = -1;
}

void board_print(const Game *g)
{
    printf("\n     a b c d e f g h\n");
    printf("   +-----------------+\n");
    for (int r = 0; r < SIZE; r++) {
        printf(" %d | ", 8 - r);
        for (int c = 0; c < SIZE; c++)
            printf("%c ", g->sq[r][c]);
        printf("| %d\n", 8 - r);
    }
    printf("   +-----------------+\n");
    printf("     a b c d e f g h\n\n");
}

int piece_color(char p)
{
    if (p == EMPTY) return -1;
    return isupper((unsigned char)p) ? WHITE : BLACK;
}

int on_board(int r, int c)
{
    return r >= 0 && r < SIZE && c >= 0 && c < SIZE;
}
