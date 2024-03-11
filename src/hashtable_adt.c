#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable_adt.h"


/*********************************************************** Data Definitions */

/*
 * A hash table `Entry` is a list in which each element is:
 */
typedef struct entry {
    size_t key_size;          /* Key size */
    char *key;                /* Copy of the key the client used at insertion */
    void *item;               /* A void pointer to the item held */
    struct entry *next;       /* A self-referential pointer */
} Entry;

/*
 * Datatype completion for `HashTableADT`:
 */
struct hash_table_type {
    size_t curr_buckets;    /* Current number of buckets */
    size_t init_buckets;    /* Initial number of buckets */
    size_t nelems;          /* Number of elements */
    HashFunction *hash;     /* Pointer to function of type `HashFunction` */
    Entry **entries;        /* Dynamically allocated array of `Entry` */
};


/******************************************************* Private Declarations */ 

static inline size_t calculate_key_hash(HashTableADT *, const void *, size_t);
static inline bool is_prime(size_t);
static inline size_t get_next_prime(size_t);


/***************************************************** Public Implementations */

/*
 * Create a hash table 
 */
HashTableADT *
cadthashtable_new(size_t nbuckets, HashFunction *fp)
{
    HashTableADT *new;

    if (nbuckets == 0 || fp == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    if ((new = malloc(sizeof(struct hash_table_type))) == NULL) { 
        perror("cadthashtable_new malloc failed allocating struct hash_table_type");
        errno = ENOMEM;
        return NULL;
    }
    
    if ((new->entries = calloc(nbuckets, sizeof(Entry*))) == NULL) {
        perror("cadthashtable_new calloc failed allocating entry array");
        free(new);
        errno = ENOMEM;
        return NULL;
    }

    nbuckets = get_next_prime(nbuckets);

    new->curr_buckets = nbuckets;
    new->init_buckets = nbuckets;
    new->nelems = 0;
    new->hash = fp;

    return new;
}
    
/*
 * Destroy hash table
 */
void 
cadthashtable_destroy(HashTableADT *ht) 
{
    free(ht->entries);
    free(ht);
    return;
}

/*
 * Insert operation
 */
void *
cadthashtable_insert(HashTableADT *ht, const void *key, size_t key_size, void *e) 
{
    size_t index;
    Entry* new;

    if (ht == NULL || key == NULL || key_size == 0 || e == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (cadthashtable_lookup(ht, key, key_size) != NULL) {
        errno = EEXIST;
        return NULL;
    }

    if ((new = malloc(sizeof(Entry))) == NULL) { 
        perror("cadthashtable_insert malloc failed allocating struct entry");
        errno = ENOMEM;
        return NULL;
    }
    if ((new->key = malloc(key_size)) == NULL) {
        perror("cadthashtable_insert malloc failed allocating new->key");
        free(new);
        errno = ENOMEM;
        return NULL;
    }

    index = calculate_key_hash(ht, key, key_size);
    memcpy(new->key, key, key_size);
    new->key_size = key_size; 
    new->item = e; 

    /* Insert entry */
    new->next = ht->entries[index];
    ht->entries[index] = new;
    ht->nelems++;

    return e;
}

/*
 * Lookup and return item, no removal.
 */
void *
cadthashtable_lookup(HashTableADT *ht, const void *key, size_t key_size)
{
    size_t index;
    Entry *e;

    if (ht == NULL || key == NULL || key_size == 0) {
        errno = EINVAL;
        return NULL;
    }

    index = calculate_key_hash(ht, key, key_size);
    e = ht->entries[index];

    if (e == NULL) {
        return NULL;
    }

    while (e != NULL) {
        if (e->key_size == key_size && memcmp(e->key, key, key_size) == 0) {
            break;
        }
        e = e->next;
    }

    if (e == NULL) {
        return NULL;
    }
    else {
        return e->item;
    }
}

/*
 * Removes a (possible chained) entry from the entries array returning the
 * entry's item on success.
 */
void *
cadthashtable_delete(HashTableADT *ht, void *key, size_t key_size, void *e)
{
	size_t index;
    Entry **pp;
    void *deleted_item;

    if (ht == NULL || key == NULL || key_size == 0 || e == NULL) {
        errno = EINVAL;
        return NULL;
    }

    index = calculate_key_hash(ht, key, key_size);
    pp = &(ht->entries[index]);

	while (*pp) {
        if ((*pp)->key_size == key_size && memcmp((*pp)->key, key, key_size) == 0) {
            Entry *temp = *pp;
            *pp = temp->next;
            deleted_item = temp->item;

            free(temp->key);
            free(temp);
            ht->nelems--;

            return deleted_item;
        }
        pp = &((*pp)->next);
    }

	return NULL;
}

/**************************************************** Private Implementations */ 

/*
 * Returns the hash index for a given key using the hash function specified by
 * the client when creating the hash table.
 */
static inline size_t 
calculate_key_hash(HashTableADT *ht, const void *key, size_t key_size)
{
    size_t index;

    index = (ht->hash(key, key_size)) % ht->curr_buckets;

    return index;
}

/*
 * Primality test
 */
static inline bool 
is_prime(size_t n)
{
    size_t i; 

    if (n == 2 || n == 3) {
        return true;
    }

    if (n <= 1 || n % 2 == 0 || n % 3 == 0) {
        return false;
    }

    for (i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

/*
 * Returns the closest prime greater than or equal to `n`.
 */
static inline size_t 
get_next_prime(size_t n)
{
    while (is_prime(n) == false) {
        n++;
    }
    return n;
}
