/*
 * Brief: Part of the Hack Assembler from Nand2Tetris.
 *
 * Code module interface, as described in Chapter 6 of "The Elements of
 * Computing Systems: Building a Modern Computer from First Principles" by
 * Noam Nisan and Shimon Shocken.
 * 
 * The module is responsible for translating predefined assembly language
 * mnemonics into their corresponding binary codes.  It treats 'binary codes' as
 * unsigned 16-bit integers that represent the set bits of an assembly
 * instruction.
 * 
 * The implementation file `code.c` performs lookups on constant symbol-address
 * pairs arrays and returns a value according to the implementation details, or
 * the `ERROR` macro if applicable.
 *
 * The rationale behind this design is: 
 * + The constant arrays pairs are self-explanatory. 
 * + It simplifies error detection and makes the task of 'assembling' the
 *   `dest`, `comp`, and `jump` fields easy, using bitwise OR, as follows: 
 *
 * For instance, `D|M` is represented as the integer 85, in binary: `1010101`.
 *
 * `code_dest("M")` returns 8, in binary:      `0000 0000 0000 1000`
 * `code_comp("D|M")` returns 5440, in binary: `0001 0101 0100 0000`
 * `code_jump("JMP")` returns 7, in binary:    `0000 0000 0000 0111`
 *                                              -------------------
 * ORing with 0xE000 gives the instruction:    `1111 0101 0100 1111`
 */

#ifndef CODE_H
#define CODE_H

#include <stdint.h>

#include "common/shared_defs.h"

/*
 * Returns a 16-bit decimal embedded with the encoded destination mnemonic
 * argument.  
 * Returns `ERROR` if the provided mnemonic is not a valid destination.
 */
uint16_t 
code_dest(const char *mnemonic);

/*
 * Returns a 16-bit decimal embedded with the encoded computation mnemonic
 * argument.  
 * Returns `ERROR` if the provided mnemonic is not a valid computation.
 */
uint16_t 
code_comp(const char *mnemonic);

/*
 * Returns a 16-bit decimal embedded with the encoded computation mnemonic
 * argument.  
 * Returns `ERROR` if the provided mnemonic is not a valid jump.
 */
uint16_t 
code_jump(const char *mnemonic);

#endif /* CODE_H */
