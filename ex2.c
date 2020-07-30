// Yair Cohen 318571718 LATE-SUBMISSION
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <zconf.h>
#define COMMAND_SIZE 100
#define NUM_COMMANDS 100
/**
 * this struct represents a process holding the command pof the process and its process id
 */
struct Process {
    char  command_prop[COMMAND_SIZE];
    pid_t process_id;
};
/**
 * this struct represens a directory holding a process and the current dir and the previous dir.
 */
struct Directory{
    struct Process process;
    char wd[COMMAND_SIZE];// the current working directory.
    char pd[COMMAND_SIZE];// the previous working directory.
};
/**
 * this struct holds all the datat there is in the shell regarding processes.
 */
struct Shell{
    struct Process jobsArr[NUM_COMMANDS];
    struct Process historyArr[NUM_COMMANDS];
    struct Directory thisDir;

};
int indexH =0;// keeps track of the index of the current command in history array.
int indexJ =0 ; // keeps track of the index of the current command in jobs array.
struct Shell shell; // this struct holds all the data of the shell,

/**
 * this method checks if the given process is still running.
 * @param i index of the commsnd in the given array.
 * @param arr the arrary.
 * @return 1 if running 0 if done.
 */
int checkIfRunning(int i,struct Process arr[NUM_COMMANDS]){
    if (!((kill(arr[i].process_id, 0)|| (arr[i].process_id == getpid()))))
        return 1;
    else
        return 0;
}
/**
 * this method prints the records of the history records.
 */
void showHistoryRecords() {
    int i = 0;
    while (i != indexH) {
        printf("%d %s ", shell.historyArr[i].process_id, shell.historyArr[i].command_prop);
        if ((checkIfRunning(i,shell.historyArr))==1) {
            printf("RUNNING\n");
        } else {
            printf("DONE\n");
        }
        i++;
    }
    printf("%d history RUNNING\n", getpid());
    exit(0);
}
/**
 * this method prints the records of the job records.
 */
void showJobsRecords() {
    int i = 0;
    while (i != indexJ) {
        if (shell.jobsArr[i].process_id == 0) {
            continue;
        }
        if ((checkIfRunning(i,shell.jobsArr))==1) {
            printf("%d %s\n", shell.jobsArr[i].process_id, shell.jobsArr[i].command_prop);
        } else {
            shell.jobsArr[i].process_id = 0;
        }
        i++;
    }
    exit(0);
}
/**
 * This method adds the command to history records.
 * @param command the given command from main.
 * @param pid the process id
 * @return
 */
int add2History(char *command, pid_t pid){
    strcpy(shell.historyArr[indexH].command_prop, command);
    shell.historyArr[indexH].process_id = pid;
    indexH++;
}
/**
 * This method adds the command to jobs records.
 * @param command the given command from main.
 * @param pid the process id
 * @return
 */
int add2Jobs(char *command, pid_t pid) {
    strcpy(shell.jobsArr[indexJ].command_prop, command);
    shell.jobsArr[indexJ].process_id = pid;
    indexJ++;
}
 /**
  * This method appends the commands to records in history and jobs.
  * @param command - given command from main
  * @param pid - the process id
  * @return
  */
int append2Arrays(char *command, pid_t pid){
    add2History(command,pid);
    add2Jobs(command,pid);
}
/**
 * This functions executes the father process
 * @param command the given command from main
 * @param pid the process id
 * @param flag the status of the wait
 */
void executeFatherProcess(char *command, pid_t pid, int flag) {
    signal(SIGCHLD, SIG_IGN);
    if (command[strlen(command) - 1] == '&') {
        command[strlen(command) - 1] = 0;
    } else {
        waitpid(pid, &flag, 0);
    }
    append2Arrays(command, pid);
}
/**
 * This method executes a built in command using the exec commands.
 * @param command given command from main
 * @param pid the process id
 */
