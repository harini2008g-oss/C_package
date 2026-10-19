/* ============================================================
 * PART 3: rules.h  --  Game rules: check, mate, making moves
 * ============================================================ */
#ifndef RULES_H
#define RULES_H

#include "board.h"

#define ONGOING   0
#define CHECKMATE 1
#define STALEMATE 2

void apply_move(Game *g, Move m);          /* play a move (assumes legal) */
int  is_in_check(const Game *g, int color);
int  is_legal(const Game *g, Move m);      /* full legality check         */
int  game_status(const Game *g);           /* ONGOING / CHECKMATE / STALEMATE */

#endif
