#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_STUDENTS 100
#define TOTAL_SUBJECTS 3

struct Student {
    int rollNo;
    char name[50];
    int marks[TOTAL_SUBJECTS];
};

int total(struct Student student_detail) {
    int totalMarks = 0;
    for(int subjectIndex = 0; subjectIndex < TOTAL_SUBJECTS; subjectIndex++) {
        totalMarks += student_detail.marks[subjectIndex];
    }
    return totalMarks;
}

float averageOfMarks(struct Student student_detail) {
    return total(student_detail) / (float)TOTAL_SUBJECTS;
}

char gradeOfStudent(float average) {
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
    printf("\n");
}

void rollNumbers(struct Student details[], int numberOfStudents, int index) {
    if(index == numberOfStudents) {
        return;
    }

    printf("%d ", details[index].rollNo);
    rollNumbers(details, numberOfStudents, index+1);
}

bool isUniqueRoll(int roll, struct Student details[], int index) {
    for(int studentIndex = 0; studentIndex < index; studentIndex++) {
        if(details[studentIndex].rollNo == roll) return false;
    }
    return true;
}

bool isNameValid(const char *name) {
    for(int i = 0; name[i]; i++) {
        if(!isalpha(name[i]) && name[i] != ' ') return false;
    }
    return true;
}

bool isMarksValid(int *marks) {
    for(int i=0; i<TOTAL_SUBJECTS; i++) {
        if(marks[i] < 0 || marks[i] > 100) {
            return false;
        }
    }
    return true;
}

int main() {
    int numberOfStudents;
    printf("Enter the number of students: ");
    scanf("%d", &numberOfStudents);
    getchar();
    
    struct Student details[MAX_STUDENTS];

    // add the details of the students
    printf("\nEnter the details of students (rollNo name marks1 marks2 marks3):\n");
    for(int studentIndex = 0; studentIndex < numberOfStudents; studentIndex++) {
        char line[150];
        int valid = 0;

        while (!valid) {
            fgets(line, sizeof(line), stdin); 
            line[strcspn(line, "\n")] = 0;

            int roll;
            char name[50];
            int marks[TOTAL_SUBJECTS];

            if(sscanf(line, "%d %49[^0-9\n] %d %d %d", &roll, name, &marks[0], &marks[1], &marks[2]) != 5) {
                printf("Invalid input! Enter again:\n");
                continue;
            }

            if(roll < 1 || roll > 100) {
                printf("Roll number must be between 1 and 100. Enter again:\n");
                continue;
            }

            if(!isUniqueRoll(roll, details, studentIndex)) {
                printf("Roll number must be unique. Enter again:\n");
                continue;
            }

            if(!isNameValid(name)) {
                printf("Name can only contain letters and spaces. Enter again:\n");
                continue;
            }

            if(!isMarksValid(marks)) {
                printf("Marks must be between 0 and 100. Enter again:\n");
                continue;
            }

            // input is correct
            details[studentIndex].rollNo = roll;
            strcpy(details[studentIndex].name, name);
            for(int subjectIndex = 0; subjectIndex < TOTAL_SUBJECTS; subjectIndex++) {
                details[studentIndex].marks[subjectIndex] = marks[subjectIndex];
            }
            valid = 1;
        }
    }

    // print the details of the students
    for(int studentIndex = 0; studentIndex < numberOfStudents; studentIndex++) {
        printf("\nRoll: %d\n", details[studentIndex].rollNo);
        printf("Name: %s\n", details[studentIndex].name);
        printf("Total: %d\n", total(details[studentIndex]));
        float average = averageOfMarks(details[studentIndex]);
        char grade = gradeOfStudent(average);
        printf("Average: %.2f\n", average);
        printf("Grade: %c\n", grade);
        if(average < 35) {
            continue;
        }
        printf("Performance: ");
        printStars(grade);
    }

    printf("\nList of Roll Numbers (via recursion): ");
    rollNumbers(details, numberOfStudents, 0);
}
