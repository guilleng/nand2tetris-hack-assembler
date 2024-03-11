#define _POSIX_C_SOURCE 200809L     /* strdup() */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common/shared_defs.h"
#include "hashtable_adt.h"          /* github.com/guilleng/c-adts */
#include "symboltable.h"

/*
 * Rationale behind the manifest constant `MAX_SYMBOL` - Permissible environment
 * SymbolAddressPairs.
 *
 * In addition to determining the initial size of the hash table to allocate,
 * it imposes an upper bound on the number of symbol-value pairs that the
 * compilation process can introduce at runtime.
 * 
 * This constant simplifies the task of monitoring environment symbols, which is
 * crucial for the proper deallocation of the abstract data type (ADT). 
 * 
 * The choice of the value is influenced by reference.  The file `Pong.asm` 
 * declares approximately 900 distinct identifiers.  Determined by the 
 * command `cat Pong.asm | grep ^\( | sort | uniq | wc --lines`
 */
#define MAX_SYMBOL    2048    


/********************************************************** Data declarations */

HashTableADT *Table = NULL;

static SymbolAddressPair ProgramSymbols[MAX_SYMBOL];  /* Runtime environment */
static uint16_t SymbolCount = 0;

static const SymbolAddressPair PredefinedSymbols[] = {
    {0x0000, "R0"},     {0x0000, "SP"},  
    {0x0001, "R1"},     {0x0001, "LCL"}, 
    {0x0002, "R2"},     {0x0002, "ARG"},
    {0x0003, "R3"},     {0x0003, "THIS"},
    {0x0004, "R4"},     {0x0004, "THAT"},
    {0x0005, "R5"},
    {0x0006, "R6"},
    {0x0007, "R7"},
    {0x0008, "R8"},     
    {0x0009, "R9"},     
    {0x000A, "R10"},
    {0x000B, "R11"},
    {0x000C, "R12"},    
    {0x000D, "R13"},    
    {0x000E, "R14"},    
    {0x000F, "R15"},    
    {0x4000, "SCREEN"},
    {0x6000, "KBD"},
};


/******************************************************* Private Declarations */

static inline HashFunction djb2;


/***************************************************** Public Implementations */

/*
 * Failure to insert a predefined symbol results in main program termination.  
 */
void 
symbol_table_init(void)
{
    uint16_t i;

    Table = cadthashtable_new(MAX_SYMBOL, djb2);

    if (Table == NULL) {
        perror("symbol_table_init cadthashtable_new");
        return;
    }

    for (i = 0; i < ARRAY_SIZE(PredefinedSymbols); i++) {
        errno = 0;
        cadthashtable_insert(Table, PredefinedSymbols[i].symbol, 
                              strlen(PredefinedSymbols[i].symbol)+1,
                              (void *)&PredefinedSymbols[i].bits);
        if (errno != 0) {
            perror("symbol_table_init cadthashtable_insert");
            errno = ENOTRECOVERABLE;
            return;
        }
    }
}

/*
 * Set `errno` and returns if `MAX_SYMBOL` is reached or an error occurs.  Makes
 * a copy of the new symbol and stores it, if possible, with its corresponding
 * address.  
 */
void 
symbol_table_add_entry(const char *symbol, const uint16_t addr)
{
    char *p;

    if (SymbolCount == MAX_SYMBOL - ARRAY_SIZE(PredefinedSymbols)) {
        fprintf(stderr, "MAX_SYMBOL reached");
        errno = EPERM;
        return;
    }

    errno = 0;
    if ((p = strdup(symbol)) == NULL) {
        perror("symbol_add_entry strdup");
        errno = ENOTRECOVERABLE;
        return;
    }

    ProgramSymbols[SymbolCount].symbol = p;
    ProgramSymbols[SymbolCount].bits = addr;

    errno = 0;
    cadthashtable_insert(Table, symbol, strlen(symbol)+1,
                            &ProgramSymbols[SymbolCount].bits);
    if (errno == EEXIST) {
        fprintf(stderr, "symbol_add_entry duplicate symbol");
        free(p);
        errno = ENOTRECOVERABLE;
        return;
    }
    if (errno != 0) {
        fprintf(stderr, "symbol_add_entry cadthashtable_insert");
        free(p);
        errno = ENOTRECOVERABLE;
        return;
    }

    SymbolCount++;
}


bool symbol_table_contains(const char *symbol)
{
    return cadthashtable_lookup(Table, symbol, strlen(symbol)+1) != NULL;
}

/*
 * One of the reasons for encapsulating the symbol table within an additional
 * module is to hide the need for explicit type casting improving readability.
 */
uint16_t symbol_table_get_addr(const char *symbol)
{
    uint16_t *addr;

    assert(symbol != NULL);

    addr = (uint16_t*)cadthashtable_lookup(Table, symbol, strlen(symbol)+1);

    if (addr == NULL) {
        return ERROR;
    }
    return *addr;
}

/*
 * Memory deallocation:
 *
 * The first loop eliminates runtime symbols.  Always safe to execute. If no
 * symbols have been loaded into the table, the condition guards against any
 * issues.
 *
 * The second loop checks if the symbol is in the table before each deletion 
 * for avoiding a potential crash.  Observe this routine can be called when  
 * `symbol_table_init` only partially loads the predefined symbols.
 *
 * The final check against `NULL` is in place because this function might be
 * called on an uninitialized symbol table.
 */
void symbol_table_destroy(void) 
{ 
    uint16_t i;

    for (i = 0; i < SymbolCount; i++)
    {
        void * e;
        errno = 0;
        e = cadthashtable_delete(Table, ProgramSymbols[i].symbol, 
                                  strlen(ProgramSymbols[i].symbol)+1,
                                  &ProgramSymbols[i].bits);

        assert(e == &ProgramSymbols[i].bits);

        free(ProgramSymbols[i].symbol);
    }

    for (i = 0; i < ARRAY_SIZE(PredefinedSymbols); i++) {
        if (symbol_table_contains(PredefinedSymbols[i].symbol)) {
        errno = 0;
        (cadthashtable_delete(Table, PredefinedSymbols[i].symbol, 
                               strlen(PredefinedSymbols[i].symbol)+1,
                               (void *) &PredefinedSymbols[i].bits));
        } else {
            break;
        }
    }
    
    if (Table != NULL) {
        cadthashtable_destroy(Table);
    }
}


/**************************************************** Private implementations */

/*
 * Hash function for the ADT.  Slightly modified to match the signature
 * required.  
 * http://www.cse.yorku.ca/~oz/hash.html
 */
size_t 
djb2(const void *str, size_t keysize)
{
    unsigned long hash = 5381;
    int c;
    char *key = (char*)str;

    (void) keysize;

    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + (unsigned long)c; /* hash * 33 + c */
    }

    return hash;
}
