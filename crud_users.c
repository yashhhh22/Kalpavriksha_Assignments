#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "users.txt"

// user structure to store each user data
struct User {
    int id;
    char name[50];
    int age;
};

// whenever we have to add a user
void addUser()
{
    struct User u, temp;
    FILE *fp;
    fp = fopen(FILE_NAME, "r");
    if (fp)
    {
        printf("Enter ID: ");
        scanf("%d", &u.id);
        while (fscanf(fp, "%d %s %d", &temp.id, temp.name, &temp.age) == 3)
        {
            if (temp.id == u.id)
            {
                printf("User ID already exists. Cannot add duplicate.\n");
                fclose(fp);
                return; 
            }
        }
        fclose(fp);
    }
    else
    {
        printf("Enter ID: ");
        scanf("%d", &u.id);
    }
    fp = fopen(FILE_NAME, "a");
    if (!fp)
    {
        printf("Cannot open file\n");
        return;
    }

    printf("Enter Name: ");
    scanf("%s", u.name);
    printf("Enter Age: ");
    scanf("%d", &u.age);

    fprintf(fp, "%d %s %d\n", u.id, u.name, u.age);
    fclose(fp);

    printf("User added successfully!\n");
}

// show all the users added
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

// update the data of the users
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

    if (found) {
        printf("User updated!\n");
    } else {
        printf("User not found\n");
    }
}

// delete the user added before
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

    if (found) {
        printf("User deleted!\n");
    } else {
        printf("User not found\n");
    }
}

int main() {
    int option;
    while (1) {
        printf("\n--- Menu ---\n");
        printf("1.Add User\n2.Show Users\n3.Update User\n4.Delete User\n5.Exit\n");
        printf("Enter choice: ");
        scanf("%d", &option);

        if (option == 1) {
            addUser();
        } else if (option == 2) {
            showUsers();
        } else if (option == 3) {
            updateUser();
        } else if (option == 4) {
            deleteUser();
        } else if (option == 5) {
            printf("Exiting program...\n");
            return 0;
        }
        else {
            printf("Invalid choice\n");
        }
    }
    return 0;
}
