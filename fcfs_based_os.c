#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_PROCESS_TABLE_BUCKETS 1009
#define MAX_PROCESS_NAME_LEN 64
#define MAX_PROCESSES 1000
#define MAX_KILL_EVENTS 1000
#define MAX_INT_STR_LEN 16

typedef enum
{
    STATE_READY,
    STATE_RUNNING,
    STATE_WAITING,
    STATE_TERMINATED
} ProcessExecutionState;

typedef struct ProcessControlRecord
{
    char process_name[MAX_PROCESS_NAME_LEN];
    int process_id;
    int total_cpu_burst;
    int remaining_cpu_burst;
    int io_start_tick;
    int io_duration_ticks;
    int remaining_io_ticks;
    int executed_cpu_time;
    int completion_tick;
    int configured_io_duration;
    int terminated_by_kill;
    int killed_at_tick;
    ProcessExecutionState execution_state;
    struct ProcessControlRecord *next_process;
} ProcessControlRecord;

typedef struct ProcessLinkedQueue
{
    ProcessControlRecord *front_process;
    ProcessControlRecord *rear_process;
} ProcessLinkedQueue;

typedef struct ProcessHashEntry
{
    int process_id_key;
    ProcessControlRecord *process_record;
    struct ProcessHashEntry *next_entry;
} ProcessHashEntry;

typedef struct KillEventRecord
{
    int kill_pid;
    int kill_time;
} KillEventRecord;

typedef struct RawProcessLine
{
    char raw_name[MAX_PROCESS_NAME_LEN];
    char raw_pid_str[MAX_INT_STR_LEN];
    char raw_burst_str[MAX_INT_STR_LEN];
    char raw_io_start_str[MAX_INT_STR_LEN];
    char raw_io_duration_str[MAX_INT_STR_LEN];
} RawProcessLine;

typedef struct ProcessSpecification
{
    char spec_name[MAX_PROCESS_NAME_LEN];
    int spec_pid;
    int spec_burst;
    int spec_io_start;
    int spec_io_duration;
} ProcessSpecification;

static ProcessHashEntry *process_table[MAX_PROCESS_TABLE_BUCKETS];
static ProcessLinkedQueue ready_queue;
static ProcessLinkedQueue waiting_queue;
static ProcessLinkedQueue finished_queue;
static ProcessControlRecord *registered_processes[MAX_PROCESSES];
static KillEventRecord kill_event_list[MAX_KILL_EVENTS];
static int registered_process_count = 0;
static int kill_event_count = 0;
static int total_process_count = 0;

int compute_bucket_index(int key);
void process_table_insert(int key, ProcessControlRecord *process_record);
ProcessControlRecord *process_table_lookup(int key);
void init_queue(ProcessLinkedQueue *queue);
void enqueue(ProcessLinkedQueue *queue, ProcessControlRecord *process_record);
ProcessControlRecord *dequeue(ProcessLinkedQueue *queue);
ProcessControlRecord *remove_from_queue_by_pid(ProcessLinkedQueue *queue, int process_id);
void queue_for_each(ProcessLinkedQueue *queue, void (*operation)(ProcessControlRecord *));
ProcessControlRecord *create_process_record(const ProcessSpecification *process_spec);
void trim_string(char *input_string);
bool is_valid_process_name(const char name[MAX_PROCESS_NAME_LEN]);
bool is_valid_integer_string(const char num_string[MAX_INT_STR_LEN]);
bool is_dash_or_integer(const char value[MAX_INT_STR_LEN]);
bool validate_raw_input(const RawProcessLine *raw_input);
bool is_valid_kill_input(const char pid_str[MAX_INT_STR_LEN], const char time_string[MAX_INT_STR_LEN]);
void parse_input_line(char *line);
void decrement_io_for_process(ProcessControlRecord *process_record);
void move_completed_io_to_ready();
void mark_process_terminated(ProcessControlRecord *process_record, int current_time);
static void kill_running_process(ProcessControlRecord **running_process_pointer, int current_time, int *terminated_counter_pointer);
static void kill_queued_process(int target_pid, int current_time, int *terminated_counter_pointer);
static void handle_kill_event(int current_time, int target_pid, ProcessControlRecord **running_process_pointer,int *terminated_counter_pointer);
void apply_pending_kill_events(int current_time, ProcessControlRecord **running_process_pointer, int *terminated_counter_pointer);
void execute_scheduler();
int compare_process_by_pid(const void *left_pointer, const void *right_pointer);
void destroy_process_table();
void destroy_process_records();
void print_result_table();
void read_kill_events_after_process_input();
void cleanup_all_memory();

