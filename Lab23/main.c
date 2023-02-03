#include "main.h"
#define BUFFER_SZ 256

void printPrompt()
{
    printf("\033[0;35m");
    printf("wish -> ");
    printf("\033[0m");
}

int wish_execute_cmd(char **args)
{
    pid_t pid, wpid;
    int status;

    // create child process to run the command
    pid = fork();
    if (pid < 0)
    {
        // Error forking
        perror("\nWish is dying.\n");
    }
    else if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror("wish");
        }
        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
            wpid++;
            wpid--;
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

char *wish_builtin_cmds[] = {
    "cd",
    "clear",
    "help",
    "exit"};

int cd_wish(char **args);
int clear_wish();
int help_wish();
int exit_wish();

int (*builtin_func[])(char **) = {
    &cd_wish,
    &clear_wish,
    &help_wish,
    &exit_wish};

char *wish_read_cmd()
{
    char *cmd_buf = (char *)malloc(BUFFER_SZ * sizeof(char));
    if (cmd_buf == 0)
    {
        perror("\nMemory Exhausted!\n");
        exit(EXIT_FAILURE);
    }

    char ch;
    long long c = 0, max = BUFFER_SZ;
    // loop to read characters untill newline is entered
    do
    {
        ch = getchar();
        cmd_buf[c] = ch;
        c++;
        if (c == max - 1)
        {
            max *= 2;
            cmd_buf = (char *)realloc(cmd_buf, max);
            if (cmd_buf == 0)
            {
                perror("\nMemory Exhausted!\n");
                exit(EXIT_FAILURE);
            }
        }
    } while (ch != '\n');
    c = c - 1;
    cmd_buf[c] = '\0';
    return cmd_buf;
}

// implement the parse function to make args 2d array, current the whole cmd string entered is passed as args
// wish_parse_cmd(){

// }

int execute_cmd(char **args)
{

    if (args[0] == NULL)
    {
        // empty command
        return 1;
    }

    for (int i = 0; i < 4; i++)
    {
        // printf("\narg[0] = %s, built_in = %s\n", args[0], wish_builtin_cmds[i]);
        if (strcmp(args[0], wish_builtin_cmds[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }
    // if no such builtin cmd exists
    return wish_execute_cmd(args);
}

int main(int argc, char **argv)
{
    // temporarily making void for compilation
    (void)argc;
    (void)argv;

    printf("Welcome to the Wish shell\n");

    char *cmd_str;
    char *argmts[2];
    int cmd_return_status;

    do
    {
        printPrompt();

        // read the command entered by user
        cmd_str = wish_read_cmd();
        printf("\nYou entered: %s\n", cmd_str);

        // parse the command string
        // argmts = wish_parse_cmd();
        if (strcmp(cmd_str, "cd") == 0)
        {
            argmts[0]="cd";
        }
        else if (strcmp(cmd_str, "clear") == 0)
        {
            argmts[0] = "clear";
        }
        else if (strcmp(cmd_str, "help") == 0)
        {
            argmts[0] = "help";
        }
        else if (strcmp(cmd_str, "exit") == 0)
        {
            argmts[0] = "exit";
        }
        argmts[1] = "..";


        // execute the command
        cmd_return_status = execute_cmd(argmts);

    } while (cmd_return_status);

    exit_wish();

    return 0;
}

// implemetation of wish's built in cmds
int cd_wish(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "wish: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("wish");
        }
    }
    return 1;
}

int clear_wish()
{
    system("clear");
    return 1;
}

int help_wish()
{
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < 4; i++)
    {
        printf("  %s\n", wish_builtin_cmds[i]);
    }

    printf("Use the man command for information on other programs.\n");
    printf("\nCheeeeetah Run Run Run\n");
    return 1;
}

int exit_wish()
{
    printf("\033[0;36m");
    printf("Wish is leaving. Have a nice day!\n");
    printf("\033[0m");
    return 0;
}