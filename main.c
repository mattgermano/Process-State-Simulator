#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 255

typedef struct process
{
    char id[4];      /* Process ID */
    char state[30];  /* Process State */
    bool updated;    /* If the process was updated in the last instruction */
} process_t;

void parse_instruction(char*, process_t*);

int main() {
    FILE *fp = fopen("inp2.txt", "r");

    char buff[MAX_LINE_LENGTH];
    char *token;

    process_t process[20];

    fgets(buff, sizeof(buff), fp);
    buff[strcspn(buff, "\r\n")] = 0;
    printf("Simulation Begins\n");
    printf("Initial State\n");
    printf("%s", buff);

    token = strtok(buff, " ");
    int process_count = 0;
    while (token != NULL) {
        strcpy(process[process_count].id, token);
        token = strtok(NULL, " ");
        strcpy(process[process_count].state, token);
        token = strtok(NULL, " ");
        process[process_count].updated = false;

        process_count++;
    }

    while (fgets(buff, sizeof(buff), fp))
    {
        buff[strcspn(buff, "\r\n")] = 0;
        printf("\n\n%s\n", buff);
        token = strtok(buff, ":");
        token = strtok(NULL, ";.");

        while (token != NULL)
        {
            char process_id[4];
            while(isspace((unsigned char)*token)) token++;
            if (token[0] == 'P')
            {
                sscanf(token, "%s", process_id);
            }
            else
            {
                sscanf(token, "%*s %*s %*s %*s %s", process_id);
            }

            for (int index = 0; index < process_count; index++)
            {
                if (strstr(process[index].id, process_id))
                {
                    parse_instruction(token, &process[index]);
                    break;
                }
            }
            token = strtok(NULL, ";.");
        }

        for (int i = 0; i < process_count; i++)
        {
            printf("%s ", process[i].id);
            if (process[i].updated)
            {
                printf("%s*", process[i].state); /* Print the state with an asterick if it was updated */
                process[i].updated = false;       /* Update back to false */
            }
            else
            {
                printf("%s", process[i].state); /* Print the state normally if it was not updated */
            }
            if (i < process_count - 1) printf(" ");
        }
    }
    fclose(fp);
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
}