int compute_bucket_index(int key)
{
    int computed_index = key % MAX_PROCESS_TABLE_BUCKETS;
    return (computed_index < 0 ? computed_index + MAX_PROCESS_TABLE_BUCKETS : computed_index);
}

void process_table_insert(int key, ProcessControlRecord *process_record)
{
    int bucket_index = compute_bucket_index(key);
    ProcessHashEntry *entry_pointer = process_table[bucket_index];

    while (entry_pointer)
    {
        if (entry_pointer->process_id_key == key)
        {
            return;
        }
        entry_pointer = entry_pointer->next_entry;
    }

    ProcessHashEntry *new_entry = (ProcessHashEntry *)malloc(sizeof(ProcessHashEntry));
    if (new_entry == NULL) {
        printf("Memory allocation failed!!!");
        return;
    }

    new_entry->process_id_key = key;
    new_entry->process_record = process_record;
    new_entry->next_entry = process_table[bucket_index];
    process_table[bucket_index] = new_entry;
}

ProcessControlRecord *process_table_lookup(int key)
{
    int bucket_index = compute_bucket_index(key);
    ProcessHashEntry *entry_pointer = process_table[bucket_index];

    while (entry_pointer)
    {
        if (entry_pointer->process_id_key == key)
        {
            return entry_pointer->process_record;
        }
        entry_pointer = entry_pointer->next_entry;
    }

    return NULL;
}

void init_queue(ProcessLinkedQueue *queue)
{
    queue->front_process = queue->rear_process = NULL;
}

void enqueue(ProcessLinkedQueue *queue, ProcessControlRecord *process_record)
{
    process_record->next_process = NULL;

    if (queue->rear_process)
    {
        queue->rear_process->next_process = process_record;
        queue->rear_process = process_record;
    }
    else
    {
        queue->front_process = queue->rear_process = process_record;
    }
}

ProcessControlRecord *dequeue(ProcessLinkedQueue *queue)
{
    if (!queue->front_process)
    {
        return NULL;
    }

    ProcessControlRecord *process_record = queue->front_process;
    queue->front_process = process_record->next_process;

    if (!queue->front_process)
    {
        queue->rear_process = NULL;
    }

    return process_record;
}

ProcessControlRecord *remove_from_queue_by_pid(ProcessLinkedQueue *queue, int process_id)
{
    ProcessControlRecord *current_process = queue->front_process;
    ProcessControlRecord *previous_process = NULL;

    while (current_process)
    {
        if (current_process->process_id == process_id)
        {
            if (previous_process)
            {
                previous_process->next_process = current_process->next_process;
            }
            else
            {
                queue->front_process = current_process->next_process;
            }

            if (current_process == queue->rear_process)
            {
                queue->rear_process = previous_process;
            }

            current_process->next_process = NULL;
            return current_process;
        }

        previous_process = current_process;
        current_process = current_process->next_process;
    }

    return NULL;
}

void queue_for_each(ProcessLinkedQueue *queue, void (*operation)(ProcessControlRecord *))
{
    ProcessControlRecord *current_process = queue->front_process;

    while (current_process)
    {
        ProcessControlRecord *next_process = current_process->next_process;
        operation(current_process);
        current_process = next_process;
    }
}

