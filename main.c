#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE_LENGTH   255
#define MAX_PROCESS_COUNT 20

/* Struct to store information about each process */ 
typedef struct process
{
    char id[4];      /* Process ID */
    char state[30];  /* Process State */
    bool updated;    /* If the process was updated in the last instruction */
} process_t;

void parse_instruction(char*, process_t*);

int main() 
{
    FILE *fp = fopen("inp3.txt", "r"); /* Open the file for reading */

    char buff[MAX_LINE_LENGTH];
    char *token;

    process_t process[MAX_PROCESS_COUNT]; /* Instantiate an array of structs for each process */

    /* Get the first line of the file and print it out to the screen */
    fgets(buff, sizeof(buff), fp);
    buff[strcspn(buff, "\r\n")] = 0;
    printf("Simulation Begins\n");
    printf("Initial State\n");
    printf("%s", buff);

    token = strtok(buff, " "); /* Tokenize the first line on each space */
    int process_count = 0;     /* Loop through each token and store the process ID and initial state into the struct array */
    while (token != NULL) {
        strcpy(process[process_count].id, token);
        token = strtok(NULL, " ");
        strcpy(process[process_count].state, token);
        token = strtok(NULL, " ");
        process[process_count].updated = false;

        process_count++;
    }

    /* Loop through the remaining lines of the input file */
    while (fgets(buff, sizeof(buff), fp))
    {
        buff[strcspn(buff, "\r\n")] = 0;
        printf("\n\n%s\n", buff);
        token = strtok(buff, ":");  /* First token is the time information */
        token = strtok(NULL, ";."); /* Next token is the instruction */

        while (token != NULL)
        {
            char process_id[4];
            while(isspace((unsigned char)*token)) token++; /* Remove the leading spaces from the instruction */
            if (token[0] == 'P') /* If the process ID is at the beginning */
            {
                sscanf(token, "%s", process_id);
            }
            else /* Else if the process ID is at the end */
            {
                sscanf(token, "%*s %*s %*s %*s %s", process_id);
            }

            for (int index = 0; index < process_count; index++)
            {
                if (strstr(process[index].id, process_id)) /* Find the struct that matches the process ID and parse the instruction */
                {
                    parse_instruction(token, &process[index]);
                    break;
                }
            }
            token = strtok(NULL, ";."); /* Get the next instruction */
        }

        /* Check if all processes are in the blocked/new state */
        int blocked_new = 0;
        for (int i = 0; i < process_count; i++)
        {
            if (strstr(process[i].state, "Blocked") || strstr(process[i].state, "New"))
            {
                blocked_new++;
            }
        }

        /* Print the Updated States */
        for (int i = 0; i < process_count; i++)
        {
            printf("%s ", process[i].id);
            if (process[i].updated)
            {
                printf("%s*", process[i].state); /* Print the state with an asterick if it was updated */
                if (blocked_new != process_count) process[i].updated = false;
            }
            else
            {
                printf("%s", process[i].state); /* Print the state normally if it was not updated */
            }
            if (i < process_count - 1) printf(" ");
        }

        /* Swap the newest process that was blocked and replace with a new process if everything is blocked/new */
        if (blocked_new == process_count)
        {
            printf("\n\nAll processes are in the blocked/new state: ");
            for (int i = 0; i < process_count; i++)
            {
                if (process[i].updated)
                {
                    process[i].updated = false;       /* Update back to false */
                    parse_instruction("swapped out", &process[i]); /* Swap out the process */
                    printf("%s is swapped out to %s; ", process[i].id, process[i].state);
                    for (int j = 0; j < process_count; j++)
                    {
                        if (strstr(process[j].state, "New"))
                        {
                            parse_instruction("admit", &process[j]); /* Swap in a new process */
                            printf("%s is admitted to the %s state.", process[j].id, process[j].state);
                            break;
                        }
                    }
                    break;
                }
            }
        }

        /* If a process exits, swap a new process in */
        int priority = 0;
        for (int i = 0; i < process_count; i++)
        {
            if (strstr(process[i].state, "Exit")) /* If a process exits */
            {
                for (int j = 0; j < process_count; j++)
                {
                    if (strstr(process[j].state, "Ready/Suspend")) /* If there are no new processes, swap in a Ready/Suspend process */
                    {
                        priority = 1;
                        parse_instruction("swapped in", &process[j]);
                        printf("\n\n%s terminated: %s swapped in to the %s state", 
                                process[i].id, process[j].id, process[j].state);
                    }
                }
                if (priority == 0)
                {
                    for (int j = 0; j < process_count; j++)
                    {
                        if (strstr(process[j].state, "New")) /* Find a new process */
                        {
                            parse_instruction("admit", &process[j]); /* Swap it in */
                            printf("\n\n%s terminated: %s swapped in to the %s state", 
                                    process[i].id, process[j].id, process[j].state);
                        }
                    }
                }
            }
        }
    }
    fclose(fp); /* Close the file */
    return 0;
}

void parse_instruction(char* instruction, process_t* process)
{
    process->updated = true;

    /* If/else if state machine to update the process state based on the instruction */
    if (strstr(instruction, "requests"))
    {
        strcpy(process->state, "Blocked");
    }
    else if (strstr(instruction, "dispatched"))
    {
        strcpy(process->state, "Running");
    }
    else if (strstr(instruction, "slice"))
    {
        strcpy(process->state, "Ready");
    }
    else if (strstr(instruction, "swapped out"))
    {
        if (strstr(process->state, "Blocked"))
        {
            strcpy(process->state, "Blocked/Suspend");
        }
        else if (strstr(process->state, "Ready"))
        {
            strcpy(process->state, "Ready/Suspend");
        }
    }
    else if (strstr(instruction, "swapped in"))
    {
        if (strstr(process->state, "Blocked/Suspend"))
        {
            strcpy(process->state, "Blocked");
        }
        else if (strstr(process->state, "Ready/Suspend"))
        {
            strcpy(process->state, "Ready");
        }

    }
    else if (strstr(instruction, "terminated"))
    {
        strcpy(process->state, "Exit");
    }
    else if (strstr(instruction, "interrupt"))
    {
        if (strstr(process->state, "Blocked/Suspend"))
        {
            strcpy(process->state, "Ready/Suspend");
        }
        else if (strstr(process->state, "Blocked"))
        {
            strcpy(process->state, "Ready");
        }
    }
    else if (strstr(instruction, "admit"))
    {
        strcpy(process->state, "Ready");
    }
}