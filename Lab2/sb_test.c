#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>

/*
* THIS IS BAD
*/

#define MALWARE_LOG printf
#define CHILDREN_NUM 5
#define GRANDCHILDREN_NUM 10

void grandchildren_logic() { while(1); }

void children_logic() { while(1); }

void create_grandchildren_and_loop() {

    for (int i = 0; i < GRANDCHILDREN_NUM; i ++) {

        // forking
        int gch_pid = fork();

        if ( gch_pid == 0 )
            //grandchildren just loop
            grandchildren_logic();

        MALWARE_LOG("[MALWARE LOG] GRANDCHILD_PID: %d\n", gch_pid);
    }

    // children go ahead and loop
    children_logic();

}

void create_children() {

    for (int i = 0; i < CHILDREN_NUM; i ++) {

        // forking
        int ch_pid = fork();

        // forking the 5 children
        if ( ch_pid == 0 )

            // this will create 10 grandchildren and then loop
            create_grandchildren_and_loop();          

        MALWARE_LOG("[MALWARE LOG] CHILD_PID: %d\n", ch_pid);  

    }

}

int main () {
    
    while (1) {
        create_children();
        sleep(5);
    }
    return 0;
}
