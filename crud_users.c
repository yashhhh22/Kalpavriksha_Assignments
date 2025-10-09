#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "users.txt"

struct User {
    int id;
    char name[50];
    int age;
};

void addUser() {
    struct User currentUser, tempUser;
    FILE *filePtr = fopen(FILE_NAME, "r");

    if (filePtr) {
        printf("Enter ID: ");
        scanf("%d", &currentUser.id);

        while (fscanf(filePtr, "%d %s %d", &tempUser.id, tempUser.name, &tempUser.age) == 3) {
            if (tempUser.id == currentUser.id) {
                printf("Error: User ID already exists.\n");
                fclose(filePtr);
                return;
            }
        }
        fclose(filePtr);
    } else {
        printf("Enter ID: ");
        scanf("%d", &currentUser.id);
    }

    filePtr = fopen(FILE_NAME, "a");
    if (!filePtr) {
        printf("Error: Unable to open file for writing.\n");
        return;
    }

    printf("Enter Name: ");
    scanf("%s", currentUser.name);
    printf("Enter Age: ");
    scanf("%d", &currentUser.age);

    fprintf(filePtr, "%d %s %d\n", currentUser.id, currentUser.name, currentUser.age);
    fclose(filePtr);
    printf("User added successfully!\n");
}

void showUsers() {
    struct User currentUser;
    FILE *filePtr = fopen(FILE_NAME, "r");

    if (!filePtr) {
        printf("No users found.\n");
        return;
    }

    printf("\n--- Users ---\n");
    while (fscanf(filePtr, "%d %s %d", &currentUser.id, currentUser.name, &currentUser.age) == 3) {
        printf("ID: %d | Name: %s | Age: %d\n", currentUser.id, currentUser.name, currentUser.age);
    }
    fclose(filePtr);
}

void updateUser() {
    struct User currentUser;
    int id, found = 0;
    FILE *filePtr = fopen(FILE_NAME, "r");
    FILE *tempFilePtr = fopen("temp.txt", "w");

    if (!filePtr || !tempFilePtr) {
        printf("Error: Unable to open file.\n");
        return;
    }

    printf("Enter ID to update: ");
    scanf("%d", &id);

    while (fscanf(filePtr, "%d %s %d", &currentUser.id, currentUser.name, &currentUser.age) == 3) {
        if (currentUser.id == id) {
            printf("Enter new Name: ");
            scanf("%s", currentUser.name);
            printf("Enter new Age: ");
            scanf("%d", &currentUser.age);
            found = 1;
        }
        fprintf(tempFilePtr, "%d %s %d\n", currentUser.id, currentUser.name, currentUser.age);
    }

    fclose(filePtr);
    fclose(tempFilePtr);

    if (remove(FILE_NAME) != 0) {
        perror("Error deleting old file");
        return;
    }
    if (rename("temp.txt", FILE_NAME) != 0) {
        perror("Error renaming temporary file");
        return;
    }

    if (found) {
        printf("User updated successfully!\n");
    } else {
        printf("User not found.\n");
    }
}

void deleteUser() {
    struct User currentUser;
    int id, found = 0;
    FILE *filePtr = fopen(FILE_NAME, "r");
    FILE *tempFilePtr = fopen("temp.txt", "w");

    if (!filePtr || !tempFilePtr) {
        printf("Error: Unable to open file.\n");
        return;
    }

    printf("Enter ID to delete: ");
    scanf("%d", &id);

    while (fscanf(filePtr, "%d %s %d", &currentUser.id, currentUser.name, &currentUser.age) == 3) {
        if (currentUser.id == id) {
            found = 1;
            continue;
        }
        fprintf(tempFilePtr, "%d %s %d\n", currentUser.id, currentUser.name, currentUser.age);
    }

    fclose(filePtr);
    fclose(tempFilePtr);

    if (remove(FILE_NAME) != 0) {
        perror("Error deleting old file");
        return;
    }
    if (rename("temp.txt", FILE_NAME) != 0) {
        perror("Error renaming temporary file");
        return;
    }

    if (found) {
        printf("User deleted successfully!\n");
    } else {
        printf("User not found.\n");
    }
}

int main() {
    int option;
    while (1) {
        printf("\n--- Menu ---\n");
        printf("1. Add User\n2. Show Users\n3. Update User\n4. Delete User\n5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &option);

        switch (option) {
            case 1: addUser(); break;
            case 2: showUsers(); break;
            case 3: updateUser(); break;
            case 4: deleteUser(); break;
            case 5: printf("Exiting program...\n"); return 0;
            default: printf("Invalid choice.\n");
        }
    }
}
