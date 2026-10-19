/* ============================================================
 * PART 2: moves.h  --  How each piece moves
 * ============================================================ */
#ifndef MOVES_H
#define MOVES_H

#include "board.h"

/* Does the move follow the piece's movement rules?
 * (Does NOT check whether it leaves your own king in check.) */
int is_pseudo_legal(const Game *g, Move m);

/* Is square (r,c) attacked by any piece of colour `by`? */
int is_square_attacked(const Game *g, int r, int c, int by);

#endif
