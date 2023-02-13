#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <glob.h>
#include <sys/wait.h>

#include <readline/readline.h>

#include "utils.h"
#include "myhistory.h"

#define LEN 10
#define PROMPT "wish ->"
#define CMD_SIZE 300

char *ltrim(char *s)
{
    size_t len = strlen(s);
    size_t i = 0;
    while (i < len && isspace(s[i]))
    {
        ++i;
    }
    memmove(s, s + i, len - i + 1);
    return s;
}

char *rtrim(char *s)
{
    int len = strlen(s);
    while (len > 0 && isspace(s[len - 1]))
    {
        --len;
    }
    s[len] = '\0';
    return s;
}

// function to check if given char is a wildcard char
int is_wildcard_char(char ch)
{
    if (ch == '*' || ch == '?')
        return 1;
    return 0;
}

// Split the string into several ones by the delimiter
vector split(char *cmd, char delim)
{
    int count = 0;
    const char *tmp = cmd;
    while (*tmp)
    {
        if (*tmp == delim)
        {
            count++;
        }
        tmp++;
    }
    count++;

    vector res;
    vector_init(&res);
    int i = 0;
    const char *start = cmd;
    tmp = cmd;
    while (*tmp)
    {
        if (*tmp == delim)
        {
            int len = tmp - start;
            char *result = (char *)malloc(len + 1);
            if (!result)
            {
                printf("Error allocating memory\n");
                // refresh();
                exit(1);
            }
            memcpy(result, start, len);
            result[len] = '\0';
            vector_add(&res, result);
            i++;
            start = tmp + 1;
        }
        tmp++;
    }
    int len = tmp - start;
    char *result = (char *)malloc(len + 1);
    if (!result)
    {
        printf("Error allocating memory\n");
        // refresh();
        exit(1);
    }
    memcpy(result, start, len);
    result[len] = '\0';
    vector_add(&res, result);

    return res;
}

// Splits the command into input and ouput
vector splitInputOuput(char *cmd)
{
    vector res;
    vector_init(&res);
    vector_add(&res, "");
    vector_add(&res, "");
    vector_add(&res, "");

    vector output;
    vector_init(&output);
    output = split(cmd, '>');

    // No output redirection
    if (vector_total(&output) == 1)
    {
        vector input;
        vector_init(&input);
        input = split(cmd, '<');

        // No input, output redirection
        if (vector_total(&input) == 1)
        {

            vector_set(&res, 0, rtrim(ltrim(vector_get(&output, 0))));
            return res;
        }

        // Input redirection present, no output redirection
        else
        {
            // Trim whitespaces
            vector_set(&res, 1, rtrim(ltrim(vector_get(&input, 1))));
            vector_set(&res, 0, rtrim(ltrim(vector_get(&input, 0))));
            return res;
        }
    }

    vector input;
    vector_init(&input);
    input = split(cmd, '<');

    // No input redirection, output redirection present
    if (vector_total(&input) == 1)
    {
        // Trim whitespaces
        vector_set(&res, 2, rtrim(ltrim(vector_get(&output, 1))));
        vector_set(&res, 0, rtrim(ltrim(vector_get(&output, 0))));
        return res;
    }

    // Input, output redirection present

    // Input redirection present in the second arg of the output
    vector vec_tmp;
    vector_init(&vec_tmp);
    vec_tmp = split(vector_get(&output, 0), '<');
    if (vector_total(&vec_tmp) == 1)
    {
        // Split the second argument into output and input respectively
        vector output_input;
        vector_init(&output_input);
        output_input = split(vector_get(&output, 1), '<');

        // Trim the whitespaces in out and in
        vector_set(&res, 2, rtrim(ltrim(vector_get(&output_input, 0))));
        vector_set(&res, 1, rtrim(ltrim(vector_get(&output_input, 1))));
        vector_set(&res, 0, rtrim(ltrim(vector_get(&output, 0))));
        return res;
    }

    // Input redirection present in the first arg of the output

    // Split the first argument into commmand and input respectively
    vector cmd_input;
    vector_init(&cmd_input);
    cmd_input = split(vector_get(&output, 0), '<');

    // Trim the whitespaces in out and in
    vector_set(&res, 1, rtrim(ltrim(vector_get(&cmd_input, 1))));
    vector_set(&res, 2, rtrim(ltrim(vector_get(&output, 1))));
    vector_set(&res, 0, rtrim(ltrim(vector_get(&cmd_input, 0))));

    return res;
}

