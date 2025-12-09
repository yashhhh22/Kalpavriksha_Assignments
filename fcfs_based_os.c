#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAP_CAPACITY 100
#define LABEL_MAX 50
#define BUFFER_MAX 100
#define DIGIT_MAX 8
#define NODE_COLLECTION_MAX 100

typedef enum
{
    PROCESS_NEW,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_DONE,
    PROCESS_FORCE_STOPPED
} ProcessState;

typedef struct ProcessNode
{
    int processIdentifier;
    char processLabel[LABEL_MAX];
    int startTick;
    int cpuBurstTotal;
    int cpuBurstRemaining;
    int cpuBurstProgress;
    int ioBeginAfterCpu;
    int ioLength;
    int ioProgress;
    int ioRemaining;
    int finishTick;
    int ioJustStartedFlag;
    struct ProcessNode *mapChain;
    ProcessState processState;
} ProcessNode;

typedef struct Node
{
    ProcessNode *process;
    struct Node *next;
} Node;

typedef struct Queue
{
    Node *head;
    Node *tail;
    int length;
} Queue;

typedef struct KillEventNode
{
    int targetJobId;
    int killTick;
    struct KillEventNode *next;
} KillEventNode;

static ProcessNode *processMap[MAP_CAPACITY];
static KillEventNode *killEventHead = NULL;
static bool hadKillEvents = false;

void init_queue(Queue *queue);
void init_process_map();
void enqueue_job(Queue *queue, ProcessNode *process);
ProcessNode *dequeue_job(Queue *queue);
int map_hash(int processId);
void map_insert(ProcessNode *process);
ProcessNode *map_lookup(int processId);
ProcessNode *build_process_node(char *label, int id, int burst, int ioStart, int ioLen);
int remove_process_from_queue(Queue *queue, int processId);
void append_kill_event(int processId, int tick);
void read_process_input(int totalProcesses, Queue *ready);
void read_kill_events_input(int totalKills);
bool line_nonempty(char buffer[BUFFER_MAX]);
bool name_valid(char label[LABEL_MAX]);
bool digits_only(char digits[DIGIT_MAX]);
bool dash_or_digits(char *token);
void apply_kill_events(int clockTick, Queue *ready, Queue *waiting, Queue *finished, ProcessNode **current);
void progress_io_for_waiting(Queue *waiting, Queue *ready);
void run_scheduler(Queue *ready, Queue *waiting, Queue *finished);
void emit_report(Queue *finished);
void free_map_entries();

int main()
{
    int total_processes = 0;
    int total_kills = 0;
    init_process_map();

    Queue ready_line;
    Queue waiting_line;
    Queue finished_line;

    init_queue(&ready_line);
    init_queue(&waiting_line);
    init_queue(&finished_line);

    if (scanf("%d", &total_processes) != 1)
    {
        return 1;
    }
    getchar();
    read_process_input(total_processes, &ready_line);

    if (scanf("%d", &total_kills) != 1)
    {
        return 1;
    }
    getchar();
    read_kill_events_input(total_kills);

    run_scheduler(&ready_line, &waiting_line, &finished_line);
    emit_report(&finished_line);

    free_map_entries();
    return 0;
}

bool line_nonempty(char buffer[BUFFER_MAX])
{
    if (buffer[0] == '\0')
    {
        return false;
    }
    return true;
}

void read_process_input(int totalProcesses, Queue *ready)
{
    int read_count = 0;
    while (read_count < totalProcesses)
    {
        char input_line[BUFFER_MAX];
        if (fgets(input_line, sizeof(input_line), stdin) == NULL)
        {
            continue;
        }
        if (!line_nonempty(input_line))
        {
            continue;
        }
        char label[LABEL_MAX];
        char idToken[DIGIT_MAX];
        char burstToken[DIGIT_MAX];
        char ioStartToken[DIGIT_MAX];
        char ioLenToken[DIGIT_MAX];

        int fields = sscanf(input_line, "%s %s %s %s %s", label, idToken, burstToken, ioStartToken, ioLenToken);
        if (fields != 5)
        {
            continue;
        }
        if (!name_valid(label))
        {
            continue;
        }
        if (!digits_only(idToken) || !digits_only(burstToken))
        {
            continue;
        }
        if (!dash_or_digits(ioStartToken) || !dash_or_digits(ioLenToken))
        {
            continue;
        }
        int id = atoi(idToken);
        int cpu = atoi(burstToken);
        int ioStart = 0;
        int ioLen = 0;
        if (strcmp(ioStartToken, "-") != 0)
        {
            ioStart = atoi(ioStartToken);
        }
        if (strcmp(ioLenToken, "-") != 0)
        {
            ioLen = atoi(ioLenToken);
        }
        ProcessNode *process = build_process_node(label, id, cpu, ioStart, ioLen);
        process->processState = PROCESS_READY;
        process->cpuBurstRemaining = cpu;
        map_insert(process);
        enqueue_job(ready, process);
        read_count++;
    }
}

