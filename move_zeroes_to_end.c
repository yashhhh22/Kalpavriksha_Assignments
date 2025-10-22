#include <stdio.h>
#include <stdlib.h>

int isInputValid(int numberOfElements);
void swapValues(int *leftElement, int *rightElement);
void moveZeroesToEnd(int inputArray[], int numberOfElements);
void printArray(int inputArray[], int numberOfElements);

int isInputValid(int numberOfElements) {
    int isValid = 1;

    if (numberOfElements < 0) {
        isValid = 0;
    }

    return isValid;
}

void swapValues(int *leftElement, int *rightElement) {
    int swappingValue = *leftElement;
    *leftElement = *rightElement;
    *rightElement = swappingValue;
}

void moveZeroesToEnd(int inputArray[], int numberOfElements) {
    int *pointerToCurrentElement = inputArray;
    int *pointerToLastNonZero = inputArray;

    while (pointerToCurrentElement < inputArray + numberOfElements) {
        if (*pointerToCurrentElement != 0) {
            swapValues(pointerToCurrentElement, pointerToLastNonZero);

            pointerToLastNonZero++;
        }

        pointerToCurrentElement++;
    }
}

void printArray(int inputArray[], int numberOfElements) {
    for (int arrayIndex = 0; arrayIndex < numberOfElements; arrayIndex++) {
        printf("%d ", *(inputArray + arrayIndex));
    }

    printf("\n");
}

int main() {
    int numberOfElements;
    printf("Enter number of elements: ");
    scanf("%d", &numberOfElements);

    if (!isInputValid(numberOfElements)) {
        printf("ERROR: Number of elements cannot be a negative value. Enter a non-negative integer(0 or greater).\n");
        return 0;
    }

    if (numberOfElements == 0) {
        printf("The array is empty, so there is no output");
        return 0;
    }

    int inputArray[numberOfElements];

    printf("Enter array elements: ");
    for (int arrayIndex = 0; arrayIndex < numberOfElements; arrayIndex++) {
        scanf("%d", inputArray + arrayIndex);
    }

    moveZeroesToEnd(inputArray, numberOfElements);

    printf("Array after moving zeroes: ");
    printArray(inputArray, numberOfElements);

    return 0;
}
