#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
}

void rollNumbers(struct Student details[], int N, int index) {
    if(index == N) {
        return;
    }

    printf("%d ", details[index].rollNo);
    rollNumbers(details, N, index+1);
}

int main() {
    int N;
    printf("Enter the number of students: ");
    scanf("%d", &N);

    struct Student details[MAX_STUDENTS];

    // add the details of the students
    printf("\nEnter the details of students:\n");
    for(int i=0; i<N; i++) {
        char line[100];

        fgets(line, sizeof(line), stdin); 
        
        line[strcspn(line, "\n")] = 0;

        sscanf(line, "%d %49s %d %d %d", 
            &details[i].rollNo,
            details[i].name,
            &details[i].marks1,
            &details[i].marks2,
            &details[i].marks3
        );
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

    printf("\nList of Roll Numbers (via recursion): ");
    rollNumbers(details, N, 0);
}

