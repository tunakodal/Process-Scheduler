#ifndef PROCESS_SCHEDULER_H
#define PROCESS_SCHEDULER_H

#include "stdio.h"
#include "stdlib.h"

typedef struct Process {
    int pid;            
    int arrival_time;
    int cpu_execution_time;
    int interval_time;
    int io_time;
    int priority;
    int aging_counter; // The time that the process has entered the ready queue for aging purposes
    int io_finish_time; 
} Process;

#define MAX_PROCESS_NUM 20
#define AGING_LIMIT 100

int compare_by_arrival_time(const void *a, const void *b);

void ready_enqueue(Process *process);
Process* ready_dequeue();

void wait_enqueue(Process *process);
void wait_dequeue();

void wait_to_ready_enqueue(Process *process);
void wait_to_ready_dequeue();

#endif // PROCESS_SCHEDULER_H