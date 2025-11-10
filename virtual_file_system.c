#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BLOCKS_PER_FILE 100
#define MAX_INPUT_LENGTH 80
#define BLOCK_SIZE 512
#define MAX_FILENAME_LENGTH 51
#define TOTAL_DISK_BLOCKS 1024
#define MAX_PATH_LENGTH 1000

typedef struct DiskBlock
{
    int blockIndex;
    struct DiskBlock *previous;
    struct DiskBlock *next;
} DiskBlock;

typedef struct Node
{
    char name[MAX_FILENAME_LENGTH];
    int isFolder;
    struct Node *parent;
    struct Node *nextSibling;
    struct Node *firstChild;
    int allocatedBlocks[MAX_BLOCKS_PER_FILE];
    int dataSize;
    int blockCount;
} Node;

char diskMemory[TOTAL_DISK_BLOCKS][BLOCK_SIZE];
DiskBlock *freeBlockListHead = NULL;
Node *rootDirectory = NULL;
Node *currentDirectory = NULL;

void initializeFreeBlocks()
{
    DiskBlock *prevBlock = NULL;

    for (int index = 0; index < TOTAL_DISK_BLOCKS; index++)
    {
        DiskBlock *newBlock = (DiskBlock *)malloc(sizeof(DiskBlock));
        newBlock->blockIndex = index;
        newBlock->next = NULL;
        newBlock->previous = prevBlock;

        if (prevBlock)
        {
            prevBlock->next = newBlock;
        }
        else
        {
            freeBlockListHead = newBlock;
        }

        prevBlock = newBlock;
    }
}

void initializeVFS()
{
    initializeFreeBlocks();

    rootDirectory = (Node *)malloc(sizeof(Node));
    rootDirectory->isFolder = 1;
    rootDirectory->parent = NULL;
    rootDirectory->firstChild = NULL;
    strcpy(rootDirectory->name, "/");
    rootDirectory->nextSibling = rootDirectory;
    rootDirectory->dataSize = 0;
    rootDirectory->blockCount = 0;
    currentDirectory = rootDirectory;

    printf("Compact VFS - ready. Type 'exit' to quit.\n");
}

int allocateBlock()
{
    if (freeBlockListHead == NULL)
    {
        printf("No memory left.\n");
        return -1;
    }
    DiskBlock *blockToAllocate = freeBlockListHead;
    int index = blockToAllocate->blockIndex;
    freeBlockListHead = freeBlockListHead->next;
    if (freeBlockListHead)
    {
        freeBlockListHead->previous = NULL;
    }
    free(blockToAllocate);
    return index;
}

void releaseFileBlocks(Node *file)
{
    for (int index = 0; index < file->blockCount; index++)
    {
        int blockIdx = file->allocatedBlocks[index];
        if (blockIdx >= 0)
        {
            DiskBlock *restoredBlock = (DiskBlock *)malloc(sizeof(DiskBlock));
            restoredBlock->blockIndex = blockIdx;
            restoredBlock->next = freeBlockListHead;
            if (freeBlockListHead)
            {
                freeBlockListHead->previous = restoredBlock;
            }
            restoredBlock->previous = NULL;
            freeBlockListHead = restoredBlock;
            file->allocatedBlocks[index] = -1;
        }
    }
    file->dataSize = 0;
    file->blockCount = 0;
}

void makeDirectory(char *folderName)
{
    Node *node = currentDirectory->firstChild;
    if (node)
    {
        do
        {
            if (strcmp(node->name, folderName) == 0)
            {
                printf("Directory with name %s already exists.\n", folderName);
                return;
            }
            node = node->nextSibling;
        } while (node != currentDirectory->firstChild);
    }

    Node *newFolder = (Node *)malloc(sizeof(Node));
    strcpy(newFolder->name, folderName);
    newFolder->isFolder = 1;
    newFolder->parent = currentDirectory;
    newFolder->firstChild = NULL;
    newFolder->dataSize = 0;
    newFolder->blockCount = 0;

    if (currentDirectory->firstChild == NULL)
    {
        currentDirectory->firstChild = newFolder;
        newFolder->nextSibling = newFolder;
    }
    else
    {
        Node *temporaryNode = currentDirectory->firstChild;
        while (temporaryNode->nextSibling != currentDirectory->firstChild)
        {
            temporaryNode = temporaryNode->nextSibling;
        }
        newFolder->nextSibling = temporaryNode->nextSibling;
        temporaryNode->nextSibling = newFolder;
    }
    printf("Directory '%s' created successfully.\n", folderName);
}

