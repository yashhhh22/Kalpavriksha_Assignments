#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXIMUM_NAME_LENGTH 50
#define INITIAL_NUMBER_OF_PRODUCTS 100
#define MINIMUM_PRICE 0.0f
#define MAXIMUM_PRICE 100000.0f
#define MINIMUM_QUANTITY 0
#define MAXIMUM_QUANTITY 1000000
#define MINIMUM_ID 1
#define MAXIMUM_ID 10000

typedef struct {
    int product_id;
    char product_name[MAXIMUM_NAME_LENGTH + 1];
    float product_price;
    int product_quantity;
} Product;

int getValidIntegerInRange(int minimumValue, int maximumValue, const char *displayText);
float getValidFloatInRange(float minimumValue, float maximumValue, const char *displayText);
void getValidString(char *name, const char *displayText);

void initializeProducts(Product *products, int count);
void handleMenu(Product **products, int *count);
int caseSensitiveMatch(const char *productName, const char *searchName);

void addProduct(Product **products, int *count);
void viewProducts(const Product *products, int count);
void updateQuantity(Product *products, int count);
void searchByID(const Product *products, int count);
void searchByName(const Product *products, int count);
void searchByPriceRange(const Product *products, int count);
void deleteProduct(Product **products, int *count);

int main() {
    int numberOfProducts;
    Product *products = NULL;

    numberOfProducts = getValidIntegerInRange(1, INITIAL_NUMBER_OF_PRODUCTS, "Enter initial number of products: ");

    products = (Product *)calloc(numberOfProducts, sizeof(Product));

    if (!products) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    initializeProducts(products, numberOfProducts);
    handleMenu(&products, &numberOfProducts);

    return 0;
}

int getValidIntegerInRange(int minimumValue, int maximumValue, const char *displayText) {
    char input[100];
    int value;

    while (1) {
        printf("%s", displayText);
        if (!fgets(input, sizeof(input), stdin)) {
            continue;
        }

        char *endPointer;
        value = strtol(input, &endPointer, 10);

        if (endPointer == input || *endPointer != '\n') {
            printf("Invalid input. Enter an integer.\n");
            continue;
        }

        if (value < minimumValue || value > maximumValue) {
            printf("Value out of range (%d - %d).\n", minimumValue, maximumValue);
            continue;
        }
        return value;
    }
}

float getValidFloatInRange(float minimumValue, float maximumValue, const char *displayText) {
    char input[100];
    float value;

    while (1) {
        printf("%s", displayText);
        if (!fgets(input, sizeof(input), stdin)) {
            continue;
        }

        char *endPointer;
        value = strtof(input, &endPointer);

        if (endPointer == input || *endPointer != '\n') {
            printf("Invalid input. Enter a decimal number.\n");
            continue;
        }

        if (value < minimumValue || value > maximumValue) {
            printf("Value out of range (%.2f - %.2f).\n", minimumValue, maximumValue);
            continue;
        }

        return value;
    }
}

void getValidString(char *name, const char *displayText) {
    while (1) {
        printf("%s", displayText);

        if (!fgets(name, MAXIMUM_NAME_LENGTH + 1, stdin)) {
            printf("Error reading input. Try again.\n");
            continue;
        }

        name[strcspn(name, "\n")] = '\0';

        if (strlen(name) == 0) {
            printf("Input cannot be empty. Try again.\n");
            continue;
        }

        break;
    }
}

void initializeProducts(Product *products, int count) {
    for (int productIndex = 0; productIndex < count; productIndex++) {
        printf("\nEnter details for product %d:\n", productIndex + 1);
        products[productIndex].product_id = getValidIntegerInRange(MINIMUM_ID, MAXIMUM_ID, "Product ID: ");
        getValidString(products[productIndex].product_name, "Product Name: ");
        products[productIndex].product_price = getValidFloatInRange(MINIMUM_PRICE, MAXIMUM_PRICE, "Product Price: ");
        products[productIndex].product_quantity = getValidIntegerInRange(MINIMUM_QUANTITY, MAXIMUM_QUANTITY, "Product Quantity: ");
    }
}

void handleMenu(Product **products, int *count) {
    int choice;

    do {
        printf("\n========= INVENTORY MENU =========\n");
        printf("1. Add New Product\n");
        printf("2. View All Products\n");
        printf("3. Update Quantity\n");
        printf("4. Search Product by ID\n");
        printf("5. Search Product by Name\n");
        printf("6. Search Product by Price Range\n");
        printf("7. Delete Product\n");
        printf("8. Exit\n");
        choice = getValidIntegerInRange(1, 8, "Enter your choice: ");

        switch (choice) {
            case 1: {
                addProduct(products, count); 
                break;
            }

            case 2: {
                viewProducts(*products, *count); 
                break;
            }

            case 3: {
                updateQuantity(*products, *count); 
                break;
            }

            case 4: {
                searchByID(*products, *count); 
                break;
            }

            case 5: {
                searchByName(*products, *count); 
                break;
            }

            case 6: {
                searchByPriceRange(*products, *count); 
                break;
            }
            case 7: {
                deleteProduct(products, count); 
                break;
            }

            case 8: {
                printf("Memory released successfully. Exiting program...\n");
                free(*products);
                return;
            }
        }
    } while (choice != 8);
}

