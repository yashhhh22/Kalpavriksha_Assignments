#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SIZE 1000

int main() {
    char inputExpression[SIZE];
    int operandStack[SIZE], operandTop = -1;
    char operatorStack[SIZE];
    int operatorTop = -1;

    printf("Enter expression: ");
    if (!fgets(inputExpression, SIZE, stdin)) {
        printf("Error: Invalid input\n");
        return 0;
    }

    // We have to first skip the leading spaces
    int i = 0;
    while (inputExpression[i] == ' ') {
        i++;
    }

    if (inputExpression[i] == '\0' || inputExpression[i] == '\n') {
        printf("Error: Empty expression\n");
        return 0;
    }

    int expectNumber = 1;  // we have to track if the next character we are expecting is a number or an operator

    while (inputExpression[i] != '\0' && inputExpression[i] != '\n') {
        if (isspace((unsigned char)inputExpression[i])) {
            i++;
            continue;
        }

        int sign = 1; // for unary operators
        if (expectNumber && (inputExpression[i] == '+' || inputExpression[i] == '-')) {
            if (inputExpression[i] == '-') {
                sign = -1;
            }
            i++;
            while (inputExpression[i] == ' ') {
                i++; // we have to skip spaces after unary operator
            }
        }

        if (isdigit((unsigned char)inputExpression[i])) {
            long value = 0;
            while (isdigit(inputExpression[i]) || inputExpression[i] == ' ') {
                if (isdigit(inputExpression[i])) {
                    value = value * 10 + (inputExpression[i] - '0');
                }
                i++;
            }
            operandStack[++operandTop] = sign * value;
            expectNumber = 0; // after the number we expect an operator
        }
        else if (!expectNumber && (inputExpression[i] == '+' || inputExpression[i] == '-' || inputExpression[i] == '*' || inputExpression[i] == '/')) {
            operatorStack[++operatorTop] = inputExpression[i];
            i++;
            expectNumber = 1; // after operator, expect number
        }
        else {
            printf("Error: Invalid expression\n");
            return 0;
        }
    }

    // We have to handle * and / first according to the precedence rule
    int newOperandStack[SIZE], k = 0;
    char newOperatorStack[SIZE];
    newOperandStack[k++] = operandStack[0];
    for (int j = 0; j <= operatorTop; j++) {
        if (operatorStack[j] == '*') {
            newOperandStack[k-1] *= operandStack[j+1];
        } else if (operatorStack[j] == '/') {
            if (operandStack[j+1] == 0) {
                printf("Error: Division by zero\n");
                return 0;
            }
            newOperandStack[k-1] /= operandStack[j+1];
        } else {
            newOperatorStack[k-1] = operatorStack[j];
            newOperandStack[k++] = operandStack[j+1];
        }
    }

    int evaluatedValue = newOperandStack[0];
    for (int j = 0; j < k-1; j++) {
        if (newOperatorStack[j] == '+') {
            evaluatedValue += newOperandStack[j+1];
        } else {
            evaluatedValue -= newOperandStack[j+1];
        }
    }

    printf("%d\n", evaluatedValue);
    return 0;
}
