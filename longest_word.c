#include <stdio.h>
#include <stdlib.h>

#define MAXIMUM_WORD_LENGTH 50

void readWords(char **arrayOfWords, int numberOfWords);
int getWordLength(char *word);
char *findLongestWord(char **arrayOfWords, int numberOfWords);

void readWords(char **arrayOfWords, int numberOfWords) {
    for(int wordsIndex = 0; wordsIndex < numberOfWords; wordsIndex++) {
        scanf("%50s", *(arrayOfWords + wordsIndex));
    }
}

int getWordLength(char *word) {
    char *pointerToWord = word;
    int lengthOfWord = 0;

    while (*pointerToWord != '\0') {
        lengthOfWord++;
        pointerToWord++;
    }

    return lengthOfWord;
}

char *findLongestWord(char **arrayOfWords, int numberOfWords) {
    char *longestWord = *arrayOfWords;
    int maximumLength = getWordLength(longestWord);

    for (char **pointerToWord = arrayOfWords + 1; pointerToWord < arrayOfWords + numberOfWords; pointerToWord++) {
        int lengthOfCurrentWord = getWordLength(*pointerToWord);

        if (lengthOfCurrentWord > maximumLength) {
            maximumLength = lengthOfCurrentWord;
            longestWord = *pointerToWord; 
        }
    }

    return longestWord;
}

int main() {
    int numberOfWords;

    printf("Enter number of words: ");
    scanf("%d", &numberOfWords);

    if (numberOfWords <= 0) {
        printf("Number of words should be greater than or equal to 1");
        return 0;
    }

    char **arrayOfWords = (char **)malloc(numberOfWords * sizeof(char *));

    for (int wordsIndex = 0; wordsIndex < numberOfWords; wordsIndex++) {
        *(arrayOfWords + wordsIndex) = (char *)malloc((MAXIMUM_WORD_LENGTH + 1) * sizeof(char));
    }

    printf("Enter words: \n");
    readWords(arrayOfWords, numberOfWords);

    char *longestWord = findLongestWord(arrayOfWords, numberOfWords);
    int lengthOfLongestWord = getWordLength(longestWord);

    printf("\nThe longest word is: %s\n", longestWord);
    printf("Length: %d\n", lengthOfLongestWord);

    return 0;
}
