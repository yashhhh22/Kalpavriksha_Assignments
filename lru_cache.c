#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 2048
#define HASH_TABLE_BUCKETS 2003

typedef struct CacheNode
{
    int key;
    char *value;
    struct CacheNode *previous;
    struct CacheNode *next;
} CacheNode;

typedef struct HashNode
{
    int key;
    CacheNode *node;
    struct HashNode *next;
} HashNode;

typedef struct LRUCache
{
    int capacity;
    int size;
    CacheNode *head;
    CacheNode *tail;
    HashNode *buckets[HASH_TABLE_BUCKETS];
} LRUCache;

static char* string_duplicate(const char *source)
{
    if (source == NULL)
    {
        return NULL;
    }

    size_t length = strlen(source);
    char *copy = (char *) malloc(length + 1);
    if (copy == NULL)
    {
        return NULL;
    }

    memcpy(copy, source, length + 1);
    return copy;
}

static unsigned int bucket_index_for_key(int key)
{
    unsigned int unsigned_key = (unsigned int) key;
    unsigned_key ^= (unsigned_key >> 16);
    return (unsigned int) (unsigned_key % HASH_TABLE_BUCKETS);
}

static void trim_whitespace(char *text)
{
    if (text == NULL)
    {
        return;
    }

    char *read = text;
    while (*read != '\0' && isspace((unsigned char) *read))
    {
        read++;
    }

    if (read != text)
    {
        memmove(text, read, strlen(read) + 1);
    }

    size_t length = strlen(text);
    while (length > 0 && isspace((unsigned char) text[length - 1]))
    {
        text[--length] = '\0';
    }
}

static int is_valid_int_string(const char *string)
{
    if (string == NULL || *string == '\0')
    {
        return 0;
    }

    const char *pointerToString = string;
    if (*pointerToString == '+' || *pointerToString == '-')
    {
        pointerToString++;
    }

    if (*pointerToString == '\0')
    {
        return 0;
    }

    while (*pointerToString)
    {
        if (!isdigit((unsigned char)*pointerToString))
        {
            return 0;
        }
        pointerToString++;
    }

    return 1;
}

static HashNode* hash_find_entry(LRUCache *cache, int key)
{
    unsigned int index = bucket_index_for_key(key);
    HashNode *iterator = cache->buckets[index];

    while (iterator != NULL)
    {
        if (iterator->key == key)
        {
            return iterator;
        }
        iterator = iterator->next;
    }

    return NULL;
}

static void hash_insert_entry(LRUCache *cache, int key, CacheNode *node)
{
    unsigned int index = bucket_index_for_key(key);
    HashNode *iterator = cache->buckets[index];

    while (iterator != NULL)
    {
        if (iterator->key == key)
        {
            iterator->node = node;
            return;
        }
        iterator = iterator->next;
    }

    HashNode *newEntry = (HashNode *) malloc(sizeof(HashNode));
    if (newEntry == NULL)
    {
        fprintf(stderr, "hash_insert_entry: out of memory\n");
        exit(EXIT_FAILURE);
    }

    newEntry->key = key;
    newEntry->node = node;
    newEntry->next = cache->buckets[index];
    cache->buckets[index] = newEntry;
}

static void hash_remove_entry(LRUCache *cache, int key)
{
    unsigned int index = bucket_index_for_key(key);
    HashNode *iterator = cache->buckets[index];
    HashNode *previous = NULL;

    while (iterator != NULL)
    {
        if (iterator->key == key)
        {
            if (previous == NULL)
            {
                cache->buckets[index] = iterator->next;
            }
            else
            {
                previous->next = iterator->next;
            }
            free(iterator);
            return;
        }

        previous = iterator;
        iterator = iterator->next;
    }
}

static void move_node_to_front(LRUCache *cache, CacheNode *node)
{
    if (node == NULL || cache->head == node)
    {
        return;
    }

    if (node->previous != NULL)
    {
        node->previous->next = node->next;
    }

    if (node->next != NULL)
    {
        node->next->previous = node->previous;
    }

    if (cache->tail == node)
    {
        cache->tail = node->previous;
    }

    node->previous = NULL;
    node->next = cache->head;

    if (cache->head != NULL)
    {
        cache->head->previous = node;
    }

    cache->head = node;

    if (cache->tail == NULL)
    {
        cache->tail = node;
    }
}

static void insert_node_front(LRUCache *cache, CacheNode *node)
{
    node->previous = NULL;
    node->next = cache->head;

    if (cache->head != NULL)
    {
        cache->head->previous = node;
    }

    cache->head = node;

    if (cache->tail == NULL)
    {
        cache->tail = node;
    }
}

static CacheNode* remove_tail_node(LRUCache *cache)
{
    CacheNode *removed = cache->tail;

    if (removed == NULL)
    {
        return NULL;
    }

    if (removed->previous != NULL)
    {
        cache->tail = removed->previous;
        cache->tail->next = NULL;
    }
    else
    {
        cache->head = NULL;
        cache->tail = NULL;
    }

    removed->previous = NULL;
    removed->next = NULL;

    return removed;
}