// Open files and redirect input and output with files as arguments
void redirect(char *inp, char *out)
{
    int inp_fd, out_fd;

    // Open input redirecting file
    if (strlen(inp))
    {
        inp_fd = open(inp, O_RDONLY); // Open in read only mode
        if (inp_fd < 0)
        {
            printf("Error opening input file\n");
            // refresh();
            exit(EXIT_FAILURE);
        }
        // Redirect input
        if (dup2(inp_fd, 0) < 0)
        {
            printf("Input redirecting error");
            // refresh();
            exit(EXIT_FAILURE);
        }
    }

    // Open output redirecting file
    if (strlen(out))
    {
        out_fd = open(out, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU); // Open in create and truncate mode
        // Redirect output
        if (dup2(out_fd, 1) < 0)
        {
            printf("Output redirecting error\n");
            // refresh();
            exit(EXIT_FAILURE);
        }
    }
}

int get_parent_pid(int pid)
{
    char proc_path[1024];
    snprintf(proc_path, 1024, "/proc/%d/status", pid);
    FILE *f = fopen(proc_path, "r");
    if (f == NULL)
    {
        printf("-1, /proc/%d/status does not exist\n", pid);
        // refresh();
        return -1;
    }
    int ppid = -1;
    char line[1024];
    while (fgets(line, 1024, f) != NULL)
    {
        if (strncmp(line, "PPid:", 5) == 0)
        {
            ppid = atoi(line + 5);
            break;
        }
    }
    fclose(f);
    //   printf("%d", ppid);
    return ppid;
}

int get_num_childeren(int pid)
{
    // opens proc/[pid]/task/[pid]/childeren
    // and then find number of childeren in it
    if (pid == 0)
        return 0;
    int cnt = 0;
    char proc_path[1024];
    snprintf(proc_path, 1024, "/proc/%d/task/%d/children", pid, pid);
    // printf("Trying to open %s\n", proc_path);
    FILE *f = fopen(proc_path, "r");
    if (f == NULL)
    {
        printf("-1, /proc/%d/task/%d/children does not exist\n", pid, pid);
        exit(0);
    }
    char line[2048];
    vector child;
    vector_init(&child);
    if (fgets(line, 2048, f) != NULL)
    {
        child = split(line, ' ');
    }
    for (int i = 0; i < vector_total(&child); i++)
    {
        int child_pid = atoi((char *)vector_get(&child, i));
        cnt += 1;
        cnt += get_num_childeren(child_pid);
    }
    return cnt;
}

// void read_stat_file(int pid){
//     char proc_path[1024];
//     snprintf(proc_path, 1024, "/proc/%d/stat", pid);
//     FILE *f = fopen(proc_path, "r");
//     if (f == NULL) {
//         printf("-1, /proc/%d/stat does not exist\n", pid);
//         //refresh();
//         return ;
//     }
//     int ppid = -1;
//     char line[2048];
//     vector fields;
//     vector_init(&fields);
//     if(fgets(line, 2048, f) != NULL) {
//        fields = split(line, ' ');
//     }

//     printf("ppid -- %s", (char*)vector_get(&fields, 22));
//     //refresh();
//     for(int i = 0; i < vector_total(&fields); i++){
//         printf("%s ", (char*)vector_get(&fields,i));
//         //refresh();
//     }
//     fclose(f);
// }

