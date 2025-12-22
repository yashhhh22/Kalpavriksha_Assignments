#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define ATM_SERVER_PORT 8080
#define BUFFER_CAPACITY 1024
#define ACCOUNT_DATA_FILE "../resource/accountDB.txt"

#define OPERATION_WITHDRAW 1
#define OPERATION_DEPOSIT 2
#define OPERATION_CHECK_BALANCE 3
#define OPERATION_EXIT 4

#define RESULT_SUCCESS 1
#define DEFAULT_BALANCE 0.00

pthread_mutex_t accountMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct ClientSessionContext
{
    int clientSocketDescriptor;
    int sessionId;
} ClientSessionContext;

float readAccountBalance();
void writeAccountBalance(float updatedBalance);
void handleWithdrawRequest(int clientSocket, float amount);
void handleDepositRequest(int clientSocket, float amount);
void handleBalanceRequest(int clientSocket);
int processClientCommand(int clientSocket, char *requestBuffer);
void *clientSessionHandler(void *threadArgument);
int createListeningSocket();
void launchServer();

int main()
{
    printf("ATM Server initialized.\n");
    launchServer();
    return 0;
}

float readAccountBalance()
{
    FILE *accountFile = fopen(ACCOUNT_DATA_FILE, "r");
    float balanceAmount;

    if (accountFile == NULL)
    {
        printf("Account file not found. Initializing with balance 0.00\n");

        accountFile = fopen(ACCOUNT_DATA_FILE, "w");
        if (accountFile == NULL)
        {
            printf("Failed to create account file.\n");
            return DEFAULT_BALANCE;
        }

        fprintf(accountFile, "0.00");
        fclose(accountFile);
        return DEFAULT_BALANCE;
    }

    fscanf(accountFile, "%f", &balanceAmount);
    fclose(accountFile);

    return balanceAmount;
}

void writeAccountBalance(float updatedBalance)
{
    FILE *accountFile = fopen(ACCOUNT_DATA_FILE, "w");

    if (accountFile == NULL)
    {
        printf("Failed to update account balance.\n");
        return;
    }

    fprintf(accountFile, "%.2f", updatedBalance);
    fclose(accountFile);
}

void handleWithdrawRequest(int clientSocket, float amount)
{
    char responseMessage[BUFFER_CAPACITY];

    pthread_mutex_lock(&accountMutex);
    float currentBalance = readAccountBalance();

    if (amount <= 0)
    {
        snprintf(responseMessage, BUFFER_CAPACITY, "FAILED: Invalid withdrawal amount.");
    }
    else if (amount > currentBalance)
    {
        snprintf(responseMessage, BUFFER_CAPACITY, "FAILED: Insufficient balance. Current Balance: %.2f", currentBalance);
    }
    else
    {
        float newBalance = currentBalance - amount;
        writeAccountBalance(newBalance);

        snprintf(responseMessage, BUFFER_CAPACITY, "SUCCESS: Withdrawn %.2f. Updated Balance: %.2f", amount, newBalance);
    }

    pthread_mutex_unlock(&accountMutex);
    send(clientSocket, responseMessage, strlen(responseMessage), 0);
}

void handleDepositRequest(int clientSocket, float amount)
{
    char responseMessage[BUFFER_CAPACITY];

    pthread_mutex_lock(&accountMutex);

    if (amount <= 0)
    {
        snprintf(responseMessage, BUFFER_CAPACITY, "FAILED: Invalid deposit amount.");
    }
    else
    {
        float currentBalance = readAccountBalance();
        float newBalance = currentBalance + amount;
        writeAccountBalance(newBalance);

        snprintf(responseMessage, BUFFER_CAPACITY, "SUCCESS: Deposited %.2f. Updated Balance: %.2f", amount, newBalance);
    }

    pthread_mutex_unlock(&accountMutex);
    send(clientSocket, responseMessage, strlen(responseMessage), 0);
}

void handleBalanceRequest(int clientSocket)
{
    char responseMessage[BUFFER_CAPACITY];

    pthread_mutex_lock(&accountMutex);
    float currentBalance = readAccountBalance();
    pthread_mutex_unlock(&accountMutex);

    snprintf(responseMessage, BUFFER_CAPACITY, "Current Balance: %.2f", currentBalance);

    send(clientSocket, responseMessage, strlen(responseMessage), 0);
}

int processClientCommand(int clientSocket, char *requestBuffer)
{
    int operationCode;
    float transactionAmount;

    sscanf(requestBuffer, "%d %f", &operationCode, &transactionAmount);

    switch (operationCode)
    {
        case OPERATION_WITHDRAW:
            handleWithdrawRequest(clientSocket, transactionAmount);
            break;

        case OPERATION_DEPOSIT:
            handleDepositRequest(clientSocket, transactionAmount);
            break;

        case OPERATION_CHECK_BALANCE:
            handleBalanceRequest(clientSocket);
            break;

        case OPERATION_EXIT:
            return 0;

        default:
            send(clientSocket, "Invalid operation", 17, 0);
    }

    return RESULT_SUCCESS;
}

void *clientSessionHandler(void *threadArgument)
{
    ClientSessionContext *sessionContext = (ClientSessionContext *)threadArgument;
    char requestBuffer[BUFFER_CAPACITY];

    while (1)
    {
        memset(requestBuffer, 0, BUFFER_CAPACITY);

        int receivedBytes = recv(sessionContext->clientSocketDescriptor, requestBuffer, BUFFER_CAPACITY, 0);

        if (receivedBytes <= 0)
        {
            break;
        }

        if (!processClientCommand(sessionContext->clientSocketDescriptor, requestBuffer))
        {
            break;
        }
    }

    close(sessionContext->clientSocketDescriptor);
    free(sessionContext);
    pthread_exit(NULL);
}

int createListeningSocket()
{
    int listeningSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (listeningSocket < 0)
    {
        return -1;
    }

    int reuseAddress = 1;
    setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress));

    return listeningSocket;
}

void launchServer()
{
    int serverSocket = createListeningSocket();

    if (serverSocket < 0)
    {
        return;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(ATM_SERVER_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        return;
    }

    if (listen(serverSocket, 5) < 0)
    {
        return;
    }

    printf("ATM server listening on port %d...\n", ATM_SERVER_PORT);

    int activeClientCount = 0;

    while (1)
    {
        struct sockaddr_in clientAddress;
        socklen_t addressLength = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &addressLength);

        if (clientSocket < 0)
        {
            continue;
        }

        activeClientCount++;

        ClientSessionContext *sessionContext = (ClientSessionContext *)malloc(sizeof(ClientSessionContext));

        sessionContext->clientSocketDescriptor = clientSocket;
        sessionContext->sessionId = activeClientCount;

        pthread_t clientThreadId;

        if (pthread_create(&clientThreadId, NULL, clientSessionHandler, sessionContext) != 0)
        {
            close(clientSocket);
            free(sessionContext);
        }
        else
        {
            pthread_detach(clientThreadId);
        }
    }

    close(serverSocket);
}
