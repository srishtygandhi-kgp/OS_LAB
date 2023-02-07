#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>

//#include "terminal.h"

#define CMD_SIZE 300
#define BREAK_CHAR -1
//#define CTRL_Z 26

#define CTRL(X) ( #X[0] - 'a' + 1 )

#define NCURSES_SETUP(MAX_ROW,MAX_COL)          \
    do {                                        \
        initscr();                              \
        raw();                                  \
        keypad(stdscr, TRUE);                   \
        noecho();                               \
        getmaxyx(stdscr, MAX_ROW, MAX_COL);     \
        nonl();                                 \
    } while (0)

void io_handler();

void print_prompt() {
    char dir[CMD_SIZE];
    getcwd(dir, CMD_SIZE);
    printw("%s:wish> ", dir);
}

int main () {

    int max_row, max_col;
    char cmd[CMD_SIZE];
    
    NCURSES_SETUP(max_row, max_col);

    while ( TRUE ) {
    
        print_prompt(); io_handler(cmd);
        printw("\n[CMD] %s\n", cmd);
    }

    return 0;
}

/**
 * Handlers
*/
void sigterm_handler() {
    printw("[LOG] CTRL-C\n");
    refresh();
}

void sigbg_handler() {
    printw("[LOG] CTRL-Z\n");
    refresh();
}

void keyup_handler() {
    printw("[LOG] up-key\n");
    refresh();
}

void keydown_handler() {
    printw("[LOG] down-key\n");
    refresh();
}

/**
 * Handle char
*/

void appnd(char *str, char c) {
    int i;
    for (i = 0; str[i]; i ++);
    str[i ++] = c;
}

void empty_buffer(char *str) {
    strcpy(str, "");
}

int handle_char(int ch, char *cmd) {
    // checking what the character is

    /*if (ch == CTRL_CZ) {
        empty_buffer(cmd);
        sigterm_handler();
    } else*/if (ch == CTRL(c)) {
        printw("CTRL+C");
    } else if (ch == CTRL(z) ) {
        printw("CTRL+Z");
    } else if (ch == KEY_UP) {
        keyup_handler();
    } else if (ch == KEY_DOWN) {
        keydown_handler();
    } else if (ch == KEY_ENTER || ch == 10 || ch == 13 ) { // Handle this
        //printw("HELO\n");
        refresh();
        return BREAK_CHAR;
    } else if (ch > 31 && ch < 127 ) {
        //addch(ch | A_STANDOUT);
        appnd(cmd, (char) ch);
    }

    return 0;
}

/**
 * IO Handler, on key-to-key basis
*/
void io_handler (char *cmd) {

    // character input
    int ch_in, y, x, highlight = -1;

    getyx(stdscr, y, x);
    printw("%d, %d", y, x);

    // checking for special values
    while ( ( ch_in = getch() ) ) { 
        standend();
        if (handle_char(ch_in, cmd, &highlight) == BREAK_CHAR) 
            return;
        move(y, x);
        
        int n = strlen(cmd);
        for (int i = 0; i < n; i ++) {
            if (i == (n + highlight)) 
        }
    }
}
