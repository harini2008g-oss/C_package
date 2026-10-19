/* ============================================================
 * PART 1: board.h  --  Data structures and board interface
 * ============================================================ */
#ifndef BOARD_H
#define BOARD_H

#define SIZE  8
#define EMPTY '.'
#define WHITE 0
#define BLACK 1

/* Pieces: uppercase = White, lowercase = Black
 *   K/k King   Q/q Queen   R/r Rook
 *   B/b Bishop N/n Knight  P/p Pawn
 * Row 0 is rank 8 (Black's back rank), row 7 is rank 1 (White's). */

typedef struct {
    char sq[SIZE][SIZE];   /* the board                                */
    int  turn;             /* WHITE or BLACK to move                   */
    int  castle_wk;        /* White may still castle king side  (1/0)  */
    int  castle_wq;        /* White may still castle queen side        */
    int  castle_bk;        /* Black may still castle king side         */
    int  castle_bq;        /* Black may still castle queen side        */
    int  ep_r, ep_c;       /* en passant target square, -1 if none     */
} Game;

typedef struct {
    int  fr, fc;           /* from row / col                           */
    int  tr, tc;           /* to row / col                             */
    char promo;            /* promotion piece (q,r,b,n) or 0           */
} Move;

void board_init(Game *g);
void board_print(const Game *g);
int  piece_color(char p);          /* WHITE, BLACK, or -1 if empty */
int  on_board(int r, int c);

#endif
