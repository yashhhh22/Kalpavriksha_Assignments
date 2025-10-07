#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SIZE 1000

int main() {
    char expression[SIZE];
    int numbers[SIZE], digitsTop = -1;
    char operators[SIZE]; int operatorTop = -1;
    int i = 0;

    printf("Enter expression: ");
    if (!fgets(expression, SIZE, stdin)) {
        printf("Error: Invalid input\n");
        return 0;
    }

    while (expression[i] != '\0' && expression[i] != '\n') {
        // if there are any spaces, skip it
        if (expression[i] == ' ') {
            i++;
            continue;
        }

        // if you read a digit, read the number fully
        if (isdigit(expression[i])) {
            int value = 0;
            while (isdigit(expression[i])) {
                value = value * 10 + (expression[i] - '0');
                i++;
            }
            numbers[++digitsTop] = value;
        }
        // read the operator
        else if (expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/') {
            operators[++operatorTop] = expression[i];
            i++;
        }
        else {
            printf("Error: Invalid expression\n");
            return 0;
        }
    }

    // first we have to handle / & * according to precedence
    int newNums[SIZE], k = 0;
    char newOperators[SIZE];
    newNums[k++] = numbers[0];
    for (int j = 0; j <= operatorTop; j++) {
        if (operators[j] == '*') {
            newNums[k-1] = newNums[k-1] * numbers[j+1];
        } else if (operators[j] == '/') {
            if (numbers[j+1] == 0) {
                printf("Error: Division by zero\n");
                return 0;
            }
            newNums[k-1] = newNums[k-1] / numbers[j+1];
        } else {
            newOperators[k-1] = operators[j];
            newNums[k++] = numbers[j+1];
        }
    }

    // then we have to handle + & -
    int evaluatedValue = newNums[0];
    for (int j = 0; j < k-1; j++) {
        if (newOperators[j] == '+') {
            evaluatedValue += newNums[j+1];
        } else {
            evaluatedValue -= newNums[j+1];
        }
    }

    printf("%d\n", evaluatedValue);
    return 0;
}

