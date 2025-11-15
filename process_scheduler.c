#include <process_scheduler.h>

int clock_time = 0;
int stop_flag = 0;

int ready_queue_size = 0;
Process *ready_queue[MAX_PROCESS_NUM];

int wait_queue_size = 0;
Process *wait_queue[MAX_PROCESS_NUM];

int wait_to_ready_size = 0;
Process *wait_to_ready[MAX_PROCESS_NUM]; // Temporary queue for processes moving from wait to ready

Process *running_process = NULL;

// Comparison function to sort processes by arrival time
int compare_by_arrival_time(const void *a, const void *b) {
    Process *process_a = (Process *)a;
    Process *process_b = (Process *)b;
    return process_a->arrival_time - process_b->arrival_time;
}

// Adding to ready queue
void ready_enqueue(Process *process) {
    process->aging_counter = clock_time;
    ready_queue[ready_queue_size++] = process;
}

// Removing from ready queue based on priority and remaining CPU time
Process* ready_dequeue() {
    int MIN_PRIORITY = -1;
    for (int i = 0; i < ready_queue_size - 1; i++) {
        if (ready_queue[i]->priority < MIN_PRIORITY)
        {
            MIN_PRIORITY = ready_queue[i]->priority;
        }
    }

    int MIN_REMAINING_TIME = -1;
    Process* selected_process = NULL;
    int seleceted_index = -1;
    for (int i = 0; i < ready_queue_size - 1; i++) {
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
    return selected_process;
} 

// Adding to wait queue
void wait_enqueue(Process *process) {
    process->io_finish_time = clock_time + process->io_time;
    wait_queue[wait_queue_size++] = process;
}

// Removing from wait queue when I/O is complete
void wait_dequeue() {
    for (int i = 0; i < wait_queue_size; i++) {
        if (wait_queue[i]->io_finish_time == clock_time)
        {
            wait_queue[i]->io_finish_time = -1; 
            wait_to_ready_enqueue(wait_queue[i]);
            wait_queue[i] = NULL;
            wait_queue_size--;
        }
    }
    null_deletor(wait_queue);
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

    int process_count = 0;
    Process all_processes[MAX_PROCESS_NUM];
    for (size_t i = 0; i < MAX_PROCESS_NUM; i++)
    {
        if (fscanf(input_file, "%d %d %d %d %d %d",
                   &all_processes[i].pid,
                   &all_processes[i].arrival_time,
                   &all_processes[i].cpu_execution_time,
                   &all_processes[i].interval_time,
                   &all_processes[i].io_time,
                   &all_processes[i].priority))
        {
            all_processes[i].aging_counter = 0; // Going to be changed as soon as the process enters the ready queue
            all_processes[i].io_finish_time = -1; // -1 indicates that the process is not currently performing I/O
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
    


        
    
}
    

