#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "util.c"
#include <fcntl.h>
#include<unistd.h>
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

// Execute the commands
void execCmd(char* cmd)
{
    // Split the command and its arguments
    vector args;
    vector_init(&args);

    vector te;
    vector_init(&te);
    te = split(cmd,' ');
    for(int i = 0; i < vector_total(&te); i++){
        if(strlen(vector_get(&te,i)))
            vector_add(&args, vector_get(&te,i));
    }


    // Create a char* array for the arguments
    char* argv[vector_total(&args)+1];

    for(int i=0 ; i<vector_total(&args) ; i++)
        argv[i] = vector_get(&args,i); // Convert string to char *
    argv[vector_total(&args)] = NULL; // Terminate with NULL pointer

    char* const* argv1 = argv; // Assign it to a constant array
    execvp(vector_get(&args,0),argv1); // Call the execvp command
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
