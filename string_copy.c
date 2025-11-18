#include <stdio.h>
#include <stdlib.h>

#define MAXIMUM_STRING_LENGTH 100

int isStringWithSpacesOnly(char *originalString);
void copyString(char *originalString, char *copiedString);

int isStringWithSpacesOnly(char *originalString) {
    char *pointerToString = originalString;

    int hasOnlySpaces = 1;

    while (*pointerToString != '\0') {
        if (*pointerToString != ' ' && *pointerToString != '\t') {
            hasOnlySpaces = 0;
            break;
        }
        pointerToString++;
    }

    return hasOnlySpaces;
}

void copyString(char *originalString, char *copiedString) {
    while (*originalString != '\0') {
        *copiedString = *originalString;

        originalString++;
        copiedString++;
    }

    *copiedString = '\0';
}

int main() {
    char originalString[MAXIMUM_STRING_LENGTH];
    
    printf("Source: ");
    fgets(originalString, sizeof(originalString), stdin);

    char *newLinePointer = originalString;
    while (*newLinePointer != '\0') {
        if (*newLinePointer == '\n') {
            *newLinePointer = '\0';
            break;
        }
        newLinePointer++;
    }

    if (*originalString == '\0') {
        printf("The string is empty, so it can't be copied, please enter something.\n");
        return 0;
    }

    if (isStringWithSpacesOnly(originalString)) {
        printf("The string has only white spaces, so it can't be copied. Please enter a string with some values.\n");
        return 0;
    }

    char copiedString[MAXIMUM_STRING_LENGTH];

    copyString(originalString, copiedString);

    printf("Copied string: %s\n", copiedString);

    return 0;
}
