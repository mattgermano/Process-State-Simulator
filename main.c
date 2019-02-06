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

int main()
{
    FILE* fp = fopen("inp1.txt", "r"); /* Open the file for reading */
    
    char buff[MAX_LINE_LENGTH];
    char* token;
    
    process_t process[20]; /* Instantiate an array of structs to hold each processes state information */

    printf("Simulation Begins\n");
    printf("Initial State\n");
    fgets(buff, sizeof(buff), (FILE*)fp);  /* Place the process line into the buffer */
    printf("%s", buff);
    token = strtok(buff, " ");             /* Place the first process identifier into the token field */

    int process_count = 0;
    while (token != NULL)
    {
        strcpy(process[process_count].id, token);
        token = strtok(NULL, " ");
        strcpy(process[process_count].state, token);
        token = strtok(NULL, " ");
        process[process_count].updated = false;

        process_count++;
    }

    while (fgets(buff, sizeof(buff), (FILE*)fp))
    {
        printf("\n\n");
        printf("%s", buff);
        strtok(buff, ":");
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
            token = strtok(NULL, ";");
        }

        /* Print the updated states */
        for (int i = 0; i < process_count; i++)
        {
            printf("%s ", process[i].id); /* Print the Process ID */
            if (process[i].updated)
            {
                printf("%s* ", process[i].state); /* Print the state with an asterick if it was updated */
                process[i].updated = false;       /* Update back to false */
            }
            else
            {
                printf("%s ", process[i].state); /* Print the state normally if it was not updated */
            }
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