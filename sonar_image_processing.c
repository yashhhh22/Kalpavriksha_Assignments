#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define MINIMUM_MATRIX_SIZE 2
#define MAXIMUM_MATRIX_SIZE 10
#define MAXIMUM_RANDOM_VALUE 256

int getValidMatrixSize();
void generateRandomMatrix(int *matrix, int matrixSize);
void printMatrix(int *matrix, int matrixSize);
void transposeMatrix(int *matrix, int matrixSize);
void reverseRows(int *matrix, int matrixSize);
void rotateMatrix90Clockwise(int *matrix, int matrixSize);
void apply3x3SmoothingFilter(int *matrix, int matrixSize);
void swapValues(int *firstElement, int *secondElement);

int getValidMatrixSize() {
    char input[100];
    int matrixSize, isInputValid;

    while (1) {
        printf("Enter matrix size (2-10): ");
        if (!fgets(input, sizeof(input), stdin)) {
            printf("Error reading input. Please try again.\n");
            continue;
        }

        input[strcspn(input, "\n")] = 0;

        isInputValid = 1;
        for (int inputChar = 0; input[inputChar] != '\0'; inputChar++) {
            if (!isdigit((unsigned char)input[inputChar])) {
                isInputValid = 0;
                break;
            }
        }

        if (!isInputValid || strlen(input) == 0) {
            printf("Invalid input: '%s' is not a number. Please enter an integer between 2 and 10.\n", input);
            continue;
        }

        matrixSize = atoi(input);

        if (matrixSize < MINIMUM_MATRIX_SIZE || matrixSize > MAXIMUM_MATRIX_SIZE) {
            printf("Invalid input: %d is out of range. Please enter a value between %d and %d.\n", matrixSize, MINIMUM_MATRIX_SIZE, MAXIMUM_MATRIX_SIZE);
            continue;
        }

        return matrixSize;
    }
}

void generateRandomMatrix(int *matrix, int matrixSize) {
    srand(time(NULL));
    for (int matrixIndex = 0; matrixIndex < matrixSize * matrixSize; matrixIndex++) {
        *(matrix + matrixIndex) = rand() % MAXIMUM_RANDOM_VALUE;
    }
}

void printMatrix(int *matrix, int matrixSize) {
    for (int row = 0; row < matrixSize; row++) {
        for (int column = 0; column < matrixSize; column++) {
            printf("%4d", *(matrix + row * matrixSize + column));
        }
        printf("\n");
    }
}

void transposeMatrix(int *matrix, int matrixSize) {
    for (int rowIndex = 0; rowIndex < matrixSize; rowIndex++) {
        for (int columnIndex = rowIndex + 1; columnIndex < matrixSize; columnIndex++) {
            int *rowElement = matrix + rowIndex * matrixSize + columnIndex;
            int *columnElement = matrix + columnIndex * matrixSize + rowIndex;

            swapValues(rowElement, columnElement);
        }
    }
}

void reverseRows(int *matrix, int matrixSize) {
    for (int rowIndex = 0; rowIndex < matrixSize; rowIndex++) {
        int *leftElement = matrix + rowIndex * matrixSize;
        int *rightElement = matrix + rowIndex * matrixSize + matrixSize - 1;

        while (leftElement < rightElement) {
            swapValues(leftElement, rightElement);
            leftElement++;
            rightElement--;
        }
    }
}

void rotateMatrix90Clockwise(int *matrix, int matrixSize) {
    transposeMatrix(matrix, matrixSize);
    reverseRows(matrix, matrixSize);
}

void apply3x3SmoothingFilter(int *matrix, int matrixSize) {
    int *previousRow = malloc(matrixSize * sizeof(int));
    int *smoothedRow = malloc(matrixSize * sizeof(int)); 

    for (int column = 0; column < matrixSize; column++) {
        *(previousRow + column) = *(matrix + column);
    }

    for (int row = 0; row < matrixSize; row++) {
        int *currentRowOriginal = malloc(matrixSize * sizeof(int));

        for (int column = 0; column < matrixSize; column++) {
            *(currentRowOriginal + column) = *(matrix + row * matrixSize + column);
        }

        for (int column = 0; column < matrixSize; column++) {
            int totalSum = 0, numberOfElements = 0;

            for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
                int neighborRow = row + rowOffset;

                if (neighborRow < 0 || neighborRow >= matrixSize) {
                    continue;
                }

                for (int columnOffset = -1; columnOffset <= 1; columnOffset++) {
                    int neighborColumn = column + columnOffset;

                    if (neighborColumn < 0 || neighborColumn >= matrixSize) {
                        continue;
                    }

                    int neighborValue;
                    if (neighborRow == row - 1) {
                        neighborValue = *(previousRow + neighborColumn);
                    }
                    else if (neighborRow == row) {
                        neighborValue = *(currentRowOriginal + neighborColumn);
                    }
                    else {
                        neighborValue = *(matrix + neighborRow * matrixSize + neighborColumn);
                    }

                    totalSum += neighborValue;
                    numberOfElements++;
                }
            }

            *(smoothedRow + column) = totalSum / numberOfElements;
        }

        for (int column = 0; column < matrixSize; column++) {
            *(matrix + row * matrixSize + column) = *(smoothedRow + column);
        }

        for (int column = 0; column < matrixSize; column++) {
            *(previousRow + column) = *(currentRowOriginal + column);
        }

        free(currentRowOriginal);
    }

    free(previousRow);
    free(smoothedRow);
}

void swapValues(int *firstElement, int *secondElement) {
    int swappingValue = *firstElement; 
    *firstElement = *secondElement;
    *secondElement = swappingValue;
}

int main() {
    int matrixSize = getValidMatrixSize();

    int *matrix = malloc(matrixSize * matrixSize * sizeof(int));

    generateRandomMatrix(matrix, matrixSize);
    printf("\nOriginal Matrix:\n");
    printMatrix(matrix, matrixSize);

    rotateMatrix90Clockwise(matrix, matrixSize);
    printf("\nMatrix after 90 degree Rotation:\n");
    printMatrix(matrix, matrixSize);

    apply3x3SmoothingFilter(matrix, matrixSize);
    printf("\nMatrix after 3x3 Smoothing Filter:\n");
    printMatrix(matrix, matrixSize);

    free(matrix);
    return 0;
}
