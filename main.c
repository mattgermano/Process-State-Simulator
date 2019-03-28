#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE_LENGTH   1024
#define MAX_PROCESS_COUNT 20
#define LATENCY_PENALTY   10
#define BUS_PENALTY       5

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
    FILE *fp = fopen("test_experiment2.txt", "r"); /* Open the file for reading */

    char buff[MAX_LINE_LENGTH];
    char *token;

    process_t process[MAX_PROCESS_COUNT]; /* Instantiate an array of structs for each process */

    int percentage, swap_count;
    int overhead_latency = 0;
    int bus_latency = 0;

    do
    {
        printf("Enter the percentage of processes in the Blocked state at which processes are swapped out (80, 90, 100): ");
        scanf("%d", &percentage);
        if (percentage != 80 && percentage != 90 && percentage != 100) printf("Error: Invalid percentage\n");
    } while (percentage != 80 && percentage != 90 && percentage != 100);

    do
    {
        printf("Enter the number of processes that should be swapped out (1 or 2): ");
        scanf("%d", &swap_count);
        if (swap_count < 1 || swap_count > 2) printf("Error: Invalid swap count\n");
    } while (swap_count < 1 || swap_count > 2);

    /* Get the first line of the file and print it out to the screen */
    fgets(buff, sizeof(buff), fp);
    buff[strcspn(buff, "\r\n")] = 0;
    printf("\nSimulation Begins\n");
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

        int blocked = 0, main_memory_count = 0;
        /* Check how many processes are in the blocked state and in main memory*/
        for (int i = 0; i < process_count; i++)
        {
            if (strstr(process[i].state, "Blocked") && !strstr(process[i].state, "Blocked/Suspend"))
            {
                blocked++;
                main_memory_count++;
            }
            else if ((strstr(process[i].state, "Ready") && !strstr(process[i].state, "Ready/Suspend")) 
                 || strstr(process[i].state, "Running"))
            {
                main_memory_count++;
            }
        }

        /* Print the Updated States */
        for (int i = 0; i < process_count; i++)
        {
            printf("%s ", process[i].id);
            if (process[i].updated)
            {
                printf("%s*", process[i].state); /* Print the state with an asterick if it was updated */
                if (!strstr(process[i].state, "Exit")) process[i].updated = false;
            }
            else
            {
                printf("%s", process[i].state); /* Print the state normally if it was not updated */
            }
            if (i < process_count - 1) printf(" ");
        }

        /* Swap processes out if the number of blocked process percentage is greater than or equal to the user input */
        char* name = (swap_count > 1) ? "processes" : "process";
        int priority = 0;
        if (((float)blocked / (float)main_memory_count) * 100 >= percentage)
        {
            overhead_latency += LATENCY_PENALTY;
            printf("\n\nNumber of blocked processes has reached the specified threshold of %d%%...Swapping out %d %s\n", 
                    percentage, swap_count, name);
            for (int swap = 0; swap < swap_count; swap++)
            {
                if (swap_count == 2) bus_latency += BUS_PENALTY;
                priority = 0;
                for (int i = 0; i < process_count; i++)
                {
                    if (strstr(process[i].state, "Blocked") && !strstr(process[i].state, "Blocked/Suspend"))
                    {
                        parse_instruction("swapped out", &process[i]); /* Swap out the process */
                        printf("%s is swapped out to %s; ", process[i].id, process[i].state);
                        for (int j = 0; j < process_count; j++)
                        {
                            if (strstr(process[j].state, "Ready/Suspend")) /* If there are no new processes, swap in a Ready/Suspend process */
                            {
                                priority = 1;
                                parse_instruction("swapped in", &process[j]);
                                printf("%s swapped in to the %s state. ", process[j].id, process[j].state);
                                break;
                            }
                        }
                        if (priority == 0)
                        {
                            for (int j = 0; j < process_count; j++)
                            {
                                if (strstr(process[j].state, "New")) /* Find a new process */
                                {
                                    parse_instruction("admit", &process[j]); /* Swap it in */
                                    printf("%s swapped in to the %s state. ", process[j].id, process[j].state);
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }

        /* If a process exits, swap a new process in */
        priority = 0;
        for (int i = 0; i < process_count; i++)
        {
            if (strstr(process[i].state, "Exit") && process[i].updated == true) /* If a process exits */
            {
                process[i].updated = false;
                for (int j = 0; j < process_count; j++)
                {
                    if (strstr(process[j].state, "Ready/Suspend")) /* If there are no new processes, swap in a Ready/Suspend process */
                    {
                        priority = 1;
                        parse_instruction("swapped in", &process[j]);
                        printf("\n\n%s terminated: %s swapped in to the %s state", 
                                process[i].id, process[j].id, process[j].state);
                        break;
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
                            break;
                        }
                    }
                }
                overhead_latency += LATENCY_PENALTY;
            }
        }
    }

    printf("\n\nThe overall overhead latency is %d ms", overhead_latency + bus_latency);

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