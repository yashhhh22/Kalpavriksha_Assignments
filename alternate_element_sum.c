#include <stdio.h>
#include <stdlib.h>

int isInputValid(int numberOfElements);
int sumOfAlternateElements(int integerArray[], int numberOfElements);

int isInputValid(int numberOfElements) {
    int isValid = 1;

    if (numberOfElements < 0) {
        isValid = 0;
    }

    return isValid;
}

int sumOfAlternateElements(int integerArray[], int numberOfElements) {
    int alternateSum = 0;
    int *pointerToArray = integerArray;

    while (pointerToArray < integerArray + numberOfElements) {
        alternateSum += *pointerToArray;
        pointerToArray += 2;
    }

    return alternateSum;
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
        printf("The array is empty, so there is no alternate sum of the array");
        return 0;
    }

    int integerArray[numberOfElements];

    printf("Enter elements: ");
    for (int arrayIndex = 0; arrayIndex < numberOfElements; arrayIndex++) {
        scanf("%d", integerArray + arrayIndex);
    }

    printf("Sum of alternate elements = %d\n", sumOfAlternateElements(integerArray, numberOfElements));

    return 0;
}
