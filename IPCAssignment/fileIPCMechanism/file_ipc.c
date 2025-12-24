#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_ELEMENTS 100
#define IPC_FILENAME "ipc_data.txt"

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

void writeArrayToFile(const char *fileName, const int arrayData[], int elementCount)
{
    FILE *filePointer;
    int currentIndex;

    filePointer = fopen(fileName, "w");

    if (filePointer == NULL)
    {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    fprintf(filePointer, "%d\n", elementCount);

    for (currentIndex = 0; currentIndex < elementCount; currentIndex++)
    {
        fprintf(filePointer, "%d ", arrayData[currentIndex]);
    }

    fclose(filePointer);
}

int readArrayFromFile(const char *fileName, int outputArray[])
{
    FILE *filePointer;
    int elementCount;
    int currentIndex;

    filePointer = fopen(fileName, "r");

    if (filePointer == NULL)
    {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    fscanf(filePointer, "%d", &elementCount);

    for (currentIndex = 0; currentIndex < elementCount; currentIndex++)
    {
        fscanf(filePointer, "%d", &outputArray[currentIndex]);
    }

    fclose(filePointer);

    return elementCount;
}

void waitUntilFileExists(const char *fileName)
{
    struct stat fileStatus;

    while (stat(fileName, &fileStatus) != 0)
    {
        usleep(100000);
    }
}

void executeParentProcess(void)
{
    int inputArray[MAX_ELEMENTS];
    int elementCount;

    elementCount = readPositiveInteger("Enter number of elements: ");

    if (elementCount > MAX_ELEMENTS)
    {
        printf("Element count exceeds allowed limit.\n");
        exit(EXIT_FAILURE);
    }

    readIntegerArrayFromUser(inputArray, elementCount);

    displayIntegerArray("Array Before Sorting", inputArray, elementCount);

    writeArrayToFile(IPC_FILENAME, inputArray, elementCount);

    wait(NULL);

    elementCount = readArrayFromFile(IPC_FILENAME, inputArray);

    displayIntegerArray("Array After Sorting", inputArray, elementCount);
}

void executeChildProcess(void)
{
    int receivedArray[MAX_ELEMENTS];
    int elementCount;

    waitUntilFileExists(IPC_FILENAME);

    elementCount = readArrayFromFile(IPC_FILENAME, receivedArray);

    sortArrayAscending(receivedArray, elementCount);

    writeArrayToFile(IPC_FILENAME, receivedArray, elementCount);
}

int main(void)
{
    pid_t processIdentifier;

    processIdentifier = fork();

    if (processIdentifier < 0)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    else if (processIdentifier > 0)
    {
        executeParentProcess();
    }
    else
    {
        executeChildProcess();
    }

    return 0;
}