void listDirectory()
{
    Node *temporaryNode = currentDirectory->firstChild;
    if (temporaryNode == NULL)
    {
        printf("(empty)\n");
        return;
    }
    do
    {
        printf("%s", temporaryNode->name);
        if (temporaryNode->isFolder)
        {
            printf("/");
        }
        printf("\n");
        temporaryNode = temporaryNode->nextSibling;
    } while (temporaryNode != currentDirectory->firstChild);
}

void printCurrentPath()
{
    Node *temporaryNode = currentDirectory;
    char fullPath[MAX_PATH_LENGTH] = "";
    char temporaryPath[MAX_PATH_LENGTH] = "";
    while (temporaryNode != rootDirectory)
    {
        snprintf(temporaryPath, sizeof(temporaryPath), "%s/%s", temporaryNode->name, fullPath);
        strncpy(fullPath, temporaryPath, sizeof(fullPath));
        temporaryNode = temporaryNode->parent;
    }
    if (strlen(fullPath) == 0)
    {
        printf("/\n");
        return;
    }
    printf("/%s\n", fullPath);
}

void moveToParentDirectory()
{
    if (currentDirectory == rootDirectory)
    {
        printf("Already on root.\n");
        return;
    }
    currentDirectory = currentDirectory->parent;
    printf("Moved to ");
    printCurrentPath();
}

void changeDirectory(char *targetName)
{
    if (strcmp(targetName, "..") == 0)
    {
        moveToParentDirectory();
        return;
    }

    if (currentDirectory->firstChild == NULL)
    {
        printf("%s is empty.\n", currentDirectory->name);
        return;
    }
    Node *temporaryNode = currentDirectory->firstChild;
    do
    {
        if (strcmp(temporaryNode->name, targetName) == 0 && temporaryNode->isFolder)
        {
            currentDirectory = temporaryNode;
            printf("Moved to ");
            printCurrentPath();
            return;
        }
        temporaryNode = temporaryNode->nextSibling;
    } while (temporaryNode != currentDirectory->firstChild);
    printf("No folder found with the name %s\n", targetName);
}

void createFile(char *fileName)
{
    Node *node = currentDirectory->firstChild;
    if (node)
    {
        do
        {
            if (strcmp(node->name, fileName) == 0)
            {
                printf("File with name %s already exists.\n", fileName);
                return;
            }
            node = node->nextSibling;
        } while (node != currentDirectory->firstChild);
    }

    Node *newFile = (Node *)malloc(sizeof(Node));
    strcpy(newFile->name, fileName);
    newFile->isFolder = 0;
    newFile->parent = currentDirectory;
    newFile->firstChild = NULL;
    newFile->dataSize = 0;
    newFile->blockCount = 0;

    for (int index = 0; index < MAX_BLOCKS_PER_FILE; index++)
    {
        newFile->allocatedBlocks[index] = -1;
    }

    if (currentDirectory->firstChild == NULL)
    {
        currentDirectory->firstChild = newFile;
        newFile->nextSibling = newFile;
    }
    else
    {
        Node *temporaryNode = currentDirectory->firstChild;
        while (temporaryNode->nextSibling != currentDirectory->firstChild)
        {
            temporaryNode = temporaryNode->nextSibling;
        }
        newFile->nextSibling = temporaryNode->nextSibling;
        temporaryNode->nextSibling = newFile;
    }
    printf("File '%s' created successfully.\n", fileName);
}

void writeContent(Node *file, char *data)
{
    if (!file || file->isFolder)
    {
        printf("Invalid file.\n");
        return;
    }
    if (file->blockCount > 0)
    {
        releaseFileBlocks(file);
    }
    int length = strlen(data);
    int blocksNeeded = (length + BLOCK_SIZE - 1) / BLOCK_SIZE;
    file->dataSize = length;
    file->blockCount = 0;
    int offset = 0;

    for (int index = 0; index < blocksNeeded; index++)
    {
        int temporaryIndex = allocateBlock();
        if (temporaryIndex == -1)
        {
            return;
        }
        file->allocatedBlocks[file->blockCount++] = temporaryIndex;
        strncpy(diskMemory[temporaryIndex], data + offset, BLOCK_SIZE);
        offset += BLOCK_SIZE;
    }
    printf("Data written successfully(size = %d bytes)\n", file->dataSize);
}

