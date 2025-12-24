#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define ATM_SERVER_PORT 8080
#define IO_BUFFER_SIZE 1024

#define OPTION_WITHDRAW 1
#define OPTION_DEPOSIT 2
#define OPTION_CHECK_BALANCE 3
#define OPTION_EXIT 4

#define OPTION_MIN 1
#define OPTION_MAX 4

#define STATUS_OK 1
#define STATUS_ERROR 0

void displayAtmOptions();
int readMenuSelection();
bool isValidMenuSelection(int selection);
float readTransactionAmount();
int buildRequestPayload(int selection, char *payload);
int initializeClientSocket();
int establishServerConnection(int socketDescriptor);
void transmitRequest(int socketDescriptor, char *payload);
void receiveServerMessage(int socketDescriptor);
void runClientInteraction(int socketDescriptor);

int main()
{
    int clientSocket = initializeClientSocket();

    if (clientSocket < 0)
    {
        return 0;
    }

    if (establishServerConnection(clientSocket) == STATUS_ERROR)
    {
        close(clientSocket);
        return 0;
    }

    runClientInteraction(clientSocket);

    close(clientSocket);
    return 0;
}

void runClientInteraction(int socketDescriptor)
{
    char requestPayload[IO_BUFFER_SIZE];

    while (1)
    {
        displayAtmOptions();
        int selectedOption = readMenuSelection();

        memset(requestPayload, 0, IO_BUFFER_SIZE);

        if (!buildRequestPayload(selectedOption, requestPayload))
        {
            transmitRequest(socketDescriptor, requestPayload);
            printf("Session terminated.\n");
            break;
        }

        transmitRequest(socketDescriptor, requestPayload);
        receiveServerMessage(socketDescriptor);
    }
}

void displayAtmOptions()
{
    printf("\n-----------------------------\n");
    printf("ATM MENU\n");
    printf("-----------------------------\n");
    printf("1. Withdraw Money\n");
    printf("2. Deposit Money\n");
    printf("3. Check Balance\n");
    printf("4. Exit\n");
    printf("-----------------------------\n");
}

bool isValidMenuSelection(int selection)
{
    return (selection >= OPTION_MIN && selection <= OPTION_MAX);
}

int readMenuSelection()
{
    int selection;

    while (1)
    {
        printf("Enter your choice: ");

        if (scanf("%d", &selection) != 1)
        {
            printf("Invalid input. Please enter a number between 1 and 4.\n");
            while (getchar() != '\n');
            continue;
        }

        if (!isValidMenuSelection(selection))
        {
            printf("Invalid choice. Please select a valid option.\n");
            continue;
        }

        while (getchar() != '\n');
        return selection;
    }
}

float readTransactionAmount()
{
    float amount;

    while (1)
    {
        printf("Enter amount: ");

        if (scanf("%f", &amount) != 1 || amount <= 0)
        {
            printf("Invalid amount. Enter a value greater than zero.\n");
            while (getchar() != '\n');
            continue;
        }

        while (getchar() != '\n');
        return amount;
    }
}

int buildRequestPayload(int selection, char *payload)
{
    float amountValue = 0.0f;

    if (selection == OPTION_WITHDRAW || selection == OPTION_DEPOSIT)
    {
        amountValue = readTransactionAmount();
    }

    sprintf(payload, "%d %.2f", selection, amountValue);

    if (selection == OPTION_EXIT)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

int initializeClientSocket()
{
    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketDescriptor < 0)
    {
        printf("Failed to create socket.\n");
        return STATUS_ERROR;
    }

    return socketDescriptor;
}

int establishServerConnection(int socketDescriptor)
{
    struct sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(ATM_SERVER_PORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Connection to server failed.\n");
        return STATUS_ERROR;
    }

    printf("Connected to ATM server successfully.\n");
    return STATUS_OK;
}

void transmitRequest(int socketDescriptor, char *payload)
{
    send(socketDescriptor, payload, strlen(payload), 0);
}

void receiveServerMessage(int socketDescriptor)
{
    char serverResponse[IO_BUFFER_SIZE];
    memset(serverResponse, 0, IO_BUFFER_SIZE);

    int receivedBytes = recv(socketDescriptor, serverResponse, IO_BUFFER_SIZE, 0);

    if (receivedBytes == 0)
    {
        printf("Server closed the connection.\n");
        return;
    }
    else if (receivedBytes < 0)
    {
        perror("Receive failed");
        return;
    }

    printf("\nServer response: %s\n", serverResponse);
}
