/* ============================================================
 * PART 4: input.c  --  Convert "e2e4" into board coordinates
 * ============================================================ */
#include <ctype.h>
#include <string.h>
#include "input.h"

int parse_move(const char *line, Move *m)
{
    char buf[8];
    int n = 0;

    /* Keep only letters and digits (ignore spaces, dashes, newline) */
    for (int i = 0; line[i] && n < 7; i++)
        if (isalnum((unsigned char)line[i]))
            buf[n++] = (char)tolower((unsigned char)line[i]);
    buf[n] = '\0';

    if (n != 4 && n != 5) return 0;
    if (buf[0] < 'a' || buf[0] > 'h' || buf[2] < 'a' || buf[2] > 'h') return 0;
    if (buf[1] < '1' || buf[1] > '8' || buf[3] < '1' || buf[3] > '8') return 0;

    m->fc = buf[0] - 'a';
    m->fr = 8 - (buf[1] - '0');
    m->tc = buf[2] - 'a';
    m->tr = 8 - (buf[3] - '0');
    m->promo = 0;

    if (n == 5) {
        if (!strchr("qrbn", buf[4])) return 0;
        m->promo = buf[4];
    }
    return 1;
}
