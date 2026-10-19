/* ============================================================
 * PART 6: gui.c  --  Graphical chess using raylib
 *
 * Replaces main.c (the text version). Reuses board.c, moves.c
 * and rules.c unchanged.
 *
 * Controls:
 *   Left click  - select a piece, then click a highlighted square
 *   U           - undo last move
 *   R           - new game
 *   Esc         - cancel selection
 * ============================================================ */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "raylib.h"

/* raylib defines WHITE and BLACK as colours, but our chess code uses them
 * as player numbers. Remove raylib's versions before including ours. */
#undef WHITE
#undef BLACK

#include "board.h"
#include "moves.h"
#include "rules.h"

/* ---------- Layout ---------- */
#define SQ        80                     /* square size in pixels */
#define BX        40                     /* board left edge       */
#define BY        40                     /* board top edge        */
#define BOARD_PX  (SQ * 8)
#define PANEL_X   (BX + BOARD_PX + 40)
#define PANEL_W   260
#define WIN_W     (PANEL_X + PANEL_W)
#define WIN_H     (BY + BOARD_PX + 40)
#define MAX_HIST  1024

/* ---------- Colours ---------- */
static const Color C_PURE_BLACK = {   0,   0,   0, 255 };
static const Color C_PURE_WHITE = { 255, 255, 255, 255 };
static const Color C_BG        = {  38,  36,  33, 255 };
static const Color C_LIGHT     = { 240, 217, 181, 255 };
static const Color C_DARK      = { 181, 136,  99, 255 };
static const Color C_LASTMOVE  = { 205, 210,  70, 130 };
static const Color C_SELECTED  = {  90, 160, 230, 150 };
static const Color C_HINT      = {  20,  20,  20,  70 };
static const Color C_CHECK     = { 230,  50,  50, 170 };
static const Color C_TEXT      = { 235, 230, 220, 255 };
static const Color C_MUTED     = { 160, 155, 145, 255 };
static const Color C_BTN       = {  70,  66,  60, 255 };
static const Color C_BTN_HOVER = {  95,  90,  82, 255 };

/* ---------- Game state ---------- */
static Game  g;
static Game  hist[MAX_HIST];       /* positions before each move (for undo) */
static Move  moves[MAX_HIST];      /* moves played                          */
static char  notes[MAX_HIST][32];  /* move text like "e2-e4"                */
static int   nhist = 0;

static int   sel_r = -1, sel_c = -1;   /* selected square              */
static int   legal[8][8];              /* legal targets of selection   */
static int   status = ONGOING;
static int   in_check = 0;

static int   promo_pending = 0;        /* waiting for promotion choice */
static Move  promo_move;

/* ---------- Piece font ---------- */
static Font  piece_font;
static int   have_font = 0;

static void load_piece_font(void)
{
    /* Solid chess symbols: king, queen, rook, bishop, knight, pawn */
    int cps[6] = { 0x265A, 0x265B, 0x265C, 0x265D, 0x265E, 0x265F };
    const char *paths[] = {
        "pieces.ttf",                                        /* your own font */
        "C:/Windows/Fonts/seguisym.ttf",                     /* Windows       */
        "/System/Library/Fonts/Apple Symbols.ttf",           /* macOS         */
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",   /* Linux         */
    };
    for (int i = 0; i < (int)(sizeof paths / sizeof paths[0]); i++) {
        if (FileExists(paths[i])) {
            piece_font = LoadFontEx(paths[i], 96, cps, 6);
            if (piece_font.texture.id != GetFontDefault().texture.id) {
                SetTextureFilter(piece_font.texture, TEXTURE_FILTER_BILINEAR);
                have_font = 1;
                return;
            }
        }
    }
}

static int glyph_for(char p)
{
    switch (toupper((unsigned char)p)) {
    case 'K': return 0x265A;
    case 'Q': return 0x265B;
    case 'R': return 0x265C;
    case 'B': return 0x265D;
    case 'N': return 0x265E;
    default:  return 0x265F;
    }
}

