#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

# define MAX_STUDENTS 100

struct Student {
    int rollNo;
    char name[50];
    int marks1;
    int marks2;
    int marks3;
};

int total(int marks1, int marks2, int marks3) {
    return marks1 + marks2 + marks3;
}

float average(struct Student s) {
    return total(s.marks1, s.marks2, s.marks3) / 3.0;
}

char grade(float average) {
    if(average >= 85) {
        return 'A';
    } else if(average >= 70) {
        return 'B';
    } else if(average >= 50) {
        return 'C';
    } else if(average >= 35) {
        return 'D';
    } else {
        return 'F';
    }
}

void printStars(char grade) {
    int stars = 0;
    switch (grade) {
        case 'A':
            stars = 5;
            break;
        case 'B':
            stars = 4;
            break;
        case 'C':
            stars = 3;
            break;
        case 'D':
            stars = 2;
            break;
    }

    for(int i=0; i<stars; i++) {
        printf("*");
    }

    printf("\n");
}

void rollNumbers(struct Student details[], int N, int index) {
    if(index == N) {
        return;
    }

    printf("%d ", details[index].rollNo);
    rollNumbers(details, N, index+1);
}

bool isUniqueRoll(int roll, struct Student details[], int index) {
    for(int i=0; i<index; i++) {
        if(details[i].rollNo == roll) return false;
    }
    return true;
}

bool isNameValid(const char *name) {
    for(int i=0; name[i]; i++) {
        if(!isalpha(name[i]) && name[i] != ' ') return false;
    }
    return true;
}

bool isMarksValid(int marks) {
    return marks >= 0 && marks <= 100;
}

int main() {
    int N;
    printf("Enter the number of students: ");
    scanf("%d", &N);
    getchar();
    
    struct Student details[MAX_STUDENTS];

    // add the details of the students
    printf("\nEnter the details of students (rollNo name marks1 marks2 marks3):\n");
    for(int i=0; i<N; i++) {
        char line[150];
        int valid = 0;

        while (!valid) {
            fgets(line, sizeof(line), stdin); 
            line[strcspn(line, "\n")] = 0;

            int roll, m1, m2, m3;
            char name[50];

            if(sscanf(line, "%d %49[^\t\n] %d %d %d", &roll, name, &m1, &m2, &m3) != 5) {
                printf("Invalid input! Enter again:\n");
                continue;
            }

            if(roll < 1 || roll > 100) {
                printf("Roll number must be between 1 and 100. Enter again:\n");
                continue;
            }

            if(!isUniqueRoll(roll, details, i)) {
                printf("Roll number must be unique. Enter again:\n");
                continue;
            }

            if(!isNameValid(name)) {
                printf("Name can only contain letters and spaces. Enter again:\n");
                continue;
            }

            if(!isMarksValid(m1) || !isMarksValid(m2) || !isMarksValid(m3)) {
                printf("Marks must be between 0 and 100. Enter again:\n");
                continue;
            }

            // input is correct
            details[i].rollNo = roll;
            strcpy(details[i].name, name);
            details[i].marks1 = m1;
            details[i].marks2 = m2;
            details[i].marks3 = m3;
            valid = 1;
        }
    }

    // print the details of the students
    for(int i=0; i<N; i++) {
        printf("\n\nRoll: %d\n", details[i].rollNo);
        printf("Name: %s\n", details[i].name);
        printf("Total: %d\n", total(details[i].marks1, details[i].marks2, details[i].marks3));
        float avg = average(details[i]);
        char grd = grade(avg);
        printf("Average: %.2f\n", avg);
        printf("Grade: %c\n", grd);
        if(avg < 35) {
            continue;
        }
        printf("Performance: ");
        printStars(grd);
        
    }

    printf("List of Roll Numbers (via recursion): ");
    rollNumbers(details, N, 0);
}



