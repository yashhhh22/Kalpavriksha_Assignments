#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SIZE 1000

int main() {
    char expression[SIZE];
    int numbers[SIZE], numTop = -1;
    char operators[SIZE];
    int opTop = -1;

    printf("Enter expression: ");
    if (!fgets(expression, SIZE, stdin)) {
        printf("Error: Invalid input\n");
        return 0;
    }

    // We have to first skip the leading spaces
    int i = 0;
    while (expression[i] == ' ') {
        i++;
    }

    if (expression[i] == '\0' || expression[i] == '\n') {
        printf("Error: Empty expression\n");
        return 0;
    }

    int expectNumber = 1;  // we have to track if the next character we are expecting is a number or an operator

    while (expression[i] != '\0' && expression[i] != '\n') {
        if (isspace(expression[i])) {
            i++;
            continue;
        }

        int sign = 1; // for unary operators
        if (expectNumber && (expression[i] == '+' || expression[i] == '-')) {
            if (expression[i] == '-') {
                sign = -1;
            }
            i++;
            while (expression[i] == ' ') {
                i++; // we have to skip spaces after unary operator
            }
        }

        if (isdigit(expression[i])) {
            long value = 0;
            while (isdigit(expression[i]) || expression[i] == ' ') {
                if (isdigit(expression[i])) {
                    value = value * 10 + (expression[i] - '0');
                }
                i++;
            }
            numbers[++numTop] = sign * value;
            expectNumber = 0; // after the number we expect an operator
        }
        else if (!expectNumber && (expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/')) {
            operators[++opTop] = expression[i];
            i++;
            expectNumber = 1; // after operator, expect number
        }
        else {
            printf("Error: Invalid expression\n");
            return 0;
        }
    }

    // We have to handle * and / first according to the precedence rule
    int newNums[SIZE], k = 0;
    char newOps[SIZE];
    newNums[k++] = numbers[0];
    for (int j = 0; j <= opTop; j++) {
        if (operators[j] == '*') {
            newNums[k-1] *= numbers[j+1];
        } else if (operators[j] == '/') {
            if (numbers[j+1] == 0) {
                printf("Error: Division by zero\n");
                return 0;
            }
            newNums[k-1] /= numbers[j+1];
        } else {
            newOps[k-1] = operators[j];
            newNums[k++] = numbers[j+1];
        }
    }

    int evaluatedValue = newNums[0];
    for (int j = 0; j < k-1; j++) {
        if (newOps[j] == '+') {
            evaluatedValue += newNums[j+1];
        } else {
            evaluatedValue -= newNums[j+1];
        }
    }

    printf("%d\n", evaluatedValue);
    return 0;
}
