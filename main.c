#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"  // Includes header file for process-related functions and structures
#include "queue.h"    // Includes header file for queue-related functions and structures
#include "utils.h"    // Includes header file for utility functions.

#define RAM_SIZE 2048  // Defines the size of the RAM

// Function to execute processes in FCFS (First Come First Serve) manner
void executeFCFS(Queue *q, const char *cpu_name) {
    while (!isQueueEmpty(q)) {
        Process *p = dequeue(q);  // Dequeues a process from the queue
        executeProcess(p, p->burst_time);  // Executes the process for its burst time
        printf("Process %s completed and terminated on %s\n", p->name, cpu_name);  // Prints process completion message
    }
}

// Function to execute processes in SJF (Shortest Job First) manner
void executeSJF(Queue *q, const char *cpu_name) {
    while (!isQueueEmpty(q)) {
        int shortest_index = -1;
        int shortest_burst = __INT_MAX__;  // Initializes shortest burst time to maximum integer value
        // Finds the process with the shortest remaining burst time
        for (int i = 0; i < q->size; i++) {
            int index = (q->front + i) % MAX_PROCESSES;
            if (q->processes[index]->remaining_time < shortest_burst) {
                shortest_burst = q->processes[index]->remaining_time;
                shortest_index = index;
            }
        }
        Process *p = q->processes[shortest_index];  // Retrieves the process with the shortest burst time
        // Rearranges the queue to remove the completed process
        q->processes[shortest_index] = q->processes[q->front];
        q->front = (q->front + 1) % MAX_PROCESSES;
        q->size--;
        // Executes the process for its burst time
        executeProcess(p, p->burst_time);
        printf("Process %s completed and terminated on %s\n", p->name, cpu_name);  // Prints process completion message
    }
}

// Function to execute processes in Round Robin manner with a given quantum
void executeRoundRobin(Queue *q, int quantum, const char *cpu_name) {
    while (!isQueueEmpty(q)) {
        Process *p = dequeue(q);  // Dequeues a process from the queue
        if (p->remaining_time > 0) {
            executeProcess(p, quantum);  // Executes the process for the given quantum
            if (p->remaining_time > 0) {
                enqueue(q, p);  // Enqueues the process back if it still has remaining time
            } else {
                printf("Process %s completed and terminated on %s\n", p->name, cpu_name);  // Prints process completion message
            }
        }
    }
}

// Main function
int main() {
    createInputFileIfNotExists("input.txt");  // Creates input file if it doesn't exist

    FILE *input_file = fopen("input.txt", "r");  // Opens input file for reading
    if (input_file == NULL) {
        printf("Error opening input file\n");
        return 1;
    }

    // Initializes queues for different priority levels
    Queue priority0_queue, priority1_queue, priority2_queue, priority3_queue;
    initQueue(&priority0_queue);
    initQueue(&priority1_queue);
    initQueue(&priority2_queue);
    initQueue(&priority3_queue);
    int ram_available = RAM_SIZE;  // Initializes available RAM

    // Arrays to store process names for each priority level
    char queue0_names[MAX_PROCESSES][10] = {0};
    char queue1_names[MAX_PROCESSES][10] = {0};
    char queue2_names[MAX_PROCESSES][10] = {0};
    char queue3_names[MAX_PROCESSES][10] = {0};
    int queue0_count = 0, queue1_count = 0, queue2_count = 0, queue3_count = 0;

    char line[100];
    // Reads processes from input file and queues them based on priority
    while (fgets(line, sizeof(line), input_file)) {
        Process *p = malloc(sizeof(Process));  // Allocates memory for a new process
        // Parses process information from the input line
        sscanf(line, "%[^,],%d,%d,%d,%d,%d", p->name, &p->arrival_time, &p->priority, &p->burst_time, &p->ram_required,
               &p->cpu_percentage);
        p->remaining_time = p->burst_time;

        if (p->priority == 0) {
            allocateRAM(p, &ram_available);  // Allocates RAM for priority 0 processes
        }

        // Queues the process based on its priority level
        if (p->priority == 0) {
            printf("Process %s queued to be assigned to CPU-1\n", p->name);
            enqueue(&priority0_queue, p);
            strcpy(queue0_names[queue0_count++], p->name);
        } else if (p->priority == 1) {
            printf("Process %s placed in priority1 queue to be assigned to CPU-2\n", p->name);
            enqueue(&priority1_queue, p);
            strcpy(queue1_names[queue1_count++], p->name);
        } else if (p->priority == 2) {
            printf("Process %s placed in priority2 queue to be assigned to CPU-2\n", p->name);
            enqueue(&priority2_queue, p);
            strcpy(queue2_names[queue2_count++], p->name);
        } else if (p->priority == 3) {
            printf("Process %s placed in priority3 queue to be assigned to CPU-2\n", p->name);
            enqueue(&priority3_queue, p);
            strcpy(queue3_names[queue3_count++], p->name);
        }
    }

    printf("CPU-1 queue (priority-0) (FCFS)→");
    for (int i = 0; i < queue0_count; i++) {
        printf("%s-", queue0_names[i]);
    }
    printf("\n");

    printf("CPU-2 queue (priority-1) (SJF)→");
    for (int i = 0; i < queue1_count; i++) {
        printf("%s-", queue1_names[i]);
    }
    printf("\n");

    printf("CPU-2 queue (priority-2) (RR-q8)→");
    for (int i = 0; i < queue2_count; i++) {
        printf("%s-", queue2_names[i]);
    }
    printf("\n");

    printf("CPU-2 queue (priority-3) (RR-q16)→");
    for (int i = 0; i < queue3_count; i++) {
        printf("%s-", queue3_names[i]);
    }
    printf("\n");

    // Execute processes in priority 0 queue using FCFS
    executeFCFS(&priority0_queue, "CPU-1 (priority-0)");

    // Execute processes in priority 1 queue using SJF
    executeSJF(&priority1_queue, "CPU-2 (priority-1)");

    // Execute processes in priority 2 queue using Round Robin with quantum 8
    executeRoundRobin(&priority2_queue, 8, "CPU-2 (priority-2)");

    // Execute processes in priority 3 queue using Round Robin with quantum 16
    executeRoundRobin(&priority3_queue, 16, "CPU-2 (priority-3)");

    fclose(input_file);

    return 0;
}