ProcessControlRecord *create_process_record(const ProcessSpecification *process_spec)
{
    ProcessControlRecord *process_record = (ProcessControlRecord *)malloc(sizeof(ProcessControlRecord));
    if (process_record == NULL) {
        printf("Memory allocation failed!!!");
        return NULL;
    }

    strcpy(process_record->process_name, process_spec->spec_name);
    process_record->process_id = process_spec->spec_pid;
    process_record->total_cpu_burst = process_spec->spec_burst;
    process_record->remaining_cpu_burst = process_spec->spec_burst;
    process_record->io_start_tick = process_spec->spec_io_start;
    process_record->io_duration_ticks = process_spec->spec_io_duration;
    process_record->remaining_io_ticks = 0;
    process_record->configured_io_duration = process_spec->spec_io_duration;
    process_record->executed_cpu_time = 0;
    process_record->completion_tick = -1;
    process_record->terminated_by_kill = 0;
    process_record->killed_at_tick = -1;
    process_record->execution_state = STATE_READY;
    process_record->next_process = NULL;
    return process_record;
}

void trim_string(char *input_string)
{
    int start_index = 0;
    int write_index = 0;

    while (isspace((unsigned char)input_string[start_index]))
    {
        start_index++;
    }

    while (input_string[start_index])
    {
        input_string[write_index++] = input_string[start_index++];
    }

    input_string[write_index] = '\0';

    while (write_index > 0 && isspace((unsigned char)input_string[write_index - 1]))
    {
        input_string[--write_index] = '\0';
    }
}

bool is_valid_process_name(const char name[MAX_PROCESS_NAME_LEN])
{
    int name_index = 0;

    while (name[name_index] == ' ')
    {
        name_index++;
    }

    if (name[name_index] == '\0')
    {
        printf("Process name can't be empty.\n");
        return false;
    }

    return true;
}

bool is_valid_integer_string(const char number_string[MAX_INT_STR_LEN])
{
    int char_index = 0;

    if (number_string[0] == '\0')
    {
        return false;
    }

    while (number_string[char_index] != '\0')
    {
        if (!isdigit((unsigned char)number_string[char_index]))
        {
            return false;
        }
        char_index++;
    }

    return true;
}

bool is_dash_or_integer(const char value[MAX_INT_STR_LEN])
{
    if (strcmp(value, "-") == 0)
    {
        return true;
    }
    return is_valid_integer_string(value);
}

bool validate_raw_input(const RawProcessLine *raw_input)
{
    if (!is_valid_process_name(raw_input->raw_name))
    {
        return false;
    }

    if (!is_valid_integer_string(raw_input->raw_pid_str))
    {
        printf("Invalid input. Process ID must be an integer.\n");
        return false;
    }

    if (!is_valid_integer_string(raw_input->raw_burst_str))
    {
        printf("Invalid input. Burst must be an integer.\n");
        return false;
    }

    if (!is_dash_or_integer(raw_input->raw_io_start_str))
    {
        printf("Invalid input. I/O start must be integer or '-'.\n");
        return false;
    }

    if (!is_dash_or_integer(raw_input->raw_io_duration_str))
    {
        printf("Invalid input. I/O duration must be integer or '-'.\n");
        return false;
    }

    int pid_value = atoi(raw_input->raw_pid_str);

    if (process_table_lookup(pid_value))
    {
        printf("Duplicate PID %d detected.\n", pid_value);
        return false;
    }

    return true;
}

bool is_valid_kill_input(const char pid_str[MAX_INT_STR_LEN], const char time_string[MAX_INT_STR_LEN])
{
    if (!is_valid_integer_string(pid_str))
    {
        printf("Invalid input. Kill PID must be an integer.\n");
        return false;
    }

    if (!is_valid_integer_string(time_string))
    {
        printf("Invalid input. Kill time must be an integer.\n");
        return false;
    }

    return true;
}