void addProduct(Product **products, int *count) {
    if (*count >= INITIAL_NUMBER_OF_PRODUCTS) {
        printf("Cannot add more products (limit reached).\n");
        return;
    }

    *products = realloc(*products, (*count + 1) * sizeof(Product));

    if (!*products) {
        printf("Memory reallocation failed.\n");
        return;
    }

    Product *newProduct = &((*products)[*count]);

    printf("\nEnter new product details:\n");
    newProduct->product_id = getValidIntegerInRange(MINIMUM_ID, MAXIMUM_ID, "Product ID: ");
    getValidString(newProduct->product_name, "Product Name: ");
    newProduct->product_price = getValidFloatInRange(MINIMUM_PRICE, MAXIMUM_PRICE, "Product Price: ");
    newProduct->product_quantity = getValidIntegerInRange(MINIMUM_QUANTITY, MAXIMUM_QUANTITY, "Product Quantity: ");

    (*count)++;
    printf("Product added successfully!\n");
}

void viewProducts(const Product *products, int count) {
    if (count == 0) {
        printf("No products to display.\n");
        return;
    }

    printf("\n========= PRODUCT LIST =========\n");
    for (int productIndex = 0; productIndex < count; productIndex++) {
        printf("ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
               products[productIndex].product_id, products[productIndex].product_name,
               products[productIndex].product_price, products[productIndex].product_quantity);
    }
}

void updateQuantity(Product *products, int count) {
    int id = getValidIntegerInRange(MINIMUM_ID, MAXIMUM_ID, "Enter Product ID to update quantity: ");
    
    for (int productIndex = 0; productIndex < count; productIndex++) {
        if (products[productIndex].product_id == id) {
            products[productIndex].product_quantity = getValidIntegerInRange(MINIMUM_QUANTITY, MAXIMUM_QUANTITY, "Enter new Quantity: ");
            printf("Quantity updated successfully!\n");
            return;
        }
    }
    printf("Product not found!\n");
}

void searchByID(const Product *products, int count) {
    int id = getValidIntegerInRange(MINIMUM_ID, MAXIMUM_ID, "Enter Product ID to search: ");
    for (int productIndex = 0; productIndex < count; productIndex++) {
        if (products[productIndex].product_id == id) {
            printf("Product Found: ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   products[productIndex].product_id, products[productIndex].product_name,
                   products[productIndex].product_price, products[productIndex].product_quantity);
            return;
        }
    }
    printf("Product not found!\n");
}

int caseSensitiveMatch(const char *productName, const char *searchName) {
    char character1, character2;
    int productNameIndex, searchNameIndex;

    for (productNameIndex = 0; productName[productNameIndex] != '\0'; productNameIndex++) {
        for (searchNameIndex = 0; searchName[searchNameIndex] != '\0'; searchNameIndex++) {
            character1 = (unsigned char)productName[productNameIndex + searchNameIndex];
            character2 = (unsigned char)searchName[searchNameIndex];

            if (character1 != character2 || productName[productNameIndex + searchNameIndex] == '\0') {
                break;
            }
        }

        if (searchName[searchNameIndex] == '\0') {
            return 1; 
        }
    }
    return 0; 
}

void searchByName(const Product *products, int count) {
    char name[MAXIMUM_NAME_LENGTH];
    int found = 0;

    printf("Enter name to search (partial allowed): ");
    fgets(name, MAXIMUM_NAME_LENGTH, stdin);
    name[strcspn(name, "\n")] = '\0';

    for (int productIndex = 0; productIndex < count; productIndex++) {
        if (caseSensitiveMatch(products[productIndex].product_name, name)) {
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   products[productIndex].product_id, products[productIndex].product_name,
                   products[productIndex].product_price, products[productIndex].product_quantity);
            found = 1;
        }
    }

    if (!found)
        printf("No products match the given name.\n");
}

void searchByPriceRange(const Product *products, int count) {
    float minimumPrice = getValidFloatInRange(MINIMUM_PRICE, MAXIMUM_PRICE, "Enter minimum price: ");
    float maximumPrice = getValidFloatInRange(minimumPrice, MAXIMUM_PRICE, "Enter maximum price: ");

    int found = 0;
    printf("\nProducts in price range:\n");

    for (int productIndex = 0; productIndex < count; productIndex++) {
        if (products[productIndex].product_price >= minimumPrice && products[productIndex].product_price <= maximumPrice) {
            printf("ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   products[productIndex].product_id, products[productIndex].product_name,
                   products[productIndex].product_price, products[productIndex].product_quantity);

            found = 1;
        }
    }

    if (!found)
        printf("No products found in this price range.\n");
}

void deleteProduct(Product **products, int *count) {
    if (*count == 0) {
        printf("No products to delete.\n");
        return;
    }

    int id = getValidIntegerInRange(MINIMUM_ID, MAXIMUM_ID, "Enter Product ID to delete: ");
    int foundIndex = -1;

    for (int productIndex = 0; productIndex < *count; productIndex++) {
        if ((*products)[productIndex].product_id == id) {
            foundIndex = productIndex;
            break;
        }
    }

    if (foundIndex == -1) {
        printf("Product not found!\n");
        return;
    }

    for (int productIndex = foundIndex; productIndex < *count - 1; productIndex++) {
        (*products)[productIndex] = (*products)[productIndex + 1];
    }

    (*count)--;
    *products = realloc(*products, (*count) * sizeof(Product));

    printf("Product deleted successfully!\n");
}
