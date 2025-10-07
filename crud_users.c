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
    struct User u;
    FILE *fp = fopen(FILE_NAME, "a");
    if (!fp) {
        printf("Cannot open file\n");
        return;
    }
    printf("Enter ID: ");
    scanf("%d", &u.id);
    printf("Enter Name: ");
    scanf("%s", u.name);
    printf("Enter Age: ");
    scanf("%d", &u.age);

    fprintf(fp, "%d %s %d\n", u.id, u.name, u.age);
    fclose(fp);
    printf("User added!\n");
}

void showUsers() {
    struct User u;
    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("No users found\n");
        return;
    }
    printf("\n--- Users ---\n");
    while (fscanf(fp, "%d %s %d", &u.id, u.name, &u.age) == 3) {
        printf("ID:%d Name:%s Age:%d\n", u.id, u.name, u.age);
    }
    fclose(fp);
}

void updateUser() {
    struct User u;
    int id, found = 0;
    FILE *fp = fopen(FILE_NAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        printf("Cannot open file\n");
        return;
    }

    printf("Enter ID to update: ");
    scanf("%d", &id);

    while (fscanf(fp, "%d %s %d", &u.id, u.name, &u.age) == 3) {
        if (u.id == id) {
            printf("Enter new Name: ");
            scanf("%s", u.name);
            printf("Enter new Age: ");
            scanf("%d", &u.age);
            found = 1;
        }
        fprintf(temp, "%d %s %d\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (found) printf("User updated!\n");
    else printf("User not found\n");
}

void deleteUser() {
    struct User u;
    int id, found = 0;
    FILE *fp = fopen(FILE_NAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        printf("Cannot open file\n");
        return;
    }

    printf("Enter ID to delete: ");
    scanf("%d", &id);

    while (fscanf(fp, "%d %s %d", &u.id, u.name, &u.age) == 3) {
        if (u.id == id) {
            found = 1;
            continue;
        }
        fprintf(temp, "%d %s %d\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (found) printf("User deleted!\n");
    else printf("User not found\n");
}

int main() {
    int ch;
    while (1) {
        printf("\n--- Menu ---\n");
        printf("1.Add User\n2.Show Users\n3.Update User\n4.Delete User\n5.Exit\n");
        printf("Enter choice: ");
        scanf("%d", &ch);

        if (ch == 1) addUser();
        else if (ch == 2) showUsers();
        else if (ch == 3) updateUser();
        else if (ch == 4) deleteUser();
        else if (ch == 5) exit(0);
        else printf("Invalid choice\n");
    }
    return 0;
}