void executeCmd(char *command, pid_t pid) {
    printf("%d\n", getpid());
    char shell[2048] = {0};
    char *com;
    char *tmpstr;
    char *p;
    char *splitstr[COMMAND_SIZE];
    char tempcom[COMMAND_SIZE];
    strcpy(tempcom, command);
    int index = 0;
    tmpstr = strtok(tempcom, " ");
    com = tmpstr;
    while (tmpstr != NULL) {
        splitstr[index] = tmpstr;
        tmpstr = strtok(NULL, " ");
        index++;
    }
    splitstr[index] = NULL;
    if (strcmp(com, "echo") == 0) {
        if (strcmp(splitstr[index - 1], "&") == 0) {
            splitstr[index - 1] = NULL;
            index--;
        }
        if (splitstr[1][0] == '\"') {
            splitstr[1]++;
            p = splitstr[index - 1];
            p[strlen(p) - 1] = 0;
        }
    }
    if (strcmp(splitstr[index - 1], "&") == 0) {
        splitstr[index - 1] = NULL;
        index--;
    }
    system(shell);
    execvp(com, splitstr);
    fprintf(stderr, "Error - can't execute system command \n");
    exit(0);
}
/**
 * This method checks the given command and sends it to the right execute methos.
 * @param command - the given command from main.
 */