void read_kill_events_input(int totalKills)
{
    int read_kill = 0;
    if (totalKills > 0)
    {
        hadKillEvents = true;
    }
    while (read_kill < totalKills)
    {
        char input_line[BUFFER_MAX];
        if (fgets(input_line, sizeof(input_line), stdin) == NULL)
        {
            continue;
        }
        if (!line_nonempty(input_line))
        {
            continue;
        }
        char idToken[DIGIT_MAX];
        char timeToken[DIGIT_MAX];
        char verb[8];
        int fields = sscanf(input_line, "%7s %7s %7s", verb, idToken, timeToken);
        if (fields != 3)
        {
            continue;
        }
        for (int index = 0; verb[index]; index++)
        {
            verb[index] = tolower((unsigned char) verb[index]);
        }
        if (strcmp(verb, "kill") != 0)
        {
            continue;
        }
        if (!digits_only(idToken) || !digits_only(timeToken))
        {
            continue;
        }
        int killId = atoi(idToken);
        int killTime = atoi(timeToken);
        append_kill_event(killId, killTime);
        read_kill++;
    }
}

bool dash_or_digits(char *token)
{
    if (strcmp(token, "-") == 0)
    {
        return true;
    }
    return digits_only(token);
}

bool name_valid(char label[LABEL_MAX])
{
    int index = 0;
    while (label[index] == ' ')
    {
        index++;
    }
    if (label[index] == '\0')
    {
        return false;
    }
    return true;
}

bool digits_only(char digits[DIGIT_MAX])
{
    int index = 0;
    if (digits[0] == '\0')
    {
        return false;
    }
    while (digits[index] != '\0')
    {
        if (!isdigit((unsigned char) digits[index]))
        {
            return false;
        }
        index++;
    }
    return true;
}

void init_queue(Queue *queue)
{
    queue->head = NULL;
    queue->tail = NULL;
    queue->length = 0;
}

void init_process_map()
{
    for (int index = 0; index < MAP_CAPACITY; index++)
    {
        processMap[index] = NULL;
    }
}

void enqueue_job(Queue *queue, ProcessNode *process)
{
    Node *newNode = malloc(sizeof(Node));
    newNode->process = process;
    newNode->next = NULL;
    if (queue->tail == NULL)
    {
        queue->head = newNode;
        queue->tail = newNode;
    }
    else
    {
        queue->tail->next = newNode;
        queue->tail = newNode;
    }
    queue->length++;
}

ProcessNode *dequeue_job(Queue *queue)
{
    if (queue->head == NULL)
    {
        return NULL;
    }
    Node *newNode = queue->head;
    ProcessNode *process = newNode->process;
    queue->head = newNode->next;
    if (queue->head == NULL)
    {
        queue->tail = NULL;
    }
    free(newNode);
    queue->length--;
    return process;
}

int remove_process_from_queue(Queue *queue, int processId)
{
    Node *currentNode = queue->head;
    Node *previousNode = NULL;
    while (currentNode != NULL)
    {
        if (currentNode->process->processIdentifier == processId)
        {
            if (previousNode == NULL)
            {
                queue->head = currentNode->next;
            }
            else
            {
                previousNode->next = currentNode->next;
            }
            if (currentNode == queue->tail)
            {
                queue->tail = previousNode;
            }
            free(currentNode);
            queue->length--;
            return 1;
        }
        previousNode = currentNode;
        currentNode = currentNode->next;
    }
    return 0;
}

