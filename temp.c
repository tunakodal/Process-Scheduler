#include "process_scheduler.h"

int clock_time = 0;
int stop_flag = 0;

int ready_queue_size = 0;
Process *ready_queue[MAX_PROCESS_NUM];

int wait_queue_size = 0;
Process *wait_queue[MAX_PROCESS_NUM];

int wait_to_ready_size = 0;
Process *wait_to_ready[MAX_PROCESS_NUM]; // Temporary queue for processes moving from wait to ready

int process_count = 0;
Process all_processes[MAX_PROCESS_NUM];

Process *running_process = NULL;

pthread_mutex_t scheduler_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

// Comparison function to sort processes by arrival time
int compare_by_arrival_time(const void *a, const void *b) {
    Process *process_a = (Process *)a;
    Process *process_b = (Process *)b;
    return process_a->arrival_time - process_b->arrival_time;
}

// Adding to ready queue
void ready_enqueue(Process *process) {
    printf("[Clock: %d] PID %d moved to READY queue\n", clock_time, process->pid);
    process->aging_counter = clock_time;
    ready_queue[ready_queue_size++] = process;
}

// Removing from ready queue based on priority and remaining CPU time
Process* ready_dequeue() {
    int MIN_PRIORITY = 11;
    for (int i = 0; i < ready_queue_size; i++) {
        if (ready_queue[i]->priority < MIN_PRIORITY)
        {
            MIN_PRIORITY = ready_queue[i]->priority;
        }
    }

    int MIN_REMAINING_TIME = 1000000;
    Process* selected_process = NULL;
    int seleceted_index = -1;
    for (int i = 0; i < ready_queue_size; i++) {
        if (ready_queue[i]->priority == MIN_PRIORITY)
        {
            if (ready_queue[i]->cpu_execution_time < MIN_REMAINING_TIME)
            {
                MIN_REMAINING_TIME = ready_queue[i]->cpu_execution_time;
                selected_process = ready_queue[i];
                seleceted_index = i;
            }
        }
    }
    ready_queue[seleceted_index] = NULL;
    ready_queue_size--;
    null_deletor(ready_queue);

    if (selected_process)
        printf("[Clock: %d] Scheduler dispatched PID %d (Pr: %d, Rm: %d) for %d ms burst\n",
               clock_time,
               selected_process->pid,
               selected_process->priority,
               selected_process->cpu_execution_time,
               selected_process->interval_time);

    return selected_process;
} 

// Adding to wait queue
void wait_enqueue(Process *process) {
    printf("[Clock: %d] PID %d blocked for I/O for %d ms\n",
           clock_time, process->pid, process->io_time);
    
    process->io_finish_time = clock_time + process->io_time;
    wait_queue[wait_queue_size++] = process;
}

// Removing from wait queue when I/O is complete
void wait_dequeue() {
    int null_count = 0;
    for (int i = 0; i < wait_queue_size; i++) {
        if (wait_queue[i]->io_finish_time == clock_time)
        {
            printf("[Clock: %d] PID %d finished I/O\n", clock_time, wait_queue[i]->pid);

            wait_queue[i]->io_finish_time = -1; 
            wait_to_ready_enqueue(wait_queue[i]);
            wait_queue[i] = NULL;
            null_count++;
        }
    }
    null_deletor(wait_queue);
    wait_queue_size -= null_count;
} 

// Helper function to remove NULL entries from a queue -- based on the "dequeue" techniques of the methods above
void null_deletor(Process **queue) {
    int write_index = 0;
    for (int read_index = 0; read_index < MAX_PROCESS_NUM; read_index++) {
        if (queue[read_index] != NULL) {
            queue[write_index++] = queue[read_index];
        }
    }

    while (write_index < MAX_PROCESS_NUM) {
        queue[write_index++] = NULL;
    }
}

// Moving processes from "wait-to-ready" queue
void wait_to_ready_enqueue(Process *process) {
    wait_to_ready[wait_to_ready_size++] = process;
}