LRUCache* lru_create(int capacity)
{
    if (capacity <= 0)
    {
        return NULL;
    }

    LRUCache *cache = (LRUCache*) malloc(sizeof(LRUCache));
    if (cache == NULL)
    {
        return NULL;
    }

    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;

    for (int bucket = 0; bucket < HASH_TABLE_BUCKETS; bucket++)
    {
        cache->buckets[bucket] = NULL;
    }

    return cache;
}

char* lru_get(LRUCache *cache, int key)
{
    if (cache == NULL)
    {
        return NULL;
    }

    HashNode *entry = hash_find_entry(cache, key);
    if (entry == NULL)
    {
        return NULL;
    }

    CacheNode* found = entry->node;
    move_node_to_front(cache, found);
    return found->value;
}

void lru_put(LRUCache *cache, int key, const char *value)
{
    if (cache == NULL)
    {
        return;
    }

    HashNode *existing = hash_find_entry(cache, key);

    if (existing != NULL)
    {
        char *newCopy = string_duplicate(value);

        if (newCopy != NULL)
        {
            free(existing->node->value);
            existing->node->value = newCopy;
        }

        move_node_to_front(cache, existing->node);
        return;
    }

    CacheNode *node = (CacheNode *) malloc(sizeof(CacheNode));
    if (node == NULL)
    {
        fprintf(stderr, "lru_put: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    node->key = key;
    node->value = string_duplicate(value);
    node->previous = NULL;
    node->next = NULL;

    insert_node_front(cache, node);
    hash_insert_entry(cache, key, node);
    cache->size++;

    if (cache->size > cache->capacity)
    {
        CacheNode *toRemove = remove_tail_node(cache);

        if (toRemove != NULL)
        {
            hash_remove_entry(cache, toRemove->key);
            free(toRemove->value);
            free(toRemove);
            cache->size--;
        }
    }
}

void lru_free(LRUCache *cache)
{
    if (cache == NULL)
    {
        return;
    }

    CacheNode *current = cache->head;

    while (current != NULL)
    {
        CacheNode *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }

    for (int bucket = 0; bucket < HASH_TABLE_BUCKETS; bucket++)
    {
        HashNode *entry = cache->buckets[bucket];

        while (entry != NULL)
        {
            HashNode *nextEntry = entry->next;
            free(entry);
            entry = nextEntry;
        }
    }

    free(cache);
}

int main(void)
{
    char line[MAX_LINE_LENGTH];
    LRUCache *cache = NULL;

    while (fgets(line, sizeof(line), stdin) != NULL)
    {
        trim_whitespace(line);

        if (strlen(line) == 0)
        {
            continue;
        }

        char *command = strtok(line, " \t");

        if (command == NULL)
        {
            continue;
        }

        if (strcmp(command, "exit") == 0)
        {
            break;
        }
        else if (strcmp(command, "createCache") == 0)
        {
            char *capitalString = strtok(NULL, " \t");

            if (capitalString == NULL)
            {
                fprintf(stderr, "createCache requires capacity\n");
                continue;
            }

            int capacity = atoi(capitalString);

            if (cache != NULL)
            {
                lru_free(cache);
            }

            cache = lru_create(capacity);

            if (cache == NULL)
            {
                fprintf(stderr, "Failed to create cache\n");
                return 1;
            }
        }
        else if (strcmp(command, "put") == 0)
        {
            if (cache == NULL)
            {
                fprintf(stderr, "Cache not created. Use createCache <capacity>\n");
                continue;
            }

            char *keyString = strtok(NULL, " \t");
            char *valueRest = strtok(NULL, "");

            if (keyString == NULL || valueRest == NULL)
            {
                fprintf(stderr, "put requires key and value\n");
                continue;
            }

            trim_whitespace(valueRest);

            if (!is_valid_int_string(keyString))
            {
                fprintf(stderr, "incorrect key, key must be an integer\n");
                continue;
            }

            int key = atoi(keyString);
            lru_put(cache, key, valueRest);
        }
        else if (strcmp(command, "get") == 0)
        {
            if (cache == NULL)
            {
                fprintf(stderr, "Cache not created. Use createCache <capacity>\n");
                continue;
            }

            char *keyStr = strtok(NULL, " \t");

            if (keyStr == NULL)
            {
                fprintf(stderr, "get requires key\n");
                continue;
            }

            if (!is_valid_int_string(keyStr))
            {
                fprintf(stderr, "Incorrect key. Key must be an integer\n");
                continue;
            }

            int key = atoi(keyStr);
            char *value = lru_get(cache, key);

            if (value != NULL)
            {
                printf("%s\n", value);
            }
            else
            {
                printf("NULL\n");
            }
        }
        else
        {
            fprintf(stderr, "Unknown command: %s\n", command);
        }
    }

    if (cache != NULL)
    {
        lru_free(cache);
    }

    return 0;
}
