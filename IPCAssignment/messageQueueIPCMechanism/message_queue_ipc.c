#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_ELEMENTS 100
#define MESSAGE_QUEUE_KEY 0x54321
#define MESSAGE_TYPE_PARENT_TO_CHILD 1
#define MESSAGE_TYPE_CHILD_TO_PARENT 2

struct MessageBuffer
{
    long messageType;
    int elementCount;
    int integerArray[MAX_ELEMENTS];
};

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
            printf("Invalid input. Enter a valid integer.\n");
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
    int messageQueueId;
    pid_t processIdentifier;
    struct MessageBuffer messageData;
    int elementCount;

    messageQueueId = msgget(MESSAGE_QUEUE_KEY, 0666 | IPC_CREAT);
    if (messageQueueId < 0)
    {
        perror("Message queue creation failed");
        exit(EXIT_FAILURE);
    }

    processIdentifier = fork();

    if (processIdentifier < 0)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (processIdentifier > 0)
    {
        elementCount = readPositiveInteger("Enter number of elements: ");

        if (elementCount > MAX_ELEMENTS)
        {
            printf("Element count exceeds allowed limit.\n");
            msgctl(messageQueueId, IPC_RMID, NULL);
            exit(EXIT_FAILURE);
        }

        readIntegerArrayFromUser(messageData.integerArray, elementCount);

        messageData.messageType = MESSAGE_TYPE_PARENT_TO_CHILD;
        messageData.elementCount = elementCount;

        msgsnd(
            messageQueueId,
            &messageData,
            sizeof(messageData) - sizeof(long),
            0
        );

        displayIntegerArray(
            "Array Before Sorting",
            messageData.integerArray,
            elementCount
        );

        msgrcv(
            messageQueueId,
            &messageData,
            sizeof(messageData) - sizeof(long),
            MESSAGE_TYPE_CHILD_TO_PARENT,
            0
        );

        displayIntegerArray(
            "Array After Sorting",
            messageData.integerArray,
            elementCount
        );

        wait(NULL);
        msgctl(messageQueueId, IPC_RMID, NULL);
    }
    else
    {
        msgrcv(
            messageQueueId,
            &messageData,
            sizeof(messageData) - sizeof(long),
            MESSAGE_TYPE_PARENT_TO_CHILD,
            0
        );

        sortArrayAscending(
            messageData.integerArray,
            messageData.elementCount
        );

        messageData.messageType = MESSAGE_TYPE_CHILD_TO_PARENT;

        msgsnd(
            messageQueueId,
            &messageData,
            sizeof(messageData) - sizeof(long),
            0
        );

        exit(EXIT_SUCCESS);
    }

    return 0;
}