int map_hash(int processId)
{
    int index = processId % MAP_CAPACITY;
    if (index < 0)
    {
        index += MAP_CAPACITY;
    }
    return index;
}

void map_insert(ProcessNode *process)
{
    int bucket = map_hash(process->processIdentifier);
    process->mapChain = processMap[bucket];
    processMap[bucket] = process;
}

ProcessNode *map_lookup(int processId)
{
    int bucket = map_hash(processId);
    ProcessNode *currentNode = processMap[bucket];
    while (currentNode != NULL)
    {
        if (currentNode->processIdentifier == processId)
        {
            return currentNode;
        }
        currentNode = currentNode->mapChain;
    }
    return NULL;
}

ProcessNode *build_process_node(char *label, int id, int burst, int ioStart, int ioLen)
{
    ProcessNode *node = malloc(sizeof(ProcessNode));
    node->processIdentifier = id;
    strncpy(node->processLabel, label, LABEL_MAX - 1);
    node->processLabel[LABEL_MAX - 1] = '\0';
    node->startTick = 0;
    node->cpuBurstTotal = burst;
    node->cpuBurstRemaining = burst;
    node->cpuBurstProgress = 0;
    node->ioBeginAfterCpu = ioStart;
    node->ioLength = ioLen;
    node->ioProgress = 0;
    node->ioRemaining = 0;
    node->finishTick = 0;
    node->ioJustStartedFlag = 0;
    node->mapChain = NULL;
    node->processState = PROCESS_NEW;

    return node;
}

void append_kill_event(int processId, int tick)
{
    KillEventNode *node = malloc(sizeof(KillEventNode));
    node->targetJobId = processId;
    node->killTick = tick;
    node->next = NULL;
    if (killEventHead == NULL)
    {
        killEventHead = node;
    }
    else
    {
        KillEventNode *currentNode = killEventHead;
        while (currentNode->next != NULL)
        {
            currentNode = currentNode->next;
        }
        currentNode->next = node;
    }
}

void apply_kill_events(int clockTick, Queue *ready, Queue *waiting, Queue *finished, ProcessNode **current)
{
    KillEventNode *currentNode = killEventHead;
    KillEventNode *previousNode = NULL;
    while (currentNode != NULL)
    {
        if (currentNode->killTick == clockTick)
        {
            ProcessNode * target = map_lookup(currentNode->targetJobId);
            if ((target != NULL) && (target->processState != PROCESS_DONE) && (target->processState != PROCESS_FORCE_STOPPED))
            {
                target->processState = PROCESS_FORCE_STOPPED;
                target->finishTick = clockTick;
                remove_process_from_queue(ready, target->processIdentifier);
                remove_process_from_queue(waiting, target->processIdentifier);
                if ((*current != NULL) && ((*current)->processIdentifier == currentNode->targetJobId))
                {
                    *current = NULL;
                }
                enqueue_job(finished, target);
            }
            if (previousNode == NULL)
            {
                killEventHead = currentNode->next;
                free(currentNode);
                currentNode = killEventHead;
            }
            else
            {
                previousNode->next = currentNode->next;
                free(currentNode);
                currentNode = previousNode->next;
            }
            continue;
        }
        previousNode = currentNode;
        currentNode = currentNode->next;
    }
}

void progress_io_for_waiting(Queue *waiting, Queue *ready)
{
    Node *currentNode = waiting->head;
    Node *previousNode = NULL;
    while (currentNode != NULL)
    {
        ProcessNode *temporaryProcess = currentNode->process;
        if (temporaryProcess->ioJustStartedFlag)
        {
            temporaryProcess->ioJustStartedFlag = 0;
            previousNode = currentNode;
            currentNode = currentNode->next;
            continue;
        }
        temporaryProcess->ioProgress++;
        temporaryProcess->ioRemaining--;
        if (temporaryProcess->ioRemaining <= 0)
        {
            temporaryProcess->processState = PROCESS_READY;
            temporaryProcess->cpuBurstRemaining = temporaryProcess->cpuBurstTotal - temporaryProcess->cpuBurstProgress;
            enqueue_job(ready, temporaryProcess);
            if (previousNode == NULL)
            {
                waiting->head = currentNode->next;
            }
            else
            {
                previousNode->next = currentNode->next;
            }
            if (currentNode == waiting->tail)
            {
                waiting->tail = previousNode;
            }
            Node *nodeToFree = currentNode;
            currentNode = currentNode->next;
            free(nodeToFree);
            waiting->length--;
            continue;
        }
        previousNode = currentNode;
        currentNode = currentNode->next;
    }
}