void parse_input_line(char *line)
{
    char first_word[16];
    sscanf(line, "%s", first_word);

    if (strcasecmp(first_word, "KILL") == 0)
    {
        char kill_pid_str[MAX_INT_STR_LEN];
        char kill_time_str[MAX_INT_STR_LEN];
        sscanf(line + strlen(first_word), "%s %s", kill_pid_str, kill_time_str);

        if (!is_valid_kill_input(kill_pid_str, kill_time_str))
        {
            return;
        }

        int pid_value = atoi(kill_pid_str);
        int time_value = atoi(kill_time_str);
        kill_event_list[kill_event_count].kill_pid = pid_value;
        kill_event_list[kill_event_count].kill_time = time_value;
        kill_event_count++;
        return;
    }

    RawProcessLine raw_input;
    memset(&raw_input, 0, sizeof(raw_input));

    char io_start_buffer[MAX_INT_STR_LEN];
    char io_duration_buffer[MAX_INT_STR_LEN];
    char pid_buffer[MAX_INT_STR_LEN];
    char burst_buffer[MAX_INT_STR_LEN];
    char name_buffer[MAX_PROCESS_NAME_LEN];

    int parsed_items = sscanf(line, "%63s %15s %15s %15s %15s", name_buffer, pid_buffer, burst_buffer, io_start_buffer, io_duration_buffer);

    if (parsed_items < 3)
    {
        return;
    }

    strncpy(raw_input.raw_name, name_buffer, MAX_PROCESS_NAME_LEN - 1);
    strncpy(raw_input.raw_pid_str, pid_buffer, MAX_INT_STR_LEN - 1);
    strncpy(raw_input.raw_burst_str, burst_buffer, MAX_INT_STR_LEN - 1);

    if (parsed_items >= 4)
    {
        strncpy(raw_input.raw_io_start_str, io_start_buffer, MAX_INT_STR_LEN - 1);
    }
    else
    {
        strncpy(raw_input.raw_io_start_str, "-", MAX_INT_STR_LEN - 1);
    }

    if (parsed_items >= 5)
    {
        strncpy(raw_input.raw_io_duration_str, io_duration_buffer, MAX_INT_STR_LEN - 1);
    }
    else
    {
        strncpy(raw_input.raw_io_duration_str, "-", MAX_INT_STR_LEN - 1);
    }

    if (!validate_raw_input(&raw_input))
    {
        return;
    }

    ProcessSpecification process_specification;
    memset(&process_specification, 0, sizeof(process_specification));
    strncpy(process_specification.spec_name, raw_input.raw_name, MAX_PROCESS_NAME_LEN - 1);

    process_specification.spec_pid = atoi(raw_input.raw_pid_str);
    process_specification.spec_burst = atoi(raw_input.raw_burst_str);
    process_specification.spec_io_start = (strcmp(raw_input.raw_io_start_str, "-") != 0) ? atoi(raw_input.raw_io_start_str) : -1;
    process_specification.spec_io_duration = (strcmp(raw_input.raw_io_duration_str, "-") != 0) ? atoi(raw_input.raw_io_duration_str) : 0;

    ProcessControlRecord *new_process_record = create_process_record(&process_specification);
    if (new_process_record == NULL) {
        return;
    }

    registered_processes[registered_process_count++] = new_process_record;
    total_process_count++;
    process_table_insert(process_specification.spec_pid, new_process_record);
    enqueue(&ready_queue, new_process_record);
}

void decrement_io_for_process(ProcessControlRecord *process_record)
{
    if (process_record->remaining_io_ticks > 0)
    {
        process_record->remaining_io_ticks--;
    }
}

void move_completed_io_to_ready()
{
    ProcessControlRecord *previous_process = NULL;
    ProcessControlRecord *current_process = waiting_queue.front_process;

    while (current_process)
    {
        ProcessControlRecord *next_process = current_process->next_process;

        if (current_process->remaining_io_ticks == 0)
        {
            if (previous_process)
            {
                previous_process->next_process = current_process->next_process;
            }
            else
            {
                waiting_queue.front_process = current_process->next_process;
            }

            if (current_process == waiting_queue.rear_process)
            {
                waiting_queue.rear_process = previous_process;
            }

            current_process->execution_state = STATE_READY;
            enqueue(&ready_queue, current_process);
        }
        else
        {
            previous_process = current_process;
        }

        current_process = next_process;
    }
}

void mark_process_terminated(ProcessControlRecord *process_record, int current_time)
{
    process_record->execution_state = STATE_TERMINATED;
    process_record->completion_tick = current_time;
    enqueue(&finished_queue, process_record);
}

static void kill_running_process(ProcessControlRecord **running_process_pointer, int current_time, int *terminated_counter_pointer)
{
    ProcessControlRecord *process_to_kill = *running_process_pointer;

    *running_process_pointer = NULL;
    process_to_kill->terminated_by_kill = 1;
    process_to_kill->killed_at_tick = current_time;

    mark_process_terminated(process_to_kill, current_time);
    (*terminated_counter_pointer)++;
}

