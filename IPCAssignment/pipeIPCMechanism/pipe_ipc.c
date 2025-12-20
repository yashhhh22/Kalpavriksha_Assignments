#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_ELEMENTS 100

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

void readIntegerArray(int inputArray[], int elementCount)
{
    int currentIndex;

    for (currentIndex = 0; currentIndex < elementCount; currentIndex++)
    {
        inputArray[currentIndex] = readStrictInteger("Enter element: ");
    }
}

void displayArray(const char *titleMessage, const int arrayData[], int elementCount)
{
    int currentIndex;

    printf("\n%s\n", titleMessage);
    printf("---------------------------------\n");

    for (currentIndex = 0; currentIndex < elementCount; currentIndex++)
    {
        printf("%d ", arrayData[currentIndex]);
    }

    printf("\n---------------------------------\n");
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

void writeAll(int fileDescriptor, const void *buffer, size_t totalBytes)
{
    size_t bytesWritten;
    const char *currentPointer;

    bytesWritten = 0;
    currentPointer = buffer;

    while (bytesWritten < totalBytes)
    {
        ssize_t result;

        result = write(
            fileDescriptor,
            currentPointer + bytesWritten,
            totalBytes - bytesWritten
        );

        if (result <= 0)
        {
            exit(EXIT_FAILURE);
        }

        bytesWritten += result;
    }
}

void readAll(int fileDescriptor, void *buffer, size_t totalBytes)
{
    size_t bytesRead;
    char *currentPointer;

    bytesRead = 0;
    currentPointer = buffer;

    while (bytesRead < totalBytes)
    {
        ssize_t result;

        result = read(
            fileDescriptor,
            currentPointer + bytesRead,
            totalBytes - bytesRead
        );

        if (result <= 0)
        {
            exit(EXIT_FAILURE);
        }

        bytesRead += result;
    }
}

void handleParentProcess(int writePipe[], int readPipe[])
{
    int elementCount;
    int inputArray[MAX_ELEMENTS];

    close(writePipe[0]);
    close(readPipe[1]);

    elementCount = readPositiveInteger("Enter number of elements: ");

    if (elementCount > MAX_ELEMENTS)
    {
        printf("Maximum allowed elements: %d\n", MAX_ELEMENTS);
        exit(EXIT_FAILURE);
    }

    readIntegerArray(inputArray, elementCount);

    displayArray("Array Before Sorting", inputArray, elementCount);

    writeAll(writePipe[1], &elementCount, sizeof(int));
    writeAll(writePipe[1], inputArray, elementCount * sizeof(int));

    readAll(readPipe[0], inputArray, elementCount * sizeof(int));

    displayArray("Array After Sorting", inputArray, elementCount);

    close(writePipe[1]);
    close(readPipe[0]);

    wait(NULL);
}

void handleChildProcess(int readPipe[], int writePipe[])
{
    int elementCount;
    int receivedArray[MAX_ELEMENTS];

    close(readPipe[1]);
    close(writePipe[0]);

    readAll(readPipe[0], &elementCount, sizeof(int));
    readAll(readPipe[0], receivedArray, elementCount * sizeof(int));

    sortArrayAscending(receivedArray, elementCount);

    writeAll(writePipe[1], receivedArray, elementCount * sizeof(int));

    close(readPipe[0]);
    close(writePipe[1]);
}

int main(void)
{
    int parentToChildPipe[2];
    int childToParentPipe[2];
    pid_t processIdentifier;

    if (pipe(parentToChildPipe) == -1 || pipe(childToParentPipe) == -1)
    {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    processIdentifier = fork();

    if (processIdentifier < 0)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    else if (processIdentifier > 0)
    {
        handleParentProcess(parentToChildPipe, childToParentPipe);
    }
    else
    {
        handleChildProcess(parentToChildPipe, childToParentPipe);
    }

    return 0;
}
