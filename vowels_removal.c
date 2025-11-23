#include <stdio.h>
#include <stdlib.h>

#define MAXIMUM_STRING_LENGTH 100

int isStringWithSpacesOnly(char *inputString);
int isVowel(char ch);
void removeVowels(char *inputString);

int isStringWithSpacesOnly(char *inputString) {
    char *pointerToString = inputString;

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

int isVowel(char ch) {
    if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' ||
        ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U'
    ) {
        return 1;
    }

    return 0;
}

void removeVowels(char *inputString) {
    char *readPointer = inputString;
    char *writePointer = inputString;

    while (*readPointer != '\0') {
        if (!isVowel(*readPointer)) {
            *writePointer = *readPointer;
            writePointer++;
        }
        readPointer++;
    }

    *writePointer = '\0';
}

int main() {
    char inputString[MAXIMUM_STRING_LENGTH];

    printf("Enter a string: ");
    fgets(inputString, sizeof(inputString), stdin);

    char *newLinePointer = inputString;
    while (*newLinePointer != '\0') {
        if (*newLinePointer == '\n') {
            *newLinePointer = '\0';
            break;
        }
        newLinePointer++;
    }

    if (*inputString == '\0') {
        printf("The string is empty. Please enter a string with certain values.\n");
        return 0;
    }

    if (isStringWithSpacesOnly(inputString)) {
        printf("The string contains only whitespace. Please enter a string with some values.\n");
        return 0;
    }

    removeVowels(inputString);

    printf("String after removing vowels: %s\n", inputString);

    return 0;
}