static void kill_queued_process(int target_pid, int current_time, int *terminated_counter_pointer)
{
    ProcessControlRecord *process_record = remove_from_queue_by_pid(&ready_queue, target_pid);

    if (!process_record)
    {
        process_record = remove_from_queue_by_pid(&waiting_queue, target_pid);
    }

    if (!process_record)
    {
        return;
    }

    process_record->terminated_by_kill = 1;
    process_record->killed_at_tick = current_time;

    mark_process_terminated(process_record, current_time);
    (*terminated_counter_pointer)++;
}

static void handle_kill_event(int current_time, int target_pid, ProcessControlRecord **running_process_pointer, int *terminated_counter_pointer)
{
    ProcessControlRecord *target_process = process_table_lookup(target_pid);

    if (!target_process || target_process->execution_state == STATE_TERMINATED)
    {
        return;
    }

    if ((*running_process_pointer) && ((*running_process_pointer)->process_id == target_pid))
    {
        kill_running_process(running_process_pointer, current_time, terminated_counter_pointer);
    }
    else
    {
        kill_queued_process(target_pid, current_time, terminated_counter_pointer);
    }
}

void apply_pending_kill_events(int current_time, ProcessControlRecord **running_process_pointer, int *terminated_counter_pointer)
{
    for (int event_index = 0; event_index < kill_event_count; event_index++)
    {
        if (kill_event_list[event_index].kill_time != current_time)
        {
            continue;
        }

        handle_kill_event(current_time, kill_event_list[event_index].kill_pid, running_process_pointer, terminated_counter_pointer);
    }
}

void execute_scheduler()
{
    int time_tick = 0;
    int terminated_counter = 0;
    ProcessControlRecord *running_process = NULL;

    while (terminated_counter < total_process_count)
    {
        apply_pending_kill_events(time_tick, &running_process, &terminated_counter);

        if (!running_process)
        {
            running_process = dequeue(&ready_queue);

            if (running_process)
            {
                running_process->execution_state = STATE_RUNNING;
            }
        }

        if (running_process)
        {
            running_process->executed_cpu_time++;
            running_process->remaining_cpu_burst--;
        }

        queue_for_each(&waiting_queue, decrement_io_for_process);

        if (running_process)
        {
            if ((running_process->remaining_cpu_burst > 0) && 
                (running_process->io_start_tick >= 0) && 
                (running_process->executed_cpu_time == running_process->io_start_tick) && 
                (running_process->io_duration_ticks > 0))
            {
                running_process->remaining_io_ticks = running_process->io_duration_ticks;
                running_process->execution_state = STATE_WAITING;
                enqueue(&waiting_queue, running_process);
                running_process = NULL;
            }
            else if (running_process->remaining_cpu_burst == 0)
            {
                mark_process_terminated(running_process, time_tick + 1);
                running_process = NULL;
                terminated_counter++;
            }
        }

        move_completed_io_to_ready();
        time_tick++;
    }
}

int compare_process_by_pid(const void *left_pointer, const void *right_pointer)
{
    ProcessControlRecord *first_process = *(ProcessControlRecord **)left_pointer;
    ProcessControlRecord *second_process = *(ProcessControlRecord **)right_pointer;
    return first_process->process_id - second_process->process_id;
}