void checkInput(char *command) {
    int flag;
    char tmppath[PATH_MAX] = {0};
    char curCommand[COMMAND_SIZE];
    char *tmpstr;
    char path[PATH_MAX] = {0};
    char *splitstr[COMMAND_SIZE];
    char wd[PATH_MAX];
    int counter = 0;
    splitstr[1] = NULL;
    splitstr[2] = NULL;
    if ((command!= NULL)||(strlen(command)>0)) {
        if (strcmp(command, "exit") == 0) {// executes the exit command
            printf("%d\n", getpid());
            exit(0);
        }
        if (strstr(command, "cd")) {// executes the cd command.
            append2Arrays(command, getpid());// appends the given command into both history and jobs command
            strcpy(curCommand, command);
            tmpstr = strtok(curCommand, " ");
            while (tmpstr != NULL) {
                splitstr[counter] = tmpstr;
                tmpstr = strtok(NULL, " ");
                counter++;
            }
            if (splitstr[2] != NULL) {
                printf("%d\n", getpid());
                fprintf(stderr, "Error\n");
                return;
            }
            printf("%d\n", getpid());
            if (getcwd(wd, sizeof(wd)) == NULL) {
                fprintf(stderr, "Error can't change to given directory\n");
                return;
            }
            strcpy(shell.thisDir.wd, wd);
            shell.thisDir.wd[strlen(wd)] = 0;
            if ((splitstr[1] == NULL) || (!strcmp(splitstr[1], "~"))) {
                if (chdir(getenv("HOME")) == -1) {
                    fprintf(stderr, "Error - can't change to home directory\n");
                    return;
                } else {
                    strcpy(shell.thisDir.pd, shell.thisDir.wd);
                    shell.thisDir.pd[strlen(shell.thisDir.wd)] = 0;
                    if (getcwd(tmppath, sizeof(tmppath) == 0)) {
                        fprintf(stderr, "Error changing Directory\n");
                    }
                    strcpy(shell.thisDir.wd, tmppath);
                    shell.thisDir.wd[strlen(tmppath)] = 0;
                    return;
                }
            }
            else if (!strcmp(splitstr[1], "..")) {
                    if (chdir("..") == -1) {
                    fprintf(stderr, "Error - can't return to parent dir\n");
                    return;
                }
                    else {
                        strcpy(shell.thisDir.pd, shell.thisDir.wd);
                        shell.thisDir.pd[strlen(shell.thisDir.wd)] = 0;
                        if (getcwd(tmppath, sizeof(tmppath) == 0)) {
                            fprintf(stderr, "Error changing Directory\n");
                        }
                        strcpy(shell.thisDir.wd, tmppath);
                        shell.thisDir.wd[strlen(tmppath)] = 0;
                        return;
                    }
            }
                else if (!strcmp(splitstr[1], "-")) {
                    if (!strcmp(shell.thisDir.pd, "")) {
                        return;
                } else {
                    if (chdir(shell.thisDir.pd) == -1) {
                        fprintf(stderr, "Error - can't return to previous dir\n");
                        return;
                    } else {
                        strcpy(shell.thisDir.pd, shell.thisDir.wd);
                        shell.thisDir.pd[strlen(shell.thisDir.wd)] = 0;
                        if (getcwd(tmppath, sizeof(tmppath) == 0)) {
                            fprintf(stderr, "Error changing Directory\n");
                        }
                        strcpy(shell.thisDir.wd, tmppath);
                        shell.thisDir.wd[strlen(tmppath)] = 0;
                        return;
                    }
                }

            } else {
                if (splitstr[1][0] == '~') {
                    strcpy(path, getenv("HOME"));
                    strcat(path, splitstr[1]);
                }
                else if (splitstr[1][0] == '-') {
                    splitstr[1]++;
                    strcpy(path, shell.thisDir.pd);
                    strcat(path, splitstr[1]);
                } else {
                    strcpy(path, splitstr[1]);
                }
                struct stat st1, st2;
                stat(path, &st1);
                stat(path, &st2);
                if (S_ISDIR(st2.st_mode)) {
                    if (chdir(path) == -1) {
                        fprintf(stderr, "Error - can't load dir\n");
                        return;
                    } else {
                        strcpy(shell.thisDir.pd, shell.thisDir.wd);
                        shell.thisDir.pd[strlen(shell.thisDir.wd)] = 0;
                        if (getcwd(tmppath, sizeof(tmppath) == 0)) {
                            fprintf(stderr, "Error changing Directory\n");
                        }
                        strcpy(shell.thisDir.wd, tmppath);
                        shell.thisDir.wd[strlen(tmppath)] = 0;
                        return;
                    }
                }
                 if(st1.st_mode & F_OK){
                     if (chdir(path) == -1) {
                         fprintf(stderr, "Error - can't load dir\n");
                         return;
                     } else {
                         strcpy(shell.thisDir.pd, shell.thisDir.wd);
                         shell.thisDir.pd[strlen(shell.thisDir.wd)] = 0;
                         if (getcwd(tmppath, sizeof(tmppath) == 0)) {
                             fprintf(stderr, "Error changing Directory\n");
                         }
                         strcpy(shell.thisDir.wd, tmppath);
                         shell.thisDir.wd[strlen(tmppath)] = 0;
                         return;
                     }
                 }
                 else {
                    fprintf(stderr, "Error - Can't find dir\n");
                }
            }
            return;
        }
        pid_t pid;
        if ((pid= fork()) == -1)// it means the fork had failed.
            fprintf(stderr, "Error - can't execute command \n");
        else if (pid == 0) {// checks what kind of command it is.
            if (!strcmp(command, "jobs"))// executes command jobs
                showJobsRecords();
            else if (!strcmp(command, "history"))// executes command history
                showHistoryRecords();
            else// executes a built in command .
                executeCmd(command,pid);
        }
        else// executes the father command.
            executeFatherProcess(command,pid, flag);
    }
}
/**
 * this is the main function that operated as the command line.
*/
int main() {
    char command[COMMAND_SIZE];// this is the array that holds the input command
    int i;
    for (i = 0; i < NUM_COMMANDS; i++) {// this loop keeps getting new commands
        printf("> ");
        fgets(command, COMMAND_SIZE, stdin);
        if(command!=NULL) {
            command[strcspn(command, "\n")] = 0;// removes the /n from the command.
            checkInput(command);// checks the input weather command whether its a job cd or anything else
        }
    }
    return 1;
}