/* Draw one piece inside a square of `size` pixels at (x,y). */
static void draw_piece(char p, float x, float y, float size)
{
    int white = isupper((unsigned char)p);
    Color fill = white ? (Color){ 250, 248, 240, 255 } : (Color){ 35, 35, 35, 255 };
    Color edge = white ? (Color){  40,  40,  40, 255 } : (Color){ 15, 15, 15, 255 };

    if (have_font) {
        int len = 0;
        char s[8] = { 0 };
        const char *utf8 = CodepointToUTF8(glyph_for(p), &len);  /* sets len first */
        memcpy(s, utf8, (size_t)len);

        float fs  = size * 0.86f;
        Vector2 m = MeasureTextEx(piece_font, s, fs, 0);
        Vector2 pos = { x + (size - m.x) / 2, y + (size - m.y) / 2 };

        /* soft shadow */
        DrawTextEx(piece_font, s, (Vector2){ pos.x + 2, pos.y + 3 }, fs, 0, Fade(C_PURE_BLACK, 0.25f));
        /* outline: draw the glyph shifted in 8 directions */
        for (int dx = -2; dx <= 2; dx += 2)
            for (int dy = -2; dy <= 2; dy += 2)
                if (dx || dy)
                    DrawTextEx(piece_font, s, (Vector2){ pos.x + dx, pos.y + dy }, fs, 0, edge);
        DrawTextEx(piece_font, s, pos, fs, 0, fill);
    } else {
        /* Fallback if no chess font: a disc with the piece letter */
        float cx = x + size / 2, cy = y + size / 2, r = size * 0.36f;
        DrawCircle((int)cx + 2, (int)cy + 3, r, Fade(C_PURE_BLACK, 0.25f));
        DrawCircle((int)cx, (int)cy, r, edge);
        DrawCircle((int)cx, (int)cy, r - 3, fill);
        char t[2] = { (char)toupper((unsigned char)p), 0 };
        int fs = (int)(size * 0.4f);
        DrawText(t, (int)(cx - MeasureText(t, fs) / 2), (int)(cy - fs / 2), fs,
                 white ? (Color){ 35, 35, 35, 255 } : (Color){ 250, 248, 240, 255 });
    }
}

/* ---------- Game helpers ---------- */
static void refresh_status(void)
{
    status   = game_status(&g);
    in_check = is_in_check(&g, g.turn);
}

static void new_game(void)
{
    board_init(&g);
    nhist = 0;
    sel_r = sel_c = -1;
    promo_pending = 0;
    memset(legal, 0, sizeof legal);
    refresh_status();
}

static void compute_legal(void)
{
    memset(legal, 0, sizeof legal);
    if (sel_r < 0) return;
    Move m = { sel_r, sel_c, 0, 0, 'q' };
    for (m.tr = 0; m.tr < 8; m.tr++)
        for (m.tc = 0; m.tc < 8; m.tc++)
            legal[m.tr][m.tc] = is_legal(&g, m);
}

static void make_move(Move m)
{
    if (nhist >= MAX_HIST) return;

    char p = g.sq[m.fr][m.fc];
    int capture = g.sq[m.tr][m.tc] != EMPTY ||
                  (toupper((unsigned char)p) == 'P' && m.fc != m.tc);

    char *n = notes[nhist];
    snprintf(n, sizeof notes[0], "%c%d%c%c%d",
             'a' + m.fc, 8 - m.fr, capture ? 'x' : '-', 'a' + m.tc, 8 - m.tr);
    if (m.promo) {
        size_t L = strlen(n);
        n[L] = '='; n[L + 1] = (char)toupper((unsigned char)m.promo); n[L + 2] = 0;
    }

    hist[nhist]  = g;
    moves[nhist] = m;
    nhist++;

    apply_move(&g, m);
    sel_r = sel_c = -1;
    memset(legal, 0, sizeof legal);
    refresh_status();
}

static void undo(void)
{
    if (nhist == 0) return;
    nhist--;
    g = hist[nhist];
    sel_r = sel_c = -1;
    promo_pending = 0;
    refresh_status();
}

