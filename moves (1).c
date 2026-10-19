/* ============================================================
 * PART 2: moves.c  --  Piece movement rules
 * ============================================================ */
#include <stdlib.h>
#include <ctype.h>
#include "moves.h"

/* All squares strictly between (fr,fc) and (tr,tc) must be empty.
 * Works for straight lines and diagonals. */
static int path_clear(const Game *g, int fr, int fc, int tr, int tc)
{
    int dr = (tr > fr) - (tr < fr);
    int dc = (tc > fc) - (tc < fc);
    int r = fr + dr, c = fc + dc;

    while (r != tr || c != tc) {
        if (g->sq[r][c] != EMPTY) return 0;
        r += dr;
        c += dc;
    }
    return 1;
}

/* Can the piece on (fr,fc) attack (tr,tc)?
 * For pawns this is the diagonal capture only. */
static int attacks(const Game *g, int fr, int fc, int tr, int tc)
{
    char p  = (char)toupper((unsigned char)g->sq[fr][fc]);
    int dr  = tr - fr, dc = tc - fc;
    int adr = abs(dr), adc = abs(dc);

    switch (p) {
    case 'P': {
        int dir = piece_color(g->sq[fr][fc]) == WHITE ? -1 : 1;
        return dr == dir && adc == 1;
    }
    case 'N':
        return (adr == 1 && adc == 2) || (adr == 2 && adc == 1);
    case 'B':
        return adr == adc && adr > 0 && path_clear(g, fr, fc, tr, tc);
    case 'R':
        return ((dr == 0) != (dc == 0)) && path_clear(g, fr, fc, tr, tc);
    case 'Q':
        return ((adr == adc && adr > 0) || ((dr == 0) != (dc == 0)))
               && path_clear(g, fr, fc, tr, tc);
    case 'K':
        return adr <= 1 && adc <= 1 && (adr + adc) > 0;
    }
    return 0;
}

int is_square_attacked(const Game *g, int r, int c, int by)
{
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (piece_color(g->sq[i][j]) == by && attacks(g, i, j, r, c))
                return 1;
    return 0;
}

/* Pawn pushes, double push, captures and en passant. */
static int pawn_move(const Game *g, Move m, int color)
{
    int dir    = (color == WHITE) ? -1 : 1;
    int start  = (color == WHITE) ? 6 : 1;
    int dr     = m.tr - m.fr;
    int dc     = m.tc - m.fc;
    char target = g->sq[m.tr][m.tc];

    if (dc == 0) {                                  /* forward push */
        if (target != EMPTY) return 0;
        if (dr == dir) return 1;
        if (dr == 2 * dir && m.fr == start &&
            g->sq[m.fr + dir][m.fc] == EMPTY) return 1;
        return 0;
    }
    if (abs(dc) == 1 && dr == dir) {                /* diagonal */
        if (target != EMPTY && piece_color(target) != color) return 1;
        if (m.tr == g->ep_r && m.tc == g->ep_c) return 1;   /* en passant */
    }
    return 0;
}

/* Castling: king moves two squares toward a rook. */
static int can_castle(const Game *g, Move m, int color)
{
    int row   = (color == WHITE) ? 7 : 0;
    int enemy = !color;

    if (m.fr != row || m.fc != 4 || m.tr != row) return 0;

    if (m.tc == 6) {                                /* king side */
        int right = (color == WHITE) ? g->castle_wk : g->castle_bk;
        if (!right) return 0;
        if (g->sq[row][5] != EMPTY || g->sq[row][6] != EMPTY) return 0;
        if (is_square_attacked(g, row, 4, enemy) ||
            is_square_attacked(g, row, 5, enemy) ||
            is_square_attacked(g, row, 6, enemy)) return 0;
        return 1;
    }
    if (m.tc == 2) {                                /* queen side */
        int right = (color == WHITE) ? g->castle_wq : g->castle_bq;
        if (!right) return 0;
        if (g->sq[row][1] != EMPTY || g->sq[row][2] != EMPTY ||
            g->sq[row][3] != EMPTY) return 0;
        if (is_square_attacked(g, row, 4, enemy) ||
            is_square_attacked(g, row, 3, enemy) ||
            is_square_attacked(g, row, 2, enemy)) return 0;
        return 1;
    }
    return 0;
}

int is_pseudo_legal(const Game *g, Move m)
{
    if (!on_board(m.fr, m.fc) || !on_board(m.tr, m.tc)) return 0;
    if (m.fr == m.tr && m.fc == m.tc) return 0;

    char p    = g->sq[m.fr][m.fc];
    int color = piece_color(p);

    if (color != g->turn) return 0;                         /* must move own piece */
    if (piece_color(g->sq[m.tr][m.tc]) == color) return 0;  /* can't capture own   */

    char up = (char)toupper((unsigned char)p);
    if (up == 'P')
        return pawn_move(g, m, color);
    if (up == 'K' && m.tr == m.fr && abs(m.tc - m.fc) == 2)
        return can_castle(g, m, color);

    return attacks(g, m.fr, m.fc, m.tr, m.tc);
}