void getparent(int pid)
{
    int parent_pid = get_parent_pid(pid);
    printf("Process ID: %d, Parent Process ID: %d\n", pid, parent_pid);
    // get_process_info(pid);
    //     read_stat_file(pid);
    printf("\n");
    // refresh();
    if (parent_pid != 1)
    {
        getparent(parent_pid);
    }
}

int sb_suggest(int pid)
{
    int arr[LEN][2];
    for (int i = 0; i < LEN; i++)
        arr[i][0] = 0;
    int i = 0;
    while (i < LEN)
    {
        if (pid == 1)
        {
            arr[i][0] = pid;
            arr[i][1] = get_num_childeren(pid);
            i++;
            break;
        }
        int ppid = get_parent_pid(pid);
        if ((pid == 0 || pid == -1))
        {
            break;
        }
        arr[i][0] = pid;
        arr[i][1] = get_num_childeren(pid);
        pid = ppid;
        i++;
    }

    // printf("i == %d\n",i);

    // for(int a = 0; a < (i); a++){
    // printf("%d -- %d\n",arr[a][0], arr[a][1]);
    // }

    // DIFFERENTIAL
    int cnt = i--;
    for (; i >= 0; i--)
    {
        arr[i][1] = arr[i][1] - arr[i - 1][1];
        // printf("%d -- %d\n", arr[i][0],arr[i][1]);
    }
    arr[cnt - 1][1] = 0;
    int max = arr[0][1];
    int max_ind = 0;
    for (int j = 1; j < cnt; j++)
    {
        // printf("%d -- %d\n", arr[j][0],arr[j][1]);
        if (max < arr[j][1])
        {
            max = arr[j][1];
            max_ind = j;
        }
    }
    return arr[max_ind][0];
}

void squash_bug(char *cmd)
{
    // syntax -- sb then number then flag
    int given_process_id = 0;
    int cnt = 3;
    int len = strlen(cmd);
    while (cnt < len)
    {
        if (cmd[cnt] >= '0' && cmd[cnt] <= '9')
        {
            given_process_id *= 10;
            given_process_id += (int)(cmd[cnt] - '0');
        }
        else
        {
            break;
        }
        cnt++;
    }
    if (strstr(cmd, " -suggest"))
    {
        printf("culprit --  %d\n", sb_suggest(given_process_id));
    }
    else
    {
        //printf("%d\n", given_process_id);
        getparent(given_process_id);
    }
}