void print_result_table()
{
    ProcessControlRecord *sorted_list[MAX_PROCESSES];
    int any_killed = 0;

    for (int registry_index = 0; registry_index < registered_process_count; registry_index++)
    {
        sorted_list[registry_index] = registered_processes[registry_index];
        if (sorted_list[registry_index]->terminated_by_kill)
        {
            any_killed = 1;
        }
    }

    qsort(sorted_list, registered_process_count, sizeof(ProcessControlRecord *), compare_process_by_pid);

    if (!any_killed)
    {
        printf("%-6s %-20s %-6s %-6s %-12s %-8s\n",
               "PID", "Name", "CPU", "IO", "Turnaround", "Waiting");

        for (int print_index = 0; print_index < registered_process_count; print_index++)
        {
            ProcessControlRecord *process_record = sorted_list[print_index];
            int turnaround = process_record->completion_tick;
            int waiting = turnaround - process_record->total_cpu_burst;
            if (waiting < 0) waiting = 0;
            printf("%-6d %-20s %-6d %-6d %-12d %-8d\n",
                   process_record->process_id,
                   process_record->process_name,
                   process_record->total_cpu_burst,
                   process_record->configured_io_duration,
                   turnaround,
                   waiting);
        }
    }
    else
    {
        printf("%-6s %-20s %-6s %-6s %-18s %-12s %-8s\n",
               "PID", "Name", "CPU", "IO", "Status", "Turnaround", "Waiting");

        for (int print_index = 0; print_index < registered_process_count; print_index++)
        {
            ProcessControlRecord *process_record = sorted_list[print_index];

            if (process_record->terminated_by_kill)
            {
                char status_text[32];
                snprintf(status_text, sizeof(status_text), "KILLED at %d", process_record->killed_at_tick);
                printf("%-6d %-20s %-6d %-6d %-18s %-12s %-8s\n",
                       process_record->process_id,
                       process_record->process_name,
                       process_record->total_cpu_burst,
                       process_record->configured_io_duration,
                       status_text,
                       "-",
                       "-");
            }
            else
            {
                int turnaround = process_record->completion_tick;
                int waiting = turnaround - process_record->total_cpu_burst;
                if (waiting < 0) waiting = 0;
                printf("%-6d %-20s %-6d %-6d %-18s %-12d %-8d\n",
                       process_record->process_id,
                       process_record->process_name,
                       process_record->total_cpu_burst,
                       process_record->configured_io_duration,
                       "OK",
                       turnaround,
                       waiting);
            }
        }
    }
}

void read_kill_events_after_process_input()
{
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stdin))
    {
        trim_string(buffer);
        if (strlen(buffer) == 0)
        {
            break;
        }
        char verb_buffer[16];
        char pid_string[MAX_INT_STR_LEN];
        char time_string[MAX_INT_STR_LEN];
        int fields_read = sscanf(buffer, "%15s %15s %15s", verb_buffer, pid_string, time_string);
        if (fields_read != 3)
        {
            continue;
        }
        for (int char_index = 0; verb_buffer[char_index]; char_index++)
        {
            verb_buffer[char_index] = (char) tolower((unsigned char)verb_buffer[char_index]);
        }
        if (strcmp(verb_buffer, "kill") != 0)
        {
            continue;
        }
        if (!is_valid_kill_input(pid_string, time_string))
        {
            continue;
        }
        int pid_value = atoi(pid_string);
        int time_value = atoi(time_string);
        kill_event_list[kill_event_count].kill_pid = pid_value;
        kill_event_list[kill_event_count].kill_time = time_value;
        kill_event_count++;
    }
}

void destroy_process_table()
{
    for (int index = 0; index < MAX_PROCESS_TABLE_BUCKETS; index++)
    {
        ProcessHashEntry *current = process_table[index];
        while (current)
        {
            ProcessHashEntry *temporaryNode = current;
            current = current->next_entry;
            free(temporaryNode);
        }
        process_table[index] = NULL;
    }
}

void destroy_process_records()
{
    for (int index = 0; index < registered_process_count; index++)
    {
        free(registered_processes[index]);
        registered_processes[index] = NULL;
    }
}

void cleanup_all_memory()
{
    destroy_process_table();
    destroy_process_records();
    printf("Freeing up memory...Memory released!!!");
}

int main()
{
    init_queue(&ready_queue);
    init_queue(&waiting_queue);
    init_queue(&finished_queue);

    printf("Enter the input in the given format :\n");
    printf("<process_name>  <process_id>  <cpu_burst_time>  <io_start_time>  <io_duration_time>\n\n");

    char input_line[256];

    while (fgets(input_line, sizeof(input_line), stdin))
    {
        trim_string(input_line);

        if (strlen(input_line) == 0)
        {
            read_kill_events_after_process_input();
            break;
        }

        parse_input_line(input_line);
    }

    if (total_process_count == 0)
    {
        printf("No valid processes entered.\n");
        return 0;
    }

    execute_scheduler();
    print_result_table();
    cleanup_all_memory();
    return 0;
}