void writeFile(char *fileName, char *data)
{
    if (data == NULL || strlen(data) < 2)
    {
        printf("Error: Missing or invalid data. Use quotes around your text.\n");
        return;
    }

    if (data[0] != '"' || data[strlen(data) - 1] != '"')
    {
        printf("Error: Data must be enclosed in double quotes. Example: write file \"Hello World\"\n");
        return;
    }

    data[strlen(data) - 1] = '\0';
    data++;

    int writeIndex = 0;
    for (int index = 0; data[index] != '\0'; index++, writeIndex++)
    {
        if (data[index] == '\\' && data[index + 1] == 'n')
        {
            data[writeIndex] = '\n';
            index++;
        }
        else
        {
            data[writeIndex] = data[index];
        }
    }
    data[writeIndex] = '\0';

    Node *temporaryNode = currentDirectory->firstChild;
    if (temporaryNode == NULL)
    {
        printf("Error: No files exist in the current directory.\n");
        return;
    }

    do
    {
        if (strcmp(temporaryNode->name, fileName) == 0)
        {
            writeContent(temporaryNode, data);
            return;
        }
        temporaryNode = temporaryNode->nextSibling;
    } while (temporaryNode != currentDirectory->firstChild);

    printf("Error: File '%s' not found in the current directory.\n", fileName);
}

void displayFileContent(Node *file)
{
    if (file->blockCount == 0)
    {
        printf("File is empty.\n");
        return;
    }
    for (int index = 0; index < file->blockCount; index++)
    {
        int temporaryIndex = file->allocatedBlocks[index];
        if (temporaryIndex >= 0)
        {
            printf("%s", diskMemory[temporaryIndex]);
        }
    }
    printf("\n");
}

void readFile(char *fileName)
{
    Node *temporaryNode = currentDirectory->firstChild;
    if (!temporaryNode)
    {
        printf("No file found.\n");
        return;
    }
    do
    {
        if (strcmp(temporaryNode->name, fileName) == 0 && !temporaryNode->isFolder)
        {
            displayFileContent(temporaryNode);
            return;
        }
        temporaryNode = temporaryNode->nextSibling;
    } while (temporaryNode != currentDirectory->firstChild);
    printf("No file found with name %s.\n", fileName);
}

void removeFile(Node *file)
{
    releaseFileBlocks(file);
    if (file->nextSibling == file)
    {
        currentDirectory->firstChild = NULL;
    }
    else
    {
        Node *temporaryNode = currentDirectory->firstChild;
        while (temporaryNode->nextSibling != file && temporaryNode->nextSibling != currentDirectory->firstChild)
        {
            temporaryNode = temporaryNode->nextSibling;
        }
        temporaryNode->nextSibling = file->nextSibling;
        if (file == currentDirectory->firstChild)
        {
            currentDirectory->firstChild = file->nextSibling;
        }
    }
    free(file);
    printf("File deleted successfully.\n");
}

void deleteFileByName(char *fileName)
{
    if (currentDirectory->firstChild == NULL)
    {
        printf("No file found.\n");
        return;
    }
    Node *temporaryNode = currentDirectory->firstChild;
    do
    {
        if (strcmp(temporaryNode->name, fileName) == 0 && !temporaryNode->isFolder)
        {
            removeFile(temporaryNode);
            return;
        }
        temporaryNode = temporaryNode->nextSibling;
    } while (temporaryNode != currentDirectory->firstChild);
    printf("No file found with name %s.\n", fileName);
}

void removeDirectory(char *dirName)
{
    if (currentDirectory->firstChild == NULL)
    {
        printf("No directory found.\n");
        return;
    }
    Node *temporaryNode = currentDirectory->firstChild;
    do
    {
        if (strcmp(temporaryNode->name, dirName) == 0)
        {
            if (!temporaryNode->isFolder)
            {
                printf("%s is not a directory.\n", dirName);
                return;
            }
            if (temporaryNode->firstChild != NULL)
            {
                printf("Directory not empty. Remove files first.\n");
                return;
            }
            if (temporaryNode->nextSibling == temporaryNode)
            {
                currentDirectory->firstChild = NULL;
            }
            else
            {
                Node *node = currentDirectory->firstChild;
                while (node->nextSibling != temporaryNode && node->nextSibling != currentDirectory->firstChild)
                {
                    node = node->nextSibling;
                }
                node->nextSibling = temporaryNode->nextSibling;
                if (temporaryNode == currentDirectory->firstChild)
                {
                    currentDirectory->firstChild = temporaryNode->nextSibling;
                }
            }
            free(temporaryNode);
            printf("Directory removed successfully.\n");
            return;
        }
        temporaryNode = temporaryNode->nextSibling;
    } while (temporaryNode != currentDirectory->firstChild);
    printf("No directory found with name %s.\n", dirName);
}

