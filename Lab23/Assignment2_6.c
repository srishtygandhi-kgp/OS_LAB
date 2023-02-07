#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "utils.c"
#include <fcntl.h>
#include<unistd.h>
#include <glob.h>
#include<sys/wait.h>

char* ltrim(char* s) {
    size_t len = strlen(s);
    size_t i = 0;
    while (i < len && isspace(s[i])) {
        ++i;
    }
    memmove(s, s + i, len - i + 1);
    return s;
}

char * rtrim(char * s) {
    int len = strlen(s);
    while (len > 0 && isspace(s[len - 1])) {
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
vector split(char* cmd, char delim)
{
    int count = 0;
    const char* tmp = cmd;
    while (*tmp) {
        if (*tmp == delim) {
            count++;
        }
        tmp++;
    }
    count++;

    vector res;
    vector_init(&res);
    int i = 0;
    const char* start = cmd;
    tmp = cmd;
    while (*tmp) {
        if (*tmp == delim) {
            int len = tmp - start;
            char* result = (char*)malloc(len + 1);
            if (!result) {
                printf("Error allocating memory\n");
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
    char* result = (char*)malloc(len + 1);
    if (!result) {
        printf("Error allocating memory\n");
        exit(1);
    }
    memcpy(result, start, len);
    result[len] = '\0';
    vector_add(&res, result);

    return res;
}


// Splits the command into input and ouput
vector splitInputOuput(char* cmd)
{
    vector res;
    vector_init(&res);
    vector_add(&res,"");
    vector_add(&res,"");
    vector_add(&res,"");

    vector output;
    vector_init(&output);
    output = split(cmd,'>');
    
    // No output redirection
    if( vector_total(&output) == 1)
    {
        vector input;
        vector_init(&input);
        input = split(cmd,'<');
        
        // No input, output redirection
        if( vector_total(&input) == 1) 
        {
            
            vector_set(&res,0,rtrim(ltrim(vector_get(&output,0))));
            return res;
        }

        // Input redirection present, no output redirection
        else
        {
            // Trim whitespaces
            vector_set(&res,1,rtrim(ltrim(vector_get(&input,1))));
            vector_set(&res,0,rtrim(ltrim(vector_get(&input,0))));
            return res;
        }   
    }

    vector input;
    vector_init(&input);
    input = split(cmd,'<');

    // No input redirection, output redirection present
    if(vector_total(&input) == 1)
    {
        // Trim whitespaces
        vector_set(&res,2,rtrim(ltrim(vector_get(&output,1))));
        vector_set(&res,0,rtrim(ltrim(vector_get(&output,0))));
        return res;
    }

    // Input, output redirection present
    
    // Input redirection present in the second arg of the output
    vector vec_tmp;
    vector_init(&vec_tmp);
    vec_tmp = split(vector_get(&output,0),'<');
    if(vector_total(&vec_tmp ) == 1)
    {
        // Split the second argument into output and input respectively
        vector output_input;
        vector_init(&output_input);
        output_input = split(vector_get(&output,1),'<');

        // Trim the whitespaces in out and in
        vector_set(&res,2,rtrim(ltrim(vector_get(&output_input,0))));
        vector_set(&res,1,rtrim(ltrim(vector_get(&output_input,1))));
        vector_set(&res,0,rtrim(ltrim(vector_get(&output,0))));
        return res;
    }

    // Input redirection present in the first arg of the output

    // Split the first argument into commmand and input respectively
    vector cmd_input;
    vector_init(&cmd_input);
    cmd_input = split(vector_get(&output,0),'<');
    
    // Trim the whitespaces in out and in
    vector_set(&res,1,rtrim(ltrim(vector_get(&cmd_input,1))));
    vector_set(&res,2,rtrim(ltrim(vector_get(&output,1))));
    vector_set(&res,0,rtrim(ltrim(vector_get(&cmd_input,0))));

    return res;    
} 

// Open files and redirect input and output with files as arguments
void redirect(char * inp, char * out)
{
    int inp_fd, out_fd;

    // Open input redirecting file
    if(strlen(inp))
    {
        inp_fd = open(inp,O_RDONLY);  // Open in read only mode
        if(inp_fd < 0)
        {
            printf("Error opening input file\n");
            exit(EXIT_FAILURE);
        }
        // Redirect input
        if( dup2(inp_fd,0) < 0 )
        {
            printf("Input redirecting error");
            exit(EXIT_FAILURE);
        }
    }

    // Open output redirecting file
    if(strlen(out))
    {
        out_fd = open(out, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);  // Open in create and truncate mode
        // Redirect output
        if( dup2(out_fd,1) < 0 )
        {
            printf("Output redirecting error\n");
            exit(EXIT_FAILURE);
        }
    }
}

int get_parent_pid(int pid) {
  char proc_path[1024];
  snprintf(proc_path, 1024, "/proc/%d/status", pid);
  FILE *f = fopen(proc_path, "r");
  if (f == NULL) {
      printf("-1, /proc/%d/status does not exist\n", pid);
    return -1;
  }
  int ppid = -1;
  char line[1024];
  while (fgets(line, 1024, f) != NULL) {
    if (strncmp(line, "PPid:", 5) == 0) {
      ppid = atoi(line + 5);
      break;
    }
  }
  fclose(f);
//   printf("%d", ppid);
  return ppid;
}

void read_stat_file(int pid){
    char proc_path[1024];
    snprintf(proc_path, 1024, "/proc/%d/stat", pid);
    FILE *f = fopen(proc_path, "r");
    if (f == NULL) {
        printf("-1, /proc/%d/stat does not exist\n", pid);
        return ;
    }
    int ppid = -1;
    char line[2048];
    vector fields;
    vector_init(&fields);
    if(fgets(line, 2048, f) != NULL) {
       fields = split(line, ' '); 
    }

    printf("ppid -- %s", (char*)vector_get(&fields, 22));
    for(int i = 0; i < vector_total(&fields); i++){
        printf("%s ", (char*)vector_get(&fields,i));
    }
    fclose(f);
}

void getparent(int pid) {
    int parent_pid = get_parent_pid(pid);
    printf("Process ID: %d, Parent Process ID: %d\n", pid, parent_pid);
    // get_process_info(pid);
    read_stat_file(pid);
    printf("\n");
    if (parent_pid != 1) {
        getparent(parent_pid);
    }
}


void squash_bug(char* cmd){
    // syntax -- sb then number then flag
    int given_process_id = 0;
    int cnt = 3;
    int len = strlen(cmd);
    while (cnt<len){
        if(cmd[cnt] >= '0' && cmd[cnt] <= '9'){
            given_process_id *= 10;
            given_process_id += (int)(cmd[cnt] - '0');
        }
        else{
            break;
        }
        cnt++;
    }

    printf("%d\n",given_process_id );
    getparent(given_process_id);
    
}


// Execute the commands
void execCmd(char *cmd)
{
    // check fot the sb in front 
    if(cmd[0] == 's' && cmd[1] =='b' && cmd[2] == ' '){
        printf("you ran sb\n");
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
        int flag = 0; // flag to check if argument contains a wildcard char
        for (int j = 0; j < strlen(arg_to_check); j++)
        {
            if(arg_to_check[j] == '"'){
                while(j < strlen(arg_to_check) && arg_to_check[j] != '"')
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
                    printf("No matches found!\n");
                else
                    printf("glob error\n");
            }

            // store the filenames in argv
            int new_size = new_size + (int)gstruct.gl_pathc +1;
            argv = realloc(argv, new_size);
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

    char *const *argv1 = argv;           // Assign it to a constant array
    execvp(vector_get(&args, 0), argv1); // Call the execvp command
}

// check for pipes, and background processes('&' at the end of string) and then execute them line by line
void runcmd(char* cmd, int* status_){
    int status = *status_;
    int bg = 0; // flag for background running
    // Check for background run
    cmd = rtrim(ltrim(cmd));
    if( cmd[strlen(cmd)-1] == '&')
        bg = 1, cmd[strlen(cmd) -1] = ' ';

        // Split into several commands wrt to |
    vector cmds;
    vector_init(&cmds);
    cmds = split(cmd, '|');

    // If no pipes are required
    if(vector_total(&cmds)==1)
    {
        // Split the commands and redirection
        vector parsed;
        vector_init(&parsed);
        parsed = splitInputOuput(vector_get(&cmds,0));
        // store destination string before splitting to avoid splitting paths containing space
        char *dest_dir = vector_get(&parsed,0); 
        vector temp = split(vector_get(&parsed,0), ' ');

        if(strcmp(vector_get(&temp,0), "cd") == 0) {
            if(vector_total(&temp)==1)
                chdir(getenv("HOME"));
            else {
                char dirPath[strlen(dest_dir)];  // dirPath to store path directory for cd
                memset(&dirPath, '\0', sizeof(dirPath));
                int c = 0;
                for (int j = 3; j < strlen(dest_dir); j++)
                {   
                    if(dest_dir[j] == '"') continue;
                    // copy the char after \ as it is to dirPath
                    if(dest_dir[j] == '\\')
                      dirPath[c++]=dest_dir[++j];
                    else dirPath[c++] = dest_dir[j];
                }
                dirPath[c]='\0';
                chdir(dirPath);
            }
            return;
        }
            
        pid_t pid = fork();
        if(pid == 0)
        {
            redirect(vector_get(&parsed,1),vector_get(&parsed,2));
            execCmd(vector_get(&parsed,0));
            exit(0); // Exit the child process
        }

        if(!bg)
            wait(&status);
    }

    else
    {
        int n=vector_total(&cmds); // No. of pipe commands
        int newFD[2], oldFD[2];

        for(int i=0; i<n; i++)
        {
            vector parsed;
            vector_init(&parsed);
            parsed = splitInputOuput(vector_get(&cmds,i));
            if(i!=n-1)                   // Create new pipe except for the last command
                pipe(newFD);
                
            pid_t pid = fork();          // Fork for every command

            // In the child process
            if(pid == 0)
            {
                if( !i || i==n-1)
                    redirect(vector_get(&parsed,1), vector_get(&parsed,2));  // For the first and last command redirect the input output files

                // Read from previous command for everything except the first command
                if(i)
                    dup2(oldFD[0],0), close(oldFD[0]), close(oldFD[1]);

                // Write into pipe for everything except last command
                if(i!=n-1)
                    close(newFD[0]), dup2(newFD[1],1), close(newFD[1]);

                // Execute command
                execCmd(vector_get(&parsed,0));
            }

            // In parent process
            if(i)
                close(oldFD[0]), close(oldFD[1]);
                
            // Copy newFD into oldFD for everything except the last process
            if(i!=n-1)
                oldFD[0] = newFD[0], oldFD[1] = newFD[1];
        }

        // If no background, then wait for all child processes to return
        if(!bg)
            while( wait(&status) > 0);
    }
}


int main()
{
    
    char inp[200];
    char* cmd = (char*)malloc(200);
    int status = 0;

    while(1)
    {

        // Get input command
        printf("COMMAND> ");
        // get the entire line
        fgets(inp,200,stdin);
        // printf("%s",inp);
        // printf("%d",strlen(inp));
        strcpy(cmd,inp);

        runcmd(cmd,&status);
    }
}
