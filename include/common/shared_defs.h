/*
 * Part of the Hack Assembler from Nand2Tetris.
 *
 * This file groups together shared definitions required across the codebase
 * and includes reference comments explaining their purpose.
 */
#ifndef SARED_DEFS_H
#define SARED_DEFS_H

#include <stdint.h>

#define ERROR       0x8000    /* Decimal -32768 reserved as error code */

#define ARRAY_SIZE(x) ((sizeof (x)) / (sizeof (*x)))

/*
 * This aggregate data type serves as a container for grouping a symbol with its
 * corresponding binary representation. 
 *
 * The `code.c` and `symboltable.c` implementations use this typedef to
 * declare constant arrays that associate predefined command strings with their
 * corresponding binary instruction translations.
 */
typedef struct SymbolAddressPair {
    uint16_t  bits;           /* Decimal that translates to the binary symbol */
    char     *symbol;         /* i.e.: JGT, D|M, (LABEL), @variable, etc.  */
} SymbolAddressPair;

#endif /* SHARED_DEFS_H */
