#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>

#include "curse.h"
#include "history.h"

void print_prompt() {
    printw("wish -> ");
}

void print_file_to_curses(const char *filepath) {

    FILE *fp = fopen(filepath, "r");
    size_t size = CMD_SIZE;
    char *buffer = (char *)malloc(size*sizeof(char));

    while(1) {
        size_t size = getline(&buffer, &size, fp);
        if(size == -1) break;
        buffer[size-1] = '\0';
        printw("%s\n", buffer);
    }
    free(buffer);
}

position getNewCursorPosition(position start, int length) {
    position max;
    getmaxyx(stdscr, max.y, max.x);

    position end;
    end.y = start.y + (start.x + length)/max.x;
    end.x = (start.x + length)%max.x;

    return end;
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
        return UPARROW;
    } else if (ch == KEY_DOWN) {
        return DOWNARROW;
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
    int historyIndex = -1; //-1 is current

    char *cmdcpy = (char *)malloc(CMD_SIZE*sizeof(char));

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

        if(historyIndex == -1 && signal != DOWNARROW && signal != UPARROW) {
            strcpy(cmdcpy, cmd);
        }

        if (signal == BREAK_CHAR) {
            currIndex = strlen(cmd)-1;
            move(end.y, end.x);
            printw("\n");
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
        else if(signal == UPARROW) {
            char* historyCmd = (char *)malloc(CMD_SIZE*sizeof(char));
            historyCmd = getHistory(historyIndex + 1);
            if(historyCmd == NULL) continue;
            strcpy(cmd, historyCmd);
            move(start.y, start.x);
            clrtobot();
            addstr(cmd);
            currIndex = strlen(cmd)-1;
            historyIndex++;
            refresh();
        }
        else if(signal == DOWNARROW) {

        }
    }
}

// int main () {

//     int max_row, max_col;
//     char cmd[CMD_SIZE];
    
//     NCURSES_SETUP(max_row, max_col);

//     while ( TRUE ) {
//         print_prompt(); io_handler(cmd);
//         printw("\n[CMD] %s\n", cmd);
//         for(int i = 0; i < CMD_SIZE; i++) cmd[i] = '\0';
//     }

//     return 0;
// }