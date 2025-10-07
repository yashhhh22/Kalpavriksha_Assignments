#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SIZE 1000

int main() {
    char expr[SIZE];
    int nums[SIZE], nTop = -1;
    char ops[SIZE]; int oTop = -1;
    int i = 0;

    printf("Enter expression: ");
    if (!fgets(expr, SIZE, stdin)) {
        printf("Error: Invalid input\n");
        return 0;
    }

    while (expr[i] != '\0' && expr[i] != '\n') {
        // skip spaces
        if (expr[i] == ' ') {
            i++;
            continue;
        }

        // read number
        if (isdigit(expr[i])) {
            int val = 0;
            while (isdigit(expr[i])) {
                val = val * 10 + (expr[i] - '0');
                i++;
            }
            nums[++nTop] = val;
        }
        // read operator
        else if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/') {
            ops[++oTop] = expr[i];
            i++;
        }
        else {
            printf("Error: Invalid expression\n");
            return 0;
        }
    }

    // first handle * and /
    int newNums[SIZE], newOps[SIZE], k = 0;
    newNums[k++] = nums[0];
    for (int j = 0; j <= oTop; j++) {
        if (ops[j] == '*') {
            newNums[k-1] = newNums[k-1] * nums[j+1];
        } else if (ops[j] == '/') {
            if (nums[j+1] == 0) {
                printf("Error: Division by zero\n");
                return 0;
            }
            newNums[k-1] = newNums[k-1] / nums[j+1];
        } else {
            newOps[k-1] = ops[j];
            newNums[k++] = nums[j+1];
        }
    }

    // then handle + and -
    int result = newNums[0];
    for (int j = 0; j < k-1; j++) {
        if (newOps[j] == '+') result += newNums[j+1];
        else result -= newNums[j+1];
    }

    printf("%d\n", result);
    return 0;
}
