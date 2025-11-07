#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BLOCK_SIZE 512
#define NUMBER_OF_BLOCKS 1024
#define MAXIMUM_NAME_LENGTH 50
#define MAXIMUM_INPUT_LENGTH 4096

typedef struct FreeBlockNode {
    int block_index;
    struct FreeBlockNode *previousNode;
    struct FreeBlockNode *nextNode;
} FreeBlockNode;

typedef struct FileNode {
    char name[MAXIMUM_NAME_LENGTH + 1];
    int is_directory;
    struct FileNode *parent;
    struct FileNode *next_sibling;
    struct FileNode *previous_sibling;
    struct FileNode *first_child; 
    
    int content_size;
    int block_count;
    int *block_pointers; 
} FileNode;

static unsigned char virtual_disk[NUMBER_OF_BLOCKS][BLOCK_SIZE];

static FreeBlockNode *free_head = NULL;
static FreeBlockNode *free_tail = NULL;
static int free_block_count = 0;

static FileNode *root_directory = NULL;
static FileNode *current_directory = NULL;

void initialize_filesystem();
void shutdown_filesystem_and_free_memory();
void insert_free_block_tail(int index);
int remove_free_block_head();
void return_blocks_to_free_list(int *blocks, int count);
FileNode* create_file_node(const char *name, int is_directory);
FileNode* find_child_node_by_name(FileNode *directory, const char *name);
int add_child_node_to_directory(FileNode *directory, FileNode *child);
int remove_child_node_from_directory(FileNode *directory, FileNode *child);
void list_directory_contents(FileNode *directory);
void command_mkdir(const char *name);
void command_create(const char *name);
void command_write(const char *filename, const char *content_raw);
void command_read(const char *filename);
void command_delete(const char *filename);
void command_rmdir(const char *name);
void command_cd(const char *name);
void command_pwd();
void command_df();
char *build_absolute_path(FileNode *node);
void free_file_node_recursive(FileNode *node); // used by shutdown
void free_file_node(FileNode *node);
void unescape_backslash_n(char *inputString);

void initialize_filesystem() {
    for (int index = 0; index < NUMBER_OF_BLOCKS; index++) {
        insert_free_block_tail(index);
    }

    root_directory = create_file_node("/", 1);
    root_directory->parent = NULL;
    root_directory->first_child = NULL;
    current_directory = root_directory;
}

void shutdown_filesystem_and_free_memory() {
    free_file_node_recursive(root_directory);

    FreeBlockNode *current = free_head;
    while (current) {
        FreeBlockNode *temporaryNode = current;
        current = current -> nextNode;
        free(temporaryNode);
    }
    free_head = free_tail = NULL;
    free_block_count = 0;
}