void showDiskUsage()
{
    int availableBlocks = 0;
    DiskBlock *temporaryNode = freeBlockListHead;
    while (temporaryNode)
    {
        availableBlocks++;
        temporaryNode = temporaryNode->next;
    }
    printf("Total blocks: %d\n", TOTAL_DISK_BLOCKS);
    printf("Used blocks: %d\n", TOTAL_DISK_BLOCKS - availableBlocks);
    printf("Free blocks: %d\n", availableBlocks);
    printf("Disk usage: %.2f%%\n", (float)((TOTAL_DISK_BLOCKS - availableBlocks) * 100.0) / TOTAL_DISK_BLOCKS);
}

void freeAllBlocks()
{
    DiskBlock *temporaryNode = freeBlockListHead;
    while (temporaryNode)
    {
        DiskBlock *next = temporaryNode->next;
        free(temporaryNode);
        temporaryNode = next;
    }
    freeBlockListHead = NULL;
}

void releaseAllNodes(Node *node)
{
    if (!node) {
        return;
    }
    Node *child = node->firstChild;
    if (child)
    {
        Node *start = child;
        do
        {
            Node *nextChild = child->nextSibling;
            releaseAllNodes(child);
            child = (nextChild == start) ? NULL : nextChild;
        } while (child);
    }
    if (node->isFolder == 0)
    {
        releaseFileBlocks(node);
    }
    free(node);
}

void exitVFS()
{
    releaseAllNodes(rootDirectory);
    rootDirectory = NULL;
    currentDirectory = NULL;
    freeAllBlocks();
    printf("Memory released. Exiting program...\n");
}

void handleUserInput()
{
    printf("%s > ", currentDirectory->name);
    char input[MAX_INPUT_LENGTH];
    fgets(input, MAX_INPUT_LENGTH, stdin);
    input[strcspn(input, "\n")] = '\0';

    char *command = strtok(input, " ");
    if (command == NULL)
    {
        return;
    }

    if (strcmp(command, "mkdir") == 0)
    {
        char *argument = strtok(NULL, " ");
        if (argument == NULL)
        {
            printf("Specify a directory name.\n");
            return;
        }
        makeDirectory(argument);
    }
    else if (strcmp(command, "cd") == 0)
    {
        char *argument = strtok(NULL, " ");
        if (argument == NULL)
        {
            printf("Specify a directory name.\n");
            return;
        }
        changeDirectory(argument);
    }
    else if (strcmp(command, "cd..") == 0)
    {
        moveToParentDirectory();
    }
    else if (strcmp(command, "ls") == 0)
    {
        listDirectory();
    }
    else if (strcmp(command, "create") == 0)
    {
        char *argument = strtok(NULL, " ");
        if (argument == NULL)
        {
            printf("Specify a file name.\n");
            return;
        }
        createFile(argument);
    }
    else if (strcmp(command, "write") == 0)
    {
        char *file = strtok(NULL, " ");
        char *data = strtok(NULL, "\n");
        if (!file || !data)
        {
            printf("Syntax: write filename \"data\"\n");
            return;
        }
        writeFile(file, data);
    }
    else if (strcmp(command, "read") == 0)
    {
        char *file = strtok(NULL, " ");
        if (!file)
        {
            printf("Specify a file name.\n");
            return;
        }
        readFile(file);
    }
    else if (strcmp(command, "delete") == 0)
    {
        char *file = strtok(NULL, " ");
        if (file == NULL)
        {
            printf("Specify a file name to delete.\n");
            return;
        }
        deleteFileByName(file);
    }
    else if (strcmp(command, "rmdir") == 0)
    {
        char *argument = strtok(NULL, " ");
        if (argument == NULL)
        {
            printf("Specify a directory to remove.\n");
            return;
        }
        removeDirectory(argument);
    }
    else if (strcmp(command, "pwd") == 0)
    {
        printCurrentPath();
    }
    else if (strcmp(command, "df") == 0)
    {
        showDiskUsage();
    }
    else if (strcmp(command, "exit") == 0)
    {
        exitVFS();
        exit(0);
    }
    else
    {
        printf("Invalid command.\n");
    }
}

int main()
{
    initializeVFS();
    while (1)
    {
        handleUserInput();
    }
    return 0;
}