// delep <filepath>
void delep(char *filePath)
{
    pid_t pID, wpid;
    int status = 0;
    pID = fork();
    vector flock_pids;
    vector_init(&flock_pids);
    // child process finds out pids holding lock over the file
    if (pID == 0)
    {
        int fd, inode;
        fd = open(filePath, O_RDONLY);
        if (fd < 0)
        {
            perror("open");
            return;
        }
        struct stat buf;
        int ret = fstat(fd, &buf);
        if (ret < 0)
        {
            perror("\nError in fstat\n");
            return;
        }
        // inode now contains inode number of the file with descriptor fd
        inode = buf.st_ino;
        // printf("inode of file = %d\n", inode);
        FILE *file_ptr = fopen("/proc/locks", "r");
        
        char *buffer = (char *)malloc(100 * sizeof(char));
        int p = 0, inodePID;
        size_t size = 10;
        while (1)
        {
            size = getline(&buffer, &size, file_ptr);
            if (size == -1)
                break;
            buffer[size - 1] = '\0';
            char *copy = (char *)malloc(size * sizeof(char));
            strcpy(copy, buffer);

            unsigned int i = 0;
            while (i < strlen(copy))
            {
                if (copy[i] == ' ')
                    break;
                i++;
            }
            i++;
            char ltype[50], bufPID[50], inode_buf[50];
            memset(&ltype, '\0', sizeof(ltype));
            memset(&bufPID, '\0', sizeof(bufPID));
            memset(&inode_buf, '\0', sizeof(inode_buf));
            int c = 0;
            // store the type
            while (i < strlen(copy))
            {
                if (copy[i] == ' ')
                    break;
                ltype[c++] = copy[i++];
            }
            ltype[c] = '\0';
            if (strcmp(ltype, "FLOCK") != 0)
                continue;
            i++;
            // skip the mode and type (read/write)
            while (i < strlen(copy))
            {
                if (copy[i] >= '0' && copy[i] <= '9')
                    break;
                i++;
            }
            // copy the pid
            c = 0;
            while (i < strlen(copy))
            {
                if (copy[i] == ' ')
                    break;
                bufPID[c++] = copy[i++];
            }
            bufPID[c] = '\0';

            // skip the until first semicolon
            while (i < strlen(copy))
            {
                if (copy[i] == ':')
                    break;
                i++;
            }
            i++;
            // skip the until second semicolon
            while (i < strlen(copy))
            {
                if (copy[i] == ':')
                    break;
                i++;
            }

            // copy the i node
            c = 0;
            i++;
            while (i < strlen(copy))
            {
                if (copy[i] == ' ')
                    break;
                inode_buf[c++] = copy[i++];
            }
            inode_buf[c] = '\0';
            inodePID = atoi(inode_buf); // convert string to int
            // printf("\npid = %s, inode = %d\n", bufPID, inodePID);

            if ((!strcmp(ltype, "FLOCK")) && (inodePID == inode))
            {
                // printf("\n%s, pid = %s, inode = %d\n", ltype, bufPID, inodePID);
                size = strlen(bufPID) + 1;
                char *str = (char *)malloc(size * sizeof(char));
                strcpy(str, bufPID);
                vector_add(&flock_pids, str);
            }
        }
        fclose(file_ptr);
        free(buffer);
    }
    // in parent process
    while ((wpid = wait(&status)) > 0)
        ; // parent waits for the child process

    char choice;
    if (vector_total(&flock_pids) == 0)
    {
        printf("File is not opened by any application. Do you want to delete? [y/n] ");
        // choice = getc(stdin);
        while(1) {
            choice = getc(stdin);
            if(choice == 'y' || choice == 'n')
            break;
            printf("Please enter choicce [y/n] ");
            getc(stdin);
        }
        if (choice == 'y')
        {
            remove(filePath);
        }
    }
    else
    {
        printf("File is opened by the following processes: \n");
        for (int i = 0; i < vector_total(&flock_pids); i++)
        {
            printf("%s\n", (char *)vector_get(&flock_pids, i));
        }
        printf("Do you want to kill all these processes and delete the file? [y/n] ");
        // choice = getc(stdin);
        while(1) {
            choice = getc(stdin);
            if(choice == 'y' || choice == 'n')
            break;
            printf("Please enter choicce [y/n] ");
            getc(stdin);
        }
        if (choice == 'y')
        {
            for (int i = 0; i < vector_total(&flock_pids); i++)
            {
                kill(atoi(vector_get(&flock_pids, i)), SIGKILL);
                printf("\nkill pid %d\n", atoi(vector_get(&flock_pids, i)));
            }
            remove(filePath);
        }
    }
    getchar();
}