void insert_free_block_tail(int index) {
    FreeBlockNode *node = (FreeBlockNode*) malloc(sizeof(FreeBlockNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for free block node\n");
        exit(1);
    }
    node->block_index = index;
    node->nextNode = NULL;
    node->previousNode = free_tail;
    if (free_tail) {
        free_tail -> nextNode = node;
    }
    free_tail = node;
    if (!free_head) {
        free_head = node;
    }
    free_block_count++;
}

int remove_free_block_head() {
    if (!free_head) {
        return -1;
    }
    FreeBlockNode *node = free_head;
    int index = node -> block_index;
    free_head = node -> nextNode;
    if (free_head) {
        free_head->previousNode = NULL;
    }
    else {
        free_tail = NULL;
    }
    free(node);
    free_block_count--;
    return index;
}

void return_blocks_to_free_list(int *blocks, int count) {
    if (count <= 0 || !blocks) {
        return;
    }
    for (int index = 0; index < count; index++) {
        insert_free_block_tail(blocks[index]);
    }
    free(blocks);
}

FileNode* create_file_node(const char *name, int is_directory) {
    FileNode *node = (FileNode*) malloc(sizeof(FileNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for FileNode\n");
        exit(1);
    }
    strncpy(node -> name, name, MAXIMUM_NAME_LENGTH);
    node -> name[MAXIMUM_NAME_LENGTH] = '\0';
    node -> is_directory = is_directory;
    node -> parent = NULL;
    node -> next_sibling = node->previous_sibling = NULL;
    node -> first_child = NULL;
    node -> content_size = 0;
    node -> block_count = 0;
    node -> block_pointers = NULL;

    return node;
}

FileNode* find_child_node_by_name(FileNode *directory, const char *name) {
    if (!directory || !directory -> is_directory) {
        return NULL;
    }
    FileNode *head = directory -> first_child;
    if (!head) {
        return NULL;
    }
    FileNode *current = head;
    do {
        if (strcmp(current -> name, name) == 0) {
            return current;
        }
        directory = current -> next_sibling;
    } while (current && current != head);

    return NULL;
}

int add_child_node_to_directory(FileNode *directory, FileNode *child) {
    if (!directory || !directory -> is_directory || !child) {
        return 0;
    }
    child->parent = directory;
    if (!directory -> first_child) {
        directory -> first_child = child;
        child -> next_sibling = child -> previous_sibling = child;
    } else {
        FileNode *head = directory -> first_child;
        FileNode *tail = head -> previous_sibling;
        tail -> next_sibling = child;
        child -> previous_sibling = tail;
        child -> next_sibling = head;
        head -> previous_sibling = child;
    }
    return 1;
}

int remove_child_node_from_directory(FileNode *directory, FileNode *child) {
    if (!directory || !directory -> is_directory || !child) {
        return 0;
    }
    FileNode *head = directory->first_child;
    if (!head) {
        return 0;
    }
    if (head == child && child->next_sibling == child) {
        directory -> first_child = NULL;
    } else {
        if (directory -> first_child == child) {
            directory -> first_child = child -> next_sibling;
        }
        child -> previous_sibling -> next_sibling = child -> next_sibling;
        child -> next_sibling -> previous_sibling = child -> previous_sibling;
    }
    child->next_sibling = child->previous_sibling = NULL;
    child->parent = NULL;

    return 1;
}

void list_directory_contents(FileNode *directory) {
    if (!directory || !directory -> is_directory) return;
    FileNode *head = directory -> first_child;
    if (!head) {
        printf("(empty)\n");
        return;
    }
    FileNode *current = head;
    do {
        if (current -> is_directory) {
            printf("%s/\n", current -> name);
        }
        else {
            printf("%s\n", current -> name);
        }
        current = current -> next_sibling;
    } while (current && current != head);
}

void command_mkdir(const char *name) {
    if (!name || strlen(name) == 0) {
        return;
    }
    if (find_child_node_by_name(current_directory, name)) {
        printf("Name already exists in current directory.\n");
        return;
    }
    FileNode *new_directory = create_file_node(name, 1);
    add_child_node_to_directory(current_directory, new_directory);
    printf("Directory '%s' created successfully.\n", name);
}

void command_create(const char *name) {
    if (!name || strlen(name) == 0) {
        return;
    }
    if (find_child_node_by_name(current_directory, name)) {
        printf("Name already exists in current directory.\n");
        return;
    }
    FileNode *new_file = create_file_node(name, 0);
    add_child_node_to_directory(current_directory, new_file);
    printf("File '%s' created successfully.\n", name);
}

void command_write(const char *filename, const char *content_raw) {
    if (!filename) {
        return;
    }
    FileNode *file = find_child_node_by_name(current_directory, filename);
    if (!file || file -> is_directory) {
        printf("File not found.\n");
        return;
    }

    char *content = NULL;
    if (content_raw) {
        content = strdup(content_raw);
        unescape_backslash_n(content);
    } else {
        content = strdup("");
    }

    int byte_count = (int)strlen(content);
    int required_blocks = (byte_count + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (byte_count == 0) {
        required_blocks = 0;
    }

    if (required_blocks > free_block_count) {
        printf("Disk is full. Write operation failed.\n");
        free(content);
        return;
    }

    if (file -> block_count > 0 && file -> block_pointers) {
        return_blocks_to_free_list(file -> block_pointers, file -> block_count);
        file -> block_pointers = NULL;
        file -> block_count = 0;
        file -> content_size = 0;
    }

    if (required_blocks > 0) {
        file -> block_pointers = (int*) malloc(sizeof(int) * required_blocks);
        if (!file -> block_pointers) {
            fprintf(stderr, "Memory allocation failed for block pointers\n");
            free(content);
            exit(1);
        }
        file -> block_count = required_blocks;
        file -> content_size = byte_count;

        int bytes_remaining = byte_count;
        int offset_in_content = 0;
        for (int index = 0; index < required_blocks; index++) {
            int block_index = remove_free_block_head();
            if (block_index < 0) {
                fprintf(stderr, "Unexpected free list underflow\n");
                free(content);
                return;
            }
            file -> block_pointers[index] = block_index;

            int to_copy = bytes_remaining < BLOCK_SIZE ? bytes_remaining : BLOCK_SIZE;
            if (to_copy > 0) {
                memcpy(virtual_disk[block_index], content + offset_in_content, to_copy);
                if (to_copy < BLOCK_SIZE) {
                    memset(virtual_disk[block_index] + to_copy, 0, BLOCK_SIZE - to_copy);
                }
                offset_in_content += to_copy;
                bytes_remaining -= to_copy;
            } else {
                memset(virtual_disk[block_index], 0, BLOCK_SIZE);
            }
        }
    } else {
        file -> block_pointers = NULL;
        file -> block_count = 0;
        file -> content_size = 0;
    }

    printf("Data written successfully (size=%d bytes).\n", byte_count);
    free(content);
}

void command_read(const char *filename) {
    if (!filename) {
        return;
    }
    FileNode *file = find_child_node_by_name(current_directory, filename);
    if (!file || file -> is_directory) {
        printf("File not found.\n");
        return;
    }
    if (file -> content_size == 0 || file -> block_count == 0) {
        printf("(empty)\n");
        return;
    }
    int bytes_to_print = file -> content_size;
    int printed = 0;
    for (int index = 0; index < file->block_count && printed < bytes_to_print; index++) {
        int block_idx = file->block_pointers[index];
        int to_print = (bytes_to_print - printed) < BLOCK_SIZE ? (bytes_to_print - printed) : BLOCK_SIZE;
        fwrite(virtual_disk[block_idx], 1, to_print, stdout);
        printed += to_print;
    }
    printf("\n");
}

void command_delete(const char *filename) {
    if (!filename) {
        return;
    } 
    FileNode *file = find_child_node_by_name(current_directory, filename);
    if (!file || file -> is_directory) {
        printf("File not found.\n");
        return;
    }
    if (file -> block_count > 0 && file -> block_pointers) {
        return_blocks_to_free_list(file -> block_pointers, file -> block_count);
        file -> block_pointers = NULL;
        file -> block_count = 0;
        file -> content_size = 0;
    }
    remove_child_node_from_directory(current_directory, file);
    free_file_node(file);
    printf("File deleted successfully.\n");
}

void command_rmdir(const char *name) {
    if (!name || strlen(name) == 0) {
        return;
    }
    FileNode *directory = find_child_node_by_name(current_directory, name);
    if (!directory || !directory -> is_directory) {
        printf("Directory not found.\n");
        return;
    }
    if (directory->first_child != NULL) {
        printf("Directory not empty. Remove files first.\n");
        return;
    }
    remove_child_node_from_directory(current_directory, directory);
    free_file_node(directory);
    printf("Directory removed successfully.\n");
}

void command_cd(const char *name) {
    if (!name || strlen(name) == 0) {
        return;
    }
    if (strcmp(name, "..") == 0) {
        if (current_directory->parent) current_directory = current_directory->parent;
        char *path = build_absolute_path(current_directory);
        printf("Moved to %s\n", path);
        free(path);
        return;
    }
    if (strcmp(name, "/") == 0) {
        current_directory = root_directory;
        char *path = build_absolute_path(current_directory);
        printf("Moved to %s\n", path);
        free(path);
        return;
    }
    FileNode *target = find_child_node_by_name(current_directory, name);
    if (!target || !target->is_directory) {
        printf("Directory not found.\n");
        return;
    }
    current_directory = target;
    char *path = build_absolute_path(current_directory);
    printf("Moved to %s\n", path);
    free(path);
}

void command_pwd() {
    char *path = build_absolute_path(current_directory);
    printf("%s\n", path);
    free(path);
}

void command_df() {
    int used = NUMBER_OF_BLOCKS - free_block_count;
    int freeBlockCount = free_block_count;
    double usage_percent = 0.0;
    if (NUMBER_OF_BLOCKS > 0) {
        usage_percent = ((double)used / (double)NUMBER_OF_BLOCKS) * 100.0;
    }
    printf("Total Blocks: %d\n", NUMBER_OF_BLOCKS);
    printf("Used Blocks: %d\n", used);
    printf("Free Blocks: %d\n", freeBlockCount);
    printf("Disk Usage: %.2f%%\n", usage_percent);
}

char *build_absolute_path(FileNode *node) {
    if (!node) {
        return strdup("/");
    }
    if (node == root_directory) {
        return strdup("/");
    }

    char buffer[MAXIMUM_INPUT_LENGTH];
    buffer[0] = '\0';
    FileNode *current = node;
    char temporaryBuffer[MAXIMUM_INPUT_LENGTH];
    temporaryBuffer[0] = '\0';
    while (current && current != root_directory) {
        char piece[128];
        snprintf(piece, sizeof(piece), "/%s", current -> name);
        char newTemporaryBuffer[MAXIMUM_INPUT_LENGTH];
        snprintf(newTemporaryBuffer, sizeof(newTemporaryBuffer), "%s%s", piece, temporaryBuffer);
        strncpy(temporaryBuffer, newTemporaryBuffer, sizeof(temporaryBuffer) - 1);
        temporaryBuffer[sizeof(temporaryBuffer)-1] = '\0';
        current = current -> parent;
    }
    if (temporaryBuffer[0] == '\0') {
        strncpy(buffer, "/", sizeof(buffer)-1);
        buffer[sizeof(buffer)-1] = '\0';
    } else {
        strncpy(buffer, temporaryBuffer, sizeof(buffer)-1);
        buffer[sizeof(buffer)-1] = '\0';
    }
    return strdup(buffer);
}

void free_file_node_recursive(FileNode *node) {
    if (!node) {
        return;
    }
    if (node->is_directory) {
        FileNode *head = node->first_child;
        if (head) {
            FileNode *current = head;
            FileNode **collected = NULL;
            int count = 0;
            do {
                collected = (FileNode**) realloc(collected, sizeof(FileNode*) * (count + 1));
                collected[count++] = current;
                current = current -> next_sibling;
            } while (current && current != head);

            for (int index = 0; index < count; index++) {
                free_file_node_recursive(collected[index]);
            }
            free(collected);
        }
        free(node);
    } else {
        if (node -> block_count > 0 && node -> block_pointers) {
            return_blocks_to_free_list(node -> block_pointers, node -> block_count);
            node -> block_pointers = NULL;
            node -> block_count = 0;
        }
        free(node);
    }
}

void free_file_node(FileNode *node) {
    if (!node) {
        return;
    }
    if (node -> is_directory) {
        free(node);
    } else {
        if (node -> block_count > 0 && node -> block_pointers) {
            return_blocks_to_free_list(node -> block_pointers, node -> block_count);
            node -> block_pointers = NULL;
            node -> block_count = 0;
        }
        free(node);
    }
}

void unescape_backslash_n(char *inputString) {
    if (!inputString) {
        return;
    }

    char *readPointer = inputString;
    char *writePointer = inputString;

    while (*readPointer) {
        if (readPointer[0] == '\\' && readPointer[1] == 'n') {
            *writePointer++ = '\n'; 
            readPointer += 2; 
        } else {
            *writePointer++ = *readPointer++;
        }
    }

    *writePointer = '\0'; 
}

static void trim_newline(char *inputString) {
    if (inputString == NULL) {
        return;
    }

    size_t stringLength = strlen(inputString);
    if (stringLength == 0) {
        return;
    }

    if (inputString[stringLength - 1] == '\n') {
        inputString[stringLength - 1] = '\0';
    }
}

int main(void) {
    initialize_filesystem();
    printf("Compact VFS - ready. Type 'exit' to quit.\n\n");

    char input_line[MAXIMUM_INPUT_LENGTH];

    while (1) {
        if (current_directory == root_directory) {
            printf("/ > ");
        }
        else {
            printf("%s > ", current_directory->name);
        }

        if (!fgets(input_line, sizeof(input_line), stdin)) {
            printf("\n");
            break;
        }
        trim_newline(input_line);

        char *line = input_line;
        while (*line && isspace((unsigned char)*line)) {
            line++;
        }
        if (*line == '\0') {
            continue;
        }

        char *command = strtok(line, " ");
        if (!command) {
            continue;
        }

        if (strcmp(command, "exit") == 0) {
            printf("Memory released. Exiting program...\n");
            break;
        } else if (strcmp(command, "mkdir") == 0) {
            char *name = strtok(NULL, " ");
            if (!name) {
                continue;
            }
            command_mkdir(name);
        } else if (strcmp(command, "create") == 0) {
            char *name = strtok(NULL, " ");
            if (!name) continue;
            command_create(name);
        } else if (strcmp(command, "write") == 0) {
            char *filename = strtok(NULL, " ");
            if (!filename) {
                printf("File not found.\n");
                continue;
            }
            char *first_quote = strchr(input_line, '"');
            char *last_quote = NULL;
            char *content_between = NULL;
            if (first_quote) {
                last_quote = strrchr(input_line, '"');
                if (last_quote != first_quote) {
                    content_between = first_quote + 1;
                    size_t content_len = (size_t)(last_quote - content_between);
                    char *content_buffer = (char*) malloc(content_len + 1);
                    if (content_buffer) {
                        memcpy(content_buffer, content_between, content_len);
                        content_buffer[content_len] = '\0';
                        command_write(filename, content_buffer);
                        free(content_buffer);
                    } else {
                        command_write(filename, "");
                    }
                } else {
                    command_write(filename, "");
                }
            } else {
                char *rest = strtok(NULL, "");
                if (rest) {
                    while (*rest && isspace((unsigned char)*rest)) rest++;
                    command_write(filename, rest);
                } else {
                    command_write(filename, "");
                }
            }
        } else if (strcmp(command, "read") == 0) {
            char *filename = strtok(NULL, " ");
            if (!filename) {
                printf("File not found.\n");
                continue;
            }
            command_read(filename);
        } else if (strcmp(command, "delete") == 0) {
            char *filename = strtok(NULL, " ");
            if (!filename) {
                printf("File not found.\n");
                continue;
            }
            command_delete(filename);
        } else if (strcmp(command, "rmdir") == 0) {
            char *name = strtok(NULL, " ");
            if (!name) {
                continue;
            }
            command_rmdir(name);
        } else if (strcmp(command, "ls") == 0) {
            list_directory_contents(current_directory);
        } else if (strcmp(command, "cd") == 0) {
            char *name = strtok(NULL, " ");
            if (!name) {
                continue;
            }
            command_cd(name);
        } else if (strcmp(command, "pwd") == 0) {
            command_pwd();
        } else if (strcmp(command, "df") == 0) {
            command_df();
        } else {
            printf("Unknown command.\n");
        }
    }

    shutdown_filesystem_and_free_memory();
    return 0;
}
