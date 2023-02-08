#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>

//#include "terminal.h"

#define CMD_SIZE 300
#define BREAK_CHAR -1
#define TOSTART -2
#define TOEND -3
#define LEFTARROW -4
#define RIGHTARROW -5
#define DEL -6
#define BACKSPACE -7
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

typedef struct _position {
    int y;
    int x;
} position;

void io_handler();

void print_prompt() {
    char dir[CMD_SIZE];
    getcwd(dir, CMD_SIZE);
    printw("%s:wish> ", dir);
}

position getNewCursorPosition(position start, int length) {
    position max;
    getmaxyx(stdscr, max.y, max.x);

    position end;
    end.y = start.y + (start.x + length)/max.x;
    end.x = (start.x + length)%max.x;

    return end;
}

int main () {

    int max_row, max_col;
    char cmd[CMD_SIZE];
    
    NCURSES_SETUP(max_row, max_col);

    while ( TRUE ) {
        print_prompt(); io_handler(cmd);
        printw("\n[CMD] %s\n", cmd);
        for(int i = 0; i < CMD_SIZE; i++) cmd[i] = '\0';
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

void sigexit_handler() {
    exit(0);
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

void appnd(char *str, char c, int index) {
    int currIndex = index;
    char temp[300];
    strncpy(temp, str + currIndex, strlen(str) - currIndex + 1);
    str[currIndex] = c;
    int end = currIndex+1;
    for(int i = 0; i < strlen(temp) && i < CMD_SIZE-1; i++) {
        str[end++] = temp[i];
    }
    str[end] = '\0';
}

void del(char *str, int currIndex) {
    char temp[300];
    strncpy(temp, str + currIndex + 1, strlen(str) - (currIndex + 1) + 1);
    int end = currIndex;
    for(int i = 0; i < strlen(temp) && i < CMD_SIZE-1; i++) {
        str[end++] = temp[i];
    }
    str[end] = '\0';
}

void empty_buffer(char *str) {
    strcpy(str, "");
}

int handle_char(int ch, char *cmd, int *currIndex) {
    // checking what the character is

    if (ch == CTRL(c)) {
        sigterm_handler();
    } else if (ch == CTRL(z) ) {
        sigbg_handler();
    } else if (ch == CTRL(d) ) {
        sigexit_handler(); //for now
    } else if (ch == KEY_UP) {
        keyup_handler();
    } else if (ch == KEY_DOWN) {
        keydown_handler();
    } else if (ch == CTRL(a)) {
        return TOSTART;
    } else if (ch == CTRL(e)) {
        return TOEND;
    } else if (ch == KEY_LEFT) {
        return LEFTARROW;
    } else if (ch == KEY_RIGHT) {
        return RIGHTARROW;
    } else if (ch == KEY_BACKSPACE) {
        return BACKSPACE;
    } else if (ch == KEY_DC) {
        return DEL;
    } else if (ch == KEY_ENTER || ch == 10 || ch == 13 ) { // Handle this
        //printw("HELO\n");
        refresh();
        return BREAK_CHAR;
    } else if (ch > 31 && ch < 127 ) {
        appnd(cmd, (char) ch, *currIndex);
        clrtobot();
        addstr(cmd + (*currIndex));
        refresh();
        (*currIndex)++;
    }

    return 0;
}

/**
 * IO Handler, on key-to-key basis
*/
void io_handler (char *cmd) {

    // character input
    int ch_in, currIndex = 0;

    position start, end, current;

    getyx(stdscr, start.y, start.x);
    current = start;

    // checking for special values
    while ( ( ch_in = getch() ) ) { 

        standend();

        int signal = handle_char(ch_in, cmd, &currIndex);
        current = getNewCursorPosition(start, currIndex);
        move(current.y, current.x);
        end = getNewCursorPosition(start, strlen(cmd));

        if (signal == BREAK_CHAR) {
            currIndex = strlen(cmd)-1;
            move(end.y, end.x);
            refresh();
            return;
        }
        else if(signal == TOSTART) {
            move(start.y, start.x);
            currIndex = 0;
        }
        else if(signal == TOEND) {
            move(end.y, end.x);
            currIndex = strlen(cmd);
        }
        else if(signal == LEFTARROW) {
            if(currIndex > 0) {
                currIndex--;
                int temp_x = getmaxx(stdscr);
                if(current.x == 0) {current.y--; current.x = temp_x;}
                else {current.x--;}
                move(current.y, current.x);
            }
        }
        else if(signal == RIGHTARROW) {
            if(currIndex < strlen(cmd)-1) {
                currIndex++;
                int temp_x = getmaxx(stdscr);
                if(current.x + 1 > temp_x) {current.y++; current.x = 0;}
                else {current.x++;}
                move(current.y, current.x);
            }
        }
        else if(signal == DEL) {
            del(cmd, currIndex);
            clrtobot();
            addstr(cmd + currIndex);
            refresh();
            move(current.y, current.x);
        }
        else if(signal == BACKSPACE) {
            // same as LEFT ARROW then DEL
            if(currIndex > 0) {
                currIndex--;
                int temp_x = getmaxx(stdscr);
                if(current.x == 0) {current.y--; current.x = temp_x;}
                else {current.x--;}
                move(current.y, current.x);
                del(cmd, currIndex);
                clrtobot();
                addstr(cmd + currIndex);
                refresh();
                move(current.y, current.x);
            }
        }
        
        // int n = strlen(cmd);
        // for (int i = 0; i < n; i ++) {
        //     if (i == (n + highlight)) 
        // }
    }
}
