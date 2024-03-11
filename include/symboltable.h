/*
 * Brief: Part of the Hack Assembler from Nand2Tetris.
 *
 * This is the SymbolTable module, as described in Chapter 6 of "The Elements of
 * Computing Systems: Building a Modern Computer from First Principles" by Noam
 * Nisan and Shimon Shocken.
 *
 * It encapsulates access to an Abstract Data Type where the application keeps
 * track of both predefined and runtime symbols.  It is one of the more
 * intricate parts of the codebase.
 *
 * Initially, I considered incorporating the ADT directly into `hackassembler.c`
 * for direct access.  But due to its interface design, direct inclusion in code
 * that was less aesthetically pleasing.  Therefore, for better worse, I
 * introduced another layer of abstraction.  The resultant code is more modular
 * and more enjoyable to read, but it might be somewhat challenging to
 * comprehend.
 *
 * In `symboltable.c`, a Hash Table ADT is allocated, in which the predefined
 * symbols are "loaded" at initialization.  When a "new" symbol-address pair
 * arrives, it is first stored in an array of `SymbolAddressPairs` before being
 * added to the hash table, which only maintains pointers.  
 * Both `contains()` and `get_addr()` are simple lookup operations.
 */
#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Create and initialize symbol table with predefined symbols. Sets `errno` on
 * failure.
 */
void 
symbol_table_init(void);

/*
 * Adds the pair `symbol`-`addr` to the table.  Does not allows duplicates. Sets
 * `errno` on failure.  
 */
void 
symbol_table_add_entry(const char *symbol, const uint16_t addr);

/*
 * Returns true if the table contains the given `symbol`.
 */
bool 
symbol_table_contains(const char *symbol);

/*
 * Returns the address associated with the string `symbol`.  The `ERROR` macro
 * if the symbol is not in the table.  
 */
uint16_t 
symbol_table_get_addr(const char *symbol);

/*
 * Deallocate table.
 */
void 
symbol_table_destroy(void);

#endif /* SYMBOLTABLE_H */
