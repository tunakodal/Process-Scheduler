#ifndef PROCESS_SCHEDULER_H
#define PROCESS_SCHEDULER_H

#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "unistd.h"
#include "string.h"

typedef struct Process {
    int pid;            
    int arrival_time;
    int cpu_execution_time;
    int interval_time;
    int io_time;
    int priority;
    int aging_counter; // The time that the process has entered the ready queue for aging purposes
    int io_finish_time; 
    int cpu_entered_time;
} Process;

#define MAX_PROCESS_NUM 20
#define AGING_LIMIT 100

#endif // PROCESS_SCHEDULER_H