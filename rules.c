/* ============================================================
 * PART 3: rules.c  --  Check, checkmate, stalemate, special moves
 * ============================================================ */
#include <stdlib.h>
#include <ctype.h>
#include "rules.h"
#include "moves.h"

/* A king or rook leaving / being captured on its home square
 * removes the matching castling right. */
static void update_castling(Game *g, int r, int c)
{
    if (r == 7 && c == 4) g->castle_wk = g->castle_wq = 0;
    if (r == 0 && c == 4) g->castle_bk = g->castle_bq = 0;
    if (r == 7 && c == 7) g->castle_wk = 0;
    if (r == 7 && c == 0) g->castle_wq = 0;
    if (r == 0 && c == 7) g->castle_bk = 0;
    if (r == 0 && c == 0) g->castle_bq = 0;
}

void apply_move(Game *g, Move m)
{
    char p    = g->sq[m.fr][m.fc];
    char up   = (char)toupper((unsigned char)p);
    int color = piece_color(p);

    /* En passant: pawn moves diagonally onto an empty square */
    if (up == 'P' && m.fc != m.tc && g->sq[m.tr][m.tc] == EMPTY)
        g->sq[m.fr][m.tc] = EMPTY;

    /* Castling: also move the rook */
    if (up == 'K' && abs(m.tc - m.fc) == 2) {
        if (m.tc == 6) {
            g->sq[m.fr][5] = g->sq[m.fr][7];
            g->sq[m.fr][7] = EMPTY;
        } else {
            g->sq[m.fr][3] = g->sq[m.fr][0];
            g->sq[m.fr][0] = EMPTY;
        }
    }

    /* Move the piece */
    g->sq[m.tr][m.tc] = p;
    g->sq[m.fr][m.fc] = EMPTY;

    /* Promotion (defaults to queen) */
    if (up == 'P' && (m.tr == 0 || m.tr == 7)) {
        char pr = m.promo ? (char)toupper((unsigned char)m.promo) : 'Q';
        g->sq[m.tr][m.tc] = (color == WHITE) ? pr : (char)tolower((unsigned char)pr);
    }

    update_castling(g, m.fr, m.fc);
    update_castling(g, m.tr, m.tc);

    /* New en passant target after a double pawn push */
    g->ep_r = g->ep_c = -1;
    if (up == 'P' && abs(m.tr - m.fr) == 2) {
        g->ep_r = (m.fr + m.tr) / 2;
        g->ep_c = m.fc;
    }

    g->turn = !g->turn;
}

int is_in_check(const Game *g, int color)
{
    char king = (color == WHITE) ? 'K' : 'k';
    for (int r = 0; r < SIZE; r++)
        for (int c = 0; c < SIZE; c++)
            if (g->sq[r][c] == king)
                return is_square_attacked(g, r, c, !color);
    return 0;
}

/* Legal = follows piece rules AND doesn't leave own king in check.
 * We test this by playing the move on a copy of the game. */
int is_legal(const Game *g, Move m)
{
    if (!is_pseudo_legal(g, m)) return 0;
    Game copy = *g;
    apply_move(&copy, m);
    return !is_in_check(&copy, g->turn);
}

static int has_legal_move(const Game *g)
{
    Move m;
    m.promo = 'q';
    for (m.fr = 0; m.fr < SIZE; m.fr++)
        for (m.fc = 0; m.fc < SIZE; m.fc++) {
            if (piece_color(g->sq[m.fr][m.fc]) != g->turn) continue;
            for (m.tr = 0; m.tr < SIZE; m.tr++)
                for (m.tc = 0; m.tc < SIZE; m.tc++)
                    if (is_legal(g, m)) return 1;
        }
    return 0;
}

int game_status(const Game *g)
{
    if (has_legal_move(g)) return ONGOING;
    return is_in_check(g, g->turn) ? CHECKMATE : STALEMATE;
}
