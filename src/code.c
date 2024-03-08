#include <assert.h>
#include <string.h>

#include "code.h"

#define MAX_SYMBOL_LEN      4      /* Max length of a mnemonic string */


/********************************************************** Data Declarations */

/*
 * These arrays pair the most significant bits of a mnemonic field with its
 * corresponding string, as the tables in section 6.2.2
 */
static const SymbolAddressPair Destinations[] = {
                   /* ddd */
    {0x0,    ""},  /* 000 */
    {0x1,   "M"},  /* 001 */
    {0x2,   "D"},  /* 010 */
    {0x3,  "MD"},  /* 011 */
    {0x4,   "A"},  /* 100 */
    {0x5,  "AM"},  /* 101 */
    {0x6,  "AD"},  /* 110 */
    {0x7, "AMD"},  /* 111 */
};

static const SymbolAddressPair Computations[] = {
                     /* acc cccc */
    {0x2A,     "0"}, /* 010 1010 */
    {0x3F,     "1"}, /* 011 1111 */
    {0x3A,    "-1"}, /* 011 1010 */
    {0x0C,     "D"},                       /* adding 0x40 toggles the `a` bit */
    {0x30,     "A"}, {0x30 + 0x40,  "M"},  /* 011 0000, 111 0000 */
    {0x0D,    "!D"},
    {0x31,    "!A"}, {0x31 + 0x40,  "!M"}, /* 011 0001, 111 0001 */
    {0x0F,    "-D"},
    {0x33,    "-A"}, {0x33 + 0x40,  "-M"},
    {0x1F,   "D+1"},
    {0x37,   "A+1"}, {0x37 + 0x40, "M+1"},
    {0x0E,   "D-1"},
    {0x32,   "A-1"}, {0x32 + 0x40, "M-1"},
    {0x02,   "D+A"}, {0x02 + 0x40, "D+M"},
    {0x13,   "D-A"}, {0x13 + 0x40, "D-M"},
    {0x07,   "A-D"}, {0x07 + 0x40, "M-D"},
    {0x00,   "D&A"}, {0x00 + 0x40, "D&M"},
    {0x15,   "D|A"}, {0x15 + 0x40, "D|M"} 
};

static const SymbolAddressPair Jumps[] = {
    {0x00,    ""},
    {0x01, "JGT"},
    {0x02, "JEQ"},
    {0x03, "JGE"},
    {0x04, "JLT"},
    {0x05, "JNE"},
    {0x06, "JLE"},
    {0x07, "JMP"}
};

static const uint8_t DestSize  = ARRAY_SIZE(Destinations);
static const uint8_t CompSize  = ARRAY_SIZE(Computations);


/******************************************************* Private Declarations */

static inline uint16_t lookup(const char *, const SymbolAddressPair [], 
                              const uint8_t);


/***************************************************** Public Implementations */

/*
 * If found, bit shifts the destination number and sets it in its proper
 * position then returns it.
 */
uint16_t 
code_dest(const char *mnemonic)
{
    uint16_t dest;

    assert(mnemonic != NULL);
    dest = lookup(mnemonic, Destinations, DestSize);

    if (dest == ERROR){
        return ERROR;
    }
    return (uint16_t)(dest << 3);
}

/*
 * If found, bit shifts the computation value then returns it.
 */
uint16_t 
code_comp(const char *mnemonic)
{
    uint16_t comp;

    assert(mnemonic != NULL);
    comp = lookup(mnemonic, Computations, CompSize);

    if (comp == ERROR){
        return ERROR;
    }
    return (uint16_t)(comp << 6);
}

/*
 * Returns the lookup result because no bit shift is needed, jump bits are LSBs.
 */
uint16_t 
code_jump(const char *mnemonic)
{

    assert(mnemonic != NULL);
    return lookup(mnemonic, Jumps, CompSize);

}


/**************************************************** Private implementations */

/* 
 * The three routines of this module use this helper function which returns
 * either the corresponding code for a mnemonic in the array of pairs or the
 * `ERROR` macro.
 */
static inline uint16_t 
lookup(const char *s, const SymbolAddressPair arr[], const uint8_t n)
{
    uint8_t i;

    for (i = 0; i < n; i++) {
        if (strncmp(arr[i].symbol, s, MAX_SYMBOL_LEN) == 0) {
            return arr[i].bits ;
        }
    }
    return ERROR;
}
