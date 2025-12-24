#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_ELEMENTS 100
#define SHARED_MEMORY_KEY 0x12345

int readStrictInteger(const char *promptMessage)
{
    char inputBuffer[128];
    char *conversionEnd;
    long convertedValue;

    while (1)
    {
        printf("%s", promptMessage);

        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL)
        {
            exit(EXIT_FAILURE);
        }

        inputBuffer[strcspn(inputBuffer, "\n")] = '\0';

        errno = 0;
        convertedValue = strtol(inputBuffer, &conversionEnd, 10);

        if (conversionEnd == inputBuffer || *conversionEnd != '\0')
        {
            printf("Invalid input. Enter an integer.\n");
            continue;
        }

        if (errno == ERANGE || convertedValue < INT_MIN || convertedValue > INT_MAX)
        {
            printf("Integer value out of range.\n");
            continue;
        }

        return (int) convertedValue;
    }
}

int readPositiveInteger(const char *promptMessage)
{
    int enteredValue;

    while (1)
    {
        enteredValue = readStrictInteger(promptMessage);

        if (enteredValue <= 0)
        {
            printf("Value must be greater than zero.\n");
        }
        else
        {
            return enteredValue;
        }
    }
}

void readIntegerArrayFromUser(int inputArray[], int elementCount)
{
    int currentIndex;

    for (currentIndex = 0; currentIndex < elementCount; currentIndex++)
    {
        inputArray[currentIndex] = readStrictInteger("Enter element: ");
    }
}

void displayIntegerArray(const char *titleMessage, const int arrayData[], int elementCount)
{
    int currentIndex;

    printf("\n%s\n", titleMessage);
    printf("----------------------------------\n");

    for (currentIndex = 0; currentIndex < elementCount; currentIndex++)
    {
        printf("%d ", arrayData[currentIndex]);
    }

    printf("\n----------------------------------\n");
}

void swapIntegers(int *firstValue, int *secondValue)
{
    int temporaryValue;

    temporaryValue = *firstValue;
    *firstValue = *secondValue;
    *secondValue = temporaryValue;
}

void sortArrayAscending(int arrayData[], int elementCount)
{
    int outerIndex;
    int innerIndex;

    for (outerIndex = 0; outerIndex < elementCount - 1; outerIndex++)
    {
        for (innerIndex = 0; innerIndex < elementCount - outerIndex - 1; innerIndex++)
        {
            if (arrayData[innerIndex] > arrayData[innerIndex + 1])
            {
                swapIntegers(
                    &arrayData[innerIndex],
                    &arrayData[innerIndex + 1]
                );
            }
        }
    }
}

int main(void)
{
    int elementCount;
    int sharedMemoryId;
    int *sharedMemoryPointer;
    pid_t processIdentifier;

    elementCount = readPositiveInteger("Enter number of elements: ");

    if (elementCount > MAX_ELEMENTS)
    {
        printf("Element count exceeds allowed limit.\n");
        exit(EXIT_FAILURE);
    }

    sharedMemoryId = shmget(
        SHARED_MEMORY_KEY,
        sizeof(int) * (elementCount + 1),
        0666 | IPC_CREAT
    );

    if (sharedMemoryId < 0)
    {
        perror("Shared memory creation failed");
        exit(EXIT_FAILURE);
    }

    sharedMemoryPointer = (int *) shmat(sharedMemoryId, NULL, 0);
    if (sharedMemoryPointer == (int *) -1)
    {
        perror("Shared memory attachment failed");
        exit(EXIT_FAILURE);
    }

    readIntegerArrayFromUser(&sharedMemoryPointer[1], elementCount);
    sharedMemoryPointer[0] = elementCount;

    displayIntegerArray(
        "Array Before Sorting",
        &sharedMemoryPointer[1],
        elementCount
    );

    processIdentifier = fork();

    if (processIdentifier < 0)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    else if (processIdentifier == 0)
    {
        sortArrayAscending(&sharedMemoryPointer[1], elementCount);
        shmdt(sharedMemoryPointer);
        exit(EXIT_SUCCESS);
    }

    wait(NULL);

    displayIntegerArray(
        "Array After Sorting",
        &sharedMemoryPointer[1],
        elementCount
    );

    shmdt(sharedMemoryPointer);
    shmctl(sharedMemoryId, IPC_RMID, NULL);

    return 0;
}
