/* ============================================================
 * PART 4: input.h  --  Reading moves from the player
 * ============================================================ */
#ifndef INPUT_H
#define INPUT_H

#include "board.h"

/* Accepts "e2e4", "e2 e4", "e7e8q", "e7 e8 n".
 * Returns 1 on success, 0 if the text isn't a valid move format. */
int parse_move(const char *line, Move *m);

#endif