/* ---------- UI helpers ---------- */
static int button(Rectangle r, const char *label)
{
    int hover = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRounded(r, 0.3f, 8, hover ? C_BTN_HOVER : C_BTN);
    int w = MeasureText(label, 20);
    DrawText(label, (int)(r.x + (r.width - w) / 2), (int)(r.y + (r.height - 20) / 2), 20, C_TEXT);
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static Rectangle promo_box(int i)
{
    float total = 4 * SQ + 3 * 10;
    float x0 = BX + (BOARD_PX - total) / 2;
    return (Rectangle){ x0 + i * (SQ + 10), BY + BOARD_PX / 2 - SQ / 2, SQ, SQ };
}

/* ---------- Input ---------- */
static void handle_board_click(void)
{
    Vector2 mp = GetMousePosition();
    int c = (int)((mp.x - BX) / SQ);
    int r = (int)((mp.y - BY) / SQ);
    if (mp.x < BX || mp.y < BY || c > 7 || r > 7) return;

    /* Second click: try to move to this square */
    if (sel_r >= 0 && legal[r][c]) {
        Move m = { sel_r, sel_c, r, c, 0 };
        int pawn = toupper((unsigned char)g.sq[sel_r][sel_c]) == 'P';
        if (pawn && (r == 0 || r == 7)) {
            promo_pending = 1;
            promo_move = m;
        } else {
            make_move(m);
        }
        return;
    }

    /* Otherwise select own piece (or clear selection) */
    if (piece_color(g.sq[r][c]) == g.turn && !(r == sel_r && c == sel_c)) {
        sel_r = r; sel_c = c;
    } else {
        sel_r = sel_c = -1;
    }
    compute_legal();
}

static void handle_promo_click(void)
{
    const char opts[4] = { 'q', 'r', 'b', 'n' };
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    for (int i = 0; i < 4; i++)
        if (CheckCollisionPointRec(GetMousePosition(), promo_box(i))) {
            promo_move.promo = opts[i];
            promo_pending = 0;
            make_move(promo_move);
            return;
        }
}

/* ---------- Drawing ---------- */
static void draw_board(void)
{
    Vector2 mp = GetMousePosition();
    int hr = (int)((mp.y - BY) / SQ), hc = (int)((mp.x - BX) / SQ);
    int hovering = mp.x >= BX && mp.y >= BY && hr < 8 && hc < 8;

    /* frame */
    DrawRectangle(BX - 6, BY - 6, BOARD_PX + 12, BOARD_PX + 12, (Color){ 60, 45, 35, 255 });

    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            int x = BX + c * SQ, y = BY + r * SQ;
            DrawRectangle(x, y, SQ, SQ, (r + c) % 2 ? C_DARK : C_LIGHT);

            if (nhist > 0) {
                Move lm = moves[nhist - 1];
                if ((r == lm.fr && c == lm.fc) || (r == lm.tr && c == lm.tc))
                    DrawRectangle(x, y, SQ, SQ, C_LASTMOVE);
            }
            if (r == sel_r && c == sel_c)
                DrawRectangle(x, y, SQ, SQ, C_SELECTED);
            if (in_check && g.sq[r][c] == (g.turn == WHITE ? 'K' : 'k'))
                DrawCircleGradient(x + SQ / 2, y + SQ / 2, SQ * 0.55f, C_CHECK, Fade(C_CHECK, 0));
            if (hovering && r == hr && c == hc && !promo_pending && status == ONGOING)
                DrawRectangleLinesEx((Rectangle){ (float)x, (float)y, SQ, SQ }, 3, Fade(C_PURE_WHITE, 0.5f));
        }

    /* coordinates */
    for (int i = 0; i < 8; i++) {
        char f[2] = { (char)('a' + i), 0 }, rk[2] = { (char)('8' - i), 0 };
        DrawText(f,  BX + i * SQ + SQ / 2 - 5, BY + BOARD_PX + 12, 18, C_MUTED);
        DrawText(rk, BX - 26, BY + i * SQ + SQ / 2 - 9, 18, C_MUTED);
    }

    /* pieces */
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (g.sq[r][c] != EMPTY)
                draw_piece(g.sq[r][c], (float)(BX + c * SQ), (float)(BY + r * SQ), SQ);

    /* legal-move hints */
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (legal[r][c]) {
                Vector2 ctr = { BX + c * SQ + SQ / 2.0f, BY + r * SQ + SQ / 2.0f };
                if (g.sq[r][c] != EMPTY)
                    DrawRing(ctr, SQ * 0.40f, SQ * 0.48f, 0, 360, 32, C_HINT);
                else
                    DrawCircleV(ctr, SQ * 0.15f, C_HINT);
            }
}