// Dequeue all processes from "wait-to-ready" queue to ready queue
void wait_to_ready_dequeue() {
    for (int i = 0; i < wait_to_ready_size; i++) {
        ready_enqueue(wait_to_ready[i]);
        wait_to_ready[i] = NULL;
    }
    wait_to_ready_size = 0;
}
    
// Function to get processes that have arrived at the current clock time
void get_processes() {
    for (int i = 0; i < process_count; i++) {
        if (all_processes[i].arrival_time == clock_time) {
            printf("[Clock: %d] PID %d arrived\n",
                   clock_time, all_processes[i].pid);
            ready_enqueue(&all_processes[i]);
        }
    }
}

void aging_operation() {
    for (int i = 0; i < ready_queue_size; i++) {
        if ((clock_time - ready_queue[i]->aging_counter) >= AGING_LIMIT) {
            if (ready_queue[i]->priority != 0)
            {
                ready_queue[i]->priority--;
            }
            ready_queue[i]->aging_counter = clock_time; 
        }
    }
}

void* io_thread(void* arg){
    pthread_mutex_lock(&scheduler_mutex);
    while (!stop_flag) {
        pthread_cond_wait(&scheduler_cond, &scheduler_mutex);
        wait_dequeue();
        wait_to_ready_dequeue();
    }
    pthread_mutex_unlock(&scheduler_mutex);

}


int main(int argc, char *argv[]) {
    if (argc != 2) 
    {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Load processes from the input file
    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        perror("Error opening file");
        return 1;
    }

    for (size_t i = 0; i < MAX_PROCESS_NUM; i++)
    {
        if (fscanf(input_file, "%d %d %d %d %d %d",
                   &all_processes[i].pid,
                   &all_processes[i].arrival_time,
                   &all_processes[i].cpu_execution_time,
                   &all_processes[i].interval_time,
                   &all_processes[i].io_time,
                   &all_processes[i].priority) == 6)
        {
            all_processes[i].aging_counter = 0; // Going to be changed as soon as the process enters the ready queue
            all_processes[i].io_finish_time = -1; // -1 indicates that the process is not currently performing I/O
            all_processes[i].cpu_entered_time = -1; // -1 indicates that the process has not yet entered the CPU
            process_count++;
        }
        else
        {
            break; 
        }
    }

    fclose(input_file);

    if (process_count == 0)
    {
        printf("No processes loaded from the input file.\n");
        return 1;
    }

    // Sort processes by arrival time
    qsort(all_processes, process_count, sizeof(Process), compare_by_arrival_time);

    pthread_t io_thread_id;
    pthread_create(&io_thread_id, NULL, io_thread, NULL);

    // Main scheduling loop 
    int all_processes_completed = 0;
    while (all_processes_completed < process_count) {
        pthread_mutex_lock(&scheduler_mutex);

        get_processes();
        aging_operation();

        if (running_process != NULL)
        {
            running_process->cpu_execution_time--;
            if (running_process->cpu_execution_time == 0)
            {
                printf("[Clock: %d] PID %d TERMINATED\n",
                       clock_time, running_process->pid);
                
                running_process = NULL;
                all_processes_completed++;
            }else if (clock_time - running_process->cpu_entered_time >= running_process->interval_time)
            {
                wait_enqueue(running_process);
                running_process = NULL;
            }
            
        }

        if (running_process == NULL && ready_queue_size > 0) {
            running_process = ready_dequeue();
            running_process->cpu_entered_time = clock_time;
        } 

        clock_time++;

        pthread_cond_broadcast(&scheduler_cond);
        pthread_mutex_unlock(&scheduler_mutex);    
        usleep(1000);
    }    
    
    pthread_mutex_lock(&scheduler_mutex);
    stop_flag = 1;
    pthread_cond_broadcast(&scheduler_cond);
    pthread_mutex_unlock(&scheduler_mutex);

    pthread_join(io_thread_id, NULL);
    return 0;
    
}
    

