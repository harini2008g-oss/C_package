/* ============================================================
 * PART 5: main.c  --  The game loop
 * ============================================================ */
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "moves.h"
#include "rules.h"
#include "input.h"

int main(void)
{
    Game g;
    Move m;
    char line[64];

    board_init(&g);

    printf("=== Two-Player Chess in C ===\n");
    printf("Uppercase = White, lowercase = Black.\n");
    printf("Enter moves like: e2e4  or  e2 e4\n");
    printf("Castling: move the king two squares (e1g1). Promotion: e7e8q\n");
    printf("Type 'quit' to exit.\n");

    for (;;) {
        board_print(&g);

        int status = game_status(&g);
        if (status == CHECKMATE) {
            printf("Checkmate! %s wins.\n", g.turn == WHITE ? "Black" : "White");
            break;
        }
        if (status == STALEMATE) {
            printf("Stalemate! The game is a draw.\n");
            break;
        }
        if (is_in_check(&g, g.turn))
            printf("%s is in CHECK!\n", g.turn == WHITE ? "White" : "Black");

        printf("%s to move: ", g.turn == WHITE ? "White" : "Black");
        if (!fgets(line, sizeof line, stdin)) break;
        if (strncmp(line, "quit", 4) == 0) break;

        if (!parse_move(line, &m)) {
            printf("Invalid format. Try something like e2e4.\n");
            continue;
        }
        if (!is_legal(&g, m)) {
            printf("Illegal move. Try again.\n");
            continue;
        }
        apply_move(&g, m);
    }

    printf("Thanks for playing!\n");
    return 0;
}
