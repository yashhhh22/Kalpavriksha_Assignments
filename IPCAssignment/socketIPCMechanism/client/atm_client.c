#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>


#define SERVER_PORT 9090
#define BUFFER_SIZE 256

int readValidatedMenuChoice()
{
    char inputBuffer[BUFFER_SIZE];
    char *conversionEnd;
    long convertedValue;

    while (1)
    {
        printf("\n1. Withdraw\n2. Deposit\n3. Display Balance\n4. Exit\nEnter choice: ");
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
            printf("Invalid choice.\n");
            continue;
        }

        if (convertedValue < 1 || convertedValue > 4)
        {
            printf("Choice out of range.\n");
            continue;
        }

        return (int) convertedValue;
    }
}

long readValidatedAmount()
{
    char inputBuffer[BUFFER_SIZE];
    char *conversionEnd;
    long convertedValue;

    while (1)
    {
        printf("Enter amount: ");
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

        if (convertedValue <= 0 || convertedValue > INT_MAX)
        {
            printf("Invalid amount.\n");
            continue;
        }

        return convertedValue;
    }
}

int main()
{
    while (1)
    {
        int selectedOption = readValidatedMenuChoice();

        if (selectedOption == 4)
        {
            break;
        }

        long amountValue = 0;
        if (selectedOption == 1 || selectedOption == 2)
        {
            amountValue = readValidatedAmount();
        }

        int clientSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocketDescriptor < 0)
        {
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in serverAddress;
        memset(&serverAddress, 0, sizeof(serverAddress));

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

        if (connect(clientSocketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        {
            close(clientSocketDescriptor);
            exit(EXIT_FAILURE);
        }

        char requestBuffer[BUFFER_SIZE];
        snprintf(requestBuffer, sizeof(requestBuffer), "%d %ld", selectedOption, amountValue);
        send(clientSocketDescriptor, requestBuffer, strlen(requestBuffer), 0);

        char responseBuffer[BUFFER_SIZE];
        ssize_t receivedBytes = recv(clientSocketDescriptor, responseBuffer, sizeof(responseBuffer) - 1, 0);

        if (receivedBytes > 0)
        {
            responseBuffer[receivedBytes] = '\0';
            printf("\n%s", responseBuffer);
        }

        close(clientSocketDescriptor);
    }

    return 0;
}