void run_scheduler(Queue *ready, Queue *waiting, Queue *finished)
{
    int clock = 0;
    ProcessNode *runningProcess = NULL;
    while (ready->length > 0 || waiting->length > 0 || runningProcess != NULL)
    {
        apply_kill_events(clock, ready, waiting, finished, &runningProcess);
        if (runningProcess == NULL && ready->length > 0)
        {
            runningProcess = dequeue_job(ready);
            if (runningProcess != NULL)
            {
                runningProcess->processState = PROCESS_RUNNING;
            }
        }
        if (runningProcess != NULL)
        {
            runningProcess->cpuBurstProgress++;
            runningProcess->cpuBurstRemaining--;
            if (runningProcess->cpuBurstProgress == runningProcess->ioBeginAfterCpu && runningProcess->ioLength > 0)
            {
                runningProcess->processState = PROCESS_WAITING;
                runningProcess->ioRemaining = runningProcess->ioLength;
                runningProcess->ioProgress = 0;
                runningProcess->ioJustStartedFlag = 1;
                enqueue_job(waiting, runningProcess);
                runningProcess = NULL;
            }
            else if (runningProcess->cpuBurstRemaining <= 0)
            {
                runningProcess->finishTick = clock + 1;
                runningProcess->processState = PROCESS_DONE;
                enqueue_job(finished, runningProcess);
                runningProcess = NULL;
            }
        }
        progress_io_for_waiting(waiting, ready);
        clock++;
    }
}

void emit_report(Queue *finished)
{
    ProcessNode *collected[NODE_COLLECTION_MAX];
    int total = 0;
    Node *currentNode = finished->head;
    while (currentNode != NULL && total < NODE_COLLECTION_MAX)
    {
        collected[total++] = currentNode->process;
        currentNode = currentNode->next;
    }
    for (int index = 0; index < total - 1; index++)
    {
        for (int nextIndex = index + 1; nextIndex < total; nextIndex++)
        {
            if (collected[index]->processIdentifier > collected[nextIndex]->processIdentifier)
            {
                ProcessNode *swappingValue = collected[index];
                collected[index] = collected[nextIndex];
                collected[nextIndex] = swappingValue;
            }
        }
    }
    if (hadKillEvents)
    {
        printf("%-5s %-12s %-5s %-5s %-12s %-8s\n", "PID", "Name", "CPU", "IO", "Turnaround", "Waiting");
    }
    else
    {
        printf("%-5s %-12s %-5s %-5s %-12s %-8s\n", "PID", "Name", "CPU", "IO", "Turnaround", "Waiting");
    }
    for (int index = 0; index < total; index++)
    {
        ProcessNode *node = collected[index];
        int cpu = node->cpuBurstTotal;
        int io = node->ioLength;
        if (node->processState == PROCESS_FORCE_STOPPED)
        {
            int turnaround = 0;
            int waiting = 0;
            printf("%-5d %-12s %-5d %-5d %-12d %-8d\n", node->processIdentifier, node->processLabel, cpu, io, turnaround, waiting);
        }
        else
        {
            int turnaround = node->finishTick - node->startTick;
            int waiting = turnaround - cpu;
            if (waiting < 0)
            {
                waiting = 0;
            }
            printf("%-5d %-12s %-5d %-5d %-12d %-8d\n", node->processIdentifier, node->processLabel, cpu, io, turnaround, waiting);
        }
    }
}

void free_map_entries()
{
    for (int index = 0; index < MAP_CAPACITY; index++)
    {
        ProcessNode *currentNode = processMap[index];
        while (currentNode != NULL)
        {
            ProcessNode *next = currentNode->mapChain;
            free(currentNode);
            currentNode = next;
        }
        processMap[index] = NULL;
    }
    KillEventNode *killEvent = killEventHead;
    while (killEvent != NULL)
    {
        KillEventNode *nextEvent = killEvent->next;
        free(killEvent);
        killEvent = nextEvent;
    }
}
