#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>

#define SERVER_PORT 9090
#define BUFFER_SIZE 256
#define ACCOUNT_FILE_PATH "../resource/accountDB.txt"

pthread_mutex_t accountFileMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    int clientSocketDescriptor;
} ClientContext;

int readValidatedPositiveInteger(const char *message)
{
    char inputBuffer[BUFFER_SIZE];
    char *conversionEnd;
    long convertedValue;

    while (1)
    {
        printf("%s", message);
        fflush(stdout);

        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL)
        {
            continue;
        }

        inputBuffer[strcspn(inputBuffer, "\n")] = '\0';

        errno = 0;
        convertedValue = strtol(inputBuffer, &conversionEnd, 10);

        if (conversionEnd == inputBuffer || *conversionEnd != '\0')
        {
            printf("Invalid numeric input.\n");
            continue;
        }

        if (errno == ERANGE || convertedValue <= 0 || convertedValue > INT_MAX)
        {
            printf("Invalid numeric range.\n");
            continue;
        }

        return (int) convertedValue;
    }
}

long readAccountBalanceFromFile()
{
    FILE *filePointer = fopen(ACCOUNT_FILE_PATH, "r");
    if (filePointer == NULL)
    {
        return -1;
    }

    long balance;
    fscanf(filePointer, "%ld", &balance);
    fclose(filePointer);

    return balance;
}

int writeAccountBalanceToFile(long updatedBalance)
{
    FILE *filePointer = fopen(ACCOUNT_FILE_PATH, "w");
    if (filePointer == NULL)
    {
        return 0;
    }

    fprintf(filePointer, "%ld", updatedBalance);
    fclose(filePointer);

    return 1;
}

void sendResponseToClient(int socketDescriptor, const char *message)
{
    send(socketDescriptor, message, strlen(message), 0);
}

void processWithdrawRequest(int socketDescriptor, long amount)
{
    pthread_mutex_lock(&accountFileMutex);

    long currentBalance = readAccountBalanceFromFile();

    if (currentBalance < 0)
    {
        sendResponseToClient(socketDescriptor, "Server error while reading balance.\n");
    }
    else if (amount > currentBalance)
    {
        sendResponseToClient(socketDescriptor, "Withdrawal failed. Insufficient balance.\n");
    }
    else
    {
        long updatedBalance = currentBalance - amount;
        writeAccountBalanceToFile(updatedBalance);
        sendResponseToClient(socketDescriptor, "Withdrawal successful.\n");
    }

    pthread_mutex_unlock(&accountFileMutex);
}

void processDepositRequest(int socketDescriptor, long amount)
{
    pthread_mutex_lock(&accountFileMutex);

    long currentBalance = readAccountBalanceFromFile();

    if (currentBalance < 0)
    {
        sendResponseToClient(socketDescriptor, "Server error while reading balance.\n");
    }
    else
    {
        long updatedBalance = currentBalance + amount;
        writeAccountBalanceToFile(updatedBalance);
        sendResponseToClient(socketDescriptor, "Deposit successful.\n");
    }

    pthread_mutex_unlock(&accountFileMutex);
}

void processBalanceRequest(int socketDescriptor)
{
    pthread_mutex_lock(&accountFileMutex);

    long currentBalance = readAccountBalanceFromFile();

    if (currentBalance < 0)
    {
        sendResponseToClient(socketDescriptor, "Server error while reading balance.\n");
    }
    else
    {
        char responseBuffer[BUFFER_SIZE];
        snprintf(responseBuffer, sizeof(responseBuffer), "Current balance: %ld\n", currentBalance);
        sendResponseToClient(socketDescriptor, responseBuffer);
    }

    pthread_mutex_unlock(&accountFileMutex);
}

void *handleClientRequest(void *argument)
{
    ClientContext *clientContext = (ClientContext *) argument;
    int clientSocket = clientContext->clientSocketDescriptor;
    free(clientContext);

    char requestBuffer[BUFFER_SIZE];
    ssize_t receivedBytes = recv(clientSocket, requestBuffer, sizeof(requestBuffer) - 1, 0);

    if (receivedBytes <= 0)
    {
        close(clientSocket);
        return NULL;
    }

    requestBuffer[receivedBytes] = '\0';

    int operationCode;
    long transactionAmount;

    if (sscanf(requestBuffer, "%d %ld", &operationCode, &transactionAmount) < 1)
    {
        sendResponseToClient(clientSocket, "Invalid request format.\n");
        close(clientSocket);
        return NULL;
    }

    if (operationCode == 1)
    {
        processWithdrawRequest(clientSocket, transactionAmount);
    }
    else if (operationCode == 2)
    {
        processDepositRequest(clientSocket, transactionAmount);
    }
    else if (operationCode == 3)
    {
        processBalanceRequest(clientSocket);
    }
    else
    {
        sendResponseToClient(clientSocket, "Invalid operation.\n");
    }

    close(clientSocket);
    return NULL;
}

int main()
{
    int serverSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketDescriptor < 0)
    {
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    if (bind(serverSocketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        close(serverSocketDescriptor);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocketDescriptor, 10) < 0)
    {
        close(serverSocketDescriptor);
        exit(EXIT_FAILURE);
    }

    printf("ATM Server running on port %d\n", SERVER_PORT);

    while (1)
    {
        int clientSocketDescriptor = accept(serverSocketDescriptor, NULL, NULL);
        if (clientSocketDescriptor < 0)
        {
            continue;
        }

        ClientContext *context = malloc(sizeof(ClientContext));
        context->clientSocketDescriptor = clientSocketDescriptor;

        pthread_t clientThread;
        pthread_create(&clientThread, NULL, handleClientRequest, context);
        pthread_detach(clientThread);
    }

    close(serverSocketDescriptor);
    return 0;
}