// Execute the commands
void execCmd(char *cmd)
{
    // check fot the sb in front
    if (cmd[0] == 's' && cmd[1] == 'b' && cmd[2] == ' ')
    {
        printf("[sb RUNNING]\n");
        // refresh();
        squash_bug(cmd);
        return;
    }
    // Split the command and its arguments
    vector args;
    vector_init(&args);

    vector te;
    vector_init(&te);
    te = split(cmd, ' ');

    for (int i = 0; i < vector_total(&te); i++)
    {
        if (strlen(vector_get(&te, i)))
            vector_add(&args, vector_get(&te, i));
    }

    // Create a char* array for the arguments
    int initial_size = vector_total(&args) + 1;
    char **argv = (char **)calloc(initial_size, sizeof(char *));
    int cnt = 0, new_size = initial_size;

    for (int i = 0; i < vector_total(&args); i++)
    {
        char *arg_to_check = vector_get(&args, i); // Convert string to char *
        int flag = 0;                              // flag to check if argument contains a wildcard char
        for (int j = 0; j < strlen(arg_to_check); j++)
        {
            if (arg_to_check[j] == '"')
            {
                while (j < strlen(arg_to_check) && arg_to_check[j] != '"')
                {
                    j++;
                }
            }
            if (is_wildcard_char(arg_to_check[j]))
            {
                flag = 1;
            }
        }
        // if wildcard entry is present add the arguments
        if (flag)
        {
            char **arg_found;
            glob_t gstruct;
            int ret = glob(arg_to_check, GLOB_ERR, NULL, &gstruct);
            if (ret != 0)
            {
                if (ret == GLOB_NOMATCH)
                {
                    printf("No matches found!\n");
                    // refresh();
                }
                else
                {
                    printf("glob error\n");
                    // refresh();
                }
            }

            // store the filenames in argv
            int new_size = new_size + (int)gstruct.gl_pathc + 1;
            argv = (char **)realloc(argv, new_size);
            arg_found = gstruct.gl_pathv;
            while (*arg_found)
            {
                argv[cnt] = *arg_found;
                cnt++;
                arg_found++;
            }
        }
        else
        {
            argv[cnt] = arg_to_check;
            cnt++;
        }
    }
    argv[cnt] = NULL; // Terminate with NULL pointer

    char *const *argv1 = argv; // Assign it to a constant array

    execvp(vector_get(&args, 0), argv1); // Call the execvp command
}

pid_t fgpid;

// check for pipes, and background processes('&' at the end of string) and then execute them line by line
void runcmd(char *cmd, int *status_)
{
    int status = *status_;
    int bg = 0; // flag for background running
    // Check for background run
    cmd = rtrim(ltrim(cmd));
    if (cmd[strlen(cmd) - 1] == '&')
        bg = 1, cmd[strlen(cmd) - 1] = ' ';

    // Split into several commands wrt to |
    vector cmds;
    vector_init(&cmds);
    cmds = split(cmd, '|');

    // If no pipes are required
    if (vector_total(&cmds) == 1)
    {
        // Split the commands and redirection
        vector parsed;
        vector_init(&parsed);
        parsed = splitInputOuput(vector_get(&cmds, 0));
        // store destination string before splitting to avoid splitting paths containing space
        char *dest_dir = vector_get(&parsed, 0);
        vector temp = split(vector_get(&parsed, 0), ' ');

        if (strcmp(vector_get(&temp, 0), "cd") == 0)
        {
            if (vector_total(&temp) == 1)
                chdir(getenv("HOME"));
            else
            {
                char dirPath[strlen(dest_dir)]; // dirPath to store path directory for cd
                memset(&dirPath, '\0', sizeof(dirPath));
                int c = 0;
                for (int j = 3; j < strlen(dest_dir); j++)
                {
                    if (dest_dir[j] == '"')
                        continue;
                    // copy the char after \ as it is to dirPath
                    if (dest_dir[j] == '\\')
                        dirPath[c++] = dest_dir[++j];
                    else
                        dirPath[c++] = dest_dir[j];
                }
                dirPath[c] = '\0';
                if ((c = chdir(dirPath)) == -1)
                {
                    printf("wish: cd: %s: No such file or directory\n", dest_dir + 3);
                }
            }
            return;
        }
        else if (strcmp(vector_get(&temp, 0), "delep") == 0)
        {
            delep(vector_get(&temp, 1));
            return;
        }

        pid_t pid = fgpid = fork();
        if (pid == 0)
        {
            redirect(vector_get(&parsed, 1), vector_get(&parsed, 2));
            execCmd(vector_get(&parsed, 0));
            exit(0); // Exit the child process
        }

        if (!bg)
            wait(&status);
    }

    else
    {
        int n = vector_total(&cmds); // No. of pipe commands
        int newFD[2], oldFD[2];

        for (int i = 0; i < n; i++)
        {
            vector parsed;
            vector_init(&parsed);
            parsed = splitInputOuput(vector_get(&cmds, i));
            if (i != n - 1) // Create new pipe except for the last command
                pipe(newFD);

            pid_t pid = fgpid = fork(); // Fork for every command

            // In the child process
            if (pid == 0)
            {
                if (!i || i == n - 1)
                    redirect(vector_get(&parsed, 1), vector_get(&parsed, 2)); // For the first and last command redirect the input output files

                // Read from previous command for everything except the first command
                if (i)
                    dup2(oldFD[0], 0), close(oldFD[0]), close(oldFD[1]);

                // Write into pipe for everything except last command
                if (i != n - 1)
                    close(newFD[0]), dup2(newFD[1], 1), close(newFD[1]);

                // Execute command
                execCmd(vector_get(&parsed, 0));
            }

            // In parent process
            if (i)
                close(oldFD[0]), close(oldFD[1]);

            // Copy newFD into oldFD for everything except the last process
            if (i != n - 1)
                oldFD[0] = newFD[0], oldFD[1] = newFD[1];
        }

        // If no background, then wait for all child processes to return
        if (!bg)
            while (wait(&status) > 0)
                ;
    }
}

