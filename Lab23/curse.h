#ifndef CURSE_H
#define CURSE_H

#define CMD_SIZE 300
#define BREAK_CHAR -1
#define TOSTART -2
#define TOEND -3
#define LEFTARROW -4
#define RIGHTARROW -5
#define DEL -6
#define BACKSPACE -7

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

void io_handler(char *cmd);

void print_prompt();

void print_file_to_curses(const char *filepath);

#endif