static void draw_panel(void)
{
    int x = PANEL_X, y = BY;

    DrawText("Chess in C", x, y, 32, C_TEXT);
    y += 50;

    /* whose turn */
    Color tc = g.turn == WHITE ? (Color){ 250, 248, 240, 255 } : (Color){ 30, 30, 30, 255 };
    DrawCircle(x + 10, y + 11, 10, C_MUTED);
    DrawCircle(x + 10, y + 11, 8, tc);
    DrawText(g.turn == WHITE ? "White to move" : "Black to move", x + 30, y, 22, C_TEXT);
    y += 34;

    if (status == ONGOING && in_check)
        DrawText("CHECK!", x, y, 24, (Color){ 240, 80, 80, 255 });
    y += 40;

    /* move list */
    DrawText("Moves", x, y, 20, C_MUTED);
    y += 28;
    DrawLine(x, y - 4, x + PANEL_W - 30, y - 4, C_BTN);

    int full = (nhist + 1) / 2;
    int max_rows = 16;
    int start = full > max_rows ? full - max_rows : 0;
    for (int i = start; i < full; i++) {
        char num[16];
        snprintf(num, sizeof num, "%d.", i + 1);
        DrawText(num, x, y, 18, C_MUTED);
        DrawText(notes[2 * i], x + 40, y, 18, C_TEXT);
        if (2 * i + 1 < nhist)
            DrawText(notes[2 * i + 1], x + 130, y, 18, C_TEXT);
        y += 24;
    }

    /* buttons */
    float by = (float)(BY + BOARD_PX - 100);
    if (button((Rectangle){ (float)x, by, 105, 40 }, "Undo"))      undo();
    if (button((Rectangle){ (float)x + 115, by, 115, 40 }, "New Game")) new_game();

    DrawText("Click a piece, then a square", x, BY + BOARD_PX - 40, 16, C_MUTED);
    DrawText("U = undo   R = restart",       x, BY + BOARD_PX - 18, 16, C_MUTED);
}

static void draw_promo_popup(void)
{
    const char opts[4] = { 'Q', 'R', 'B', 'N' };
    DrawRectangle(BX, BY, BOARD_PX, BOARD_PX, Fade(C_PURE_BLACK, 0.55f));
    const char *t = "Promote to:";
    DrawText(t, BX + (BOARD_PX - MeasureText(t, 28)) / 2, BY + BOARD_PX / 2 - SQ / 2 - 45, 28, C_TEXT);

    for (int i = 0; i < 4; i++) {
        Rectangle b = promo_box(i);
        int hover = CheckCollisionPointRec(GetMousePosition(), b);
        DrawRectangleRounded(b, 0.15f, 8, hover ? C_LIGHT : C_DARK);
        char p = g.turn == WHITE ? opts[i] : (char)tolower((unsigned char)opts[i]);
        draw_piece(p, b.x, b.y, SQ);
    }
}

static void draw_game_over(void)
{
    char msg[64];
    if (status == CHECKMATE)
        snprintf(msg, sizeof msg, "Checkmate! %s wins", g.turn == WHITE ? "Black" : "White");
    else
        snprintf(msg, sizeof msg, "Stalemate - Draw");

    DrawRectangle(BX, BY, BOARD_PX, BOARD_PX, Fade(C_PURE_BLACK, 0.55f));
    DrawText(msg, BX + (BOARD_PX - MeasureText(msg, 40)) / 2, BY + BOARD_PX / 2 - 40, 40, C_TEXT);
    const char *h = "Press R or click New Game";
    DrawText(h, BX + (BOARD_PX - MeasureText(h, 20)) / 2, BY + BOARD_PX / 2 + 15, 20, C_MUTED);
}

/* ---------- Main ---------- */
int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(WIN_W, WIN_H, "Chess in C");
    SetExitKey(KEY_NULL);          /* don't close the window on Esc */
    SetTargetFPS(60);

    load_piece_font();
    new_game();

    while (!WindowShouldClose()) {
        /* --- input --- */
        if (IsKeyPressed(KEY_R)) new_game();
        if (IsKeyPressed(KEY_U)) undo();
        if (IsKeyPressed(KEY_ESCAPE)) { sel_r = sel_c = -1; promo_pending = 0; compute_legal(); }

        if (promo_pending)
            handle_promo_click();
        else if (status == ONGOING && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            handle_board_click();

        if (sel_r < 0) memset(legal, 0, sizeof legal);

        /* --- draw --- */
        BeginDrawing();
        ClearBackground(C_BG);
        draw_board();
        draw_panel();
        if (promo_pending)        draw_promo_popup();
        else if (status != ONGOING) draw_game_over();
        EndDrawing();
    }

    if (have_font) UnloadFont(piece_font);
    CloseWindow();
    return 0;
}