// THE READ FUNCTIONALITY
static char *line_read = (char *)NULL;

char *rl_gets()
{
    /* If the buffer has already been allocated, return the memory
       to the free pool. */
    if (line_read)
    {
        free(line_read);
        line_read = (char *)NULL;
    }

    /* Get a line from the user. */
    line_read = readline(PROMPT);

    if (!line_read){
        printf("[EXITING...]\n");
        saveHistory();
        exit(EXIT_SUCCESS);
    }

    /* If the line has any text in it, save it on the history. */
    if (line_read && *line_read)
        setHistory(line_read);

    return (line_read);
}

void shell_call()
{
    int status;

    runcmd(rl_gets(), &status);
}

// SIGNAL
void sigstp_hamdler(int signum)
{
    if ( signum == SIGTSTP )
        printf("        +%s\n", line_read);
    
    kill(getpid(), SIGCHLD);
}

void sigint_handler(int signum) {
    printf("        %s\n", line_read);
    kill(fgpid, SIGINT);
}

int up_arrow_function(int count, int key) {
    char* historyCmd = (char *)malloc(CMD_SIZE*sizeof(char));
    historyCmd = getHistory(historyIndex + 1);
    if(historyCmd == NULL) return 0;
    if(historyIndex == -1) strcpy(commandBackup, rl_line_buffer);
    strcpy(rl_line_buffer, historyCmd);
    rl_end = strlen(historyCmd);
    rl_redisplay();
    historyIndex++;
    return 0;
}

int down_arrow_function(int count, int key) {
    if(historyIndex == -1) return 0;
    else if(historyIndex == 0) {
        strcpy(rl_line_buffer, commandBackup);
        rl_end = strlen(commandBackup);
    }
    else {
        char* historyCmd = (char *)malloc(CMD_SIZE*sizeof(char));
        historyCmd = getHistory(historyIndex - 1);
        strcpy(rl_line_buffer, historyCmd);
        rl_end = strlen(historyCmd);
    }
    rl_redisplay();
    historyIndex--;
    return 0;
}

int main()
{
    signal(SIGTSTP, sigstp_hamdler);
    signal(SIGINT, sigint_handler);

    rl_bind_keyseq("\\e[A", up_arrow_function);
    rl_bind_keyseq("\\e[B", down_arrow_function);

    loadHistory();

    while (TRUE)
    {
        historyIndex = -1;
        shell_call();
    }

    return 0;
}
