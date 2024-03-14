/*
 * Part of the Hack Assembler from Nand2Tetris.
 *
 * Parser module interface, as described in Chapter 6 of "The Elements of
 * Computing Systems: Building a Modern Computer from First Principles" by
 * Noam Nisan and Shimon Shocken.
 *
 * This module accesses the input code managing its fields and symbols.
 * 
 * Its implementation `parser.c` relies on a set of local variables that model
 * the status of the parsing process.  All operations over its variables are
 * abstracted away and accessed through these interface functions.  Strings
 * returned are null-terminated.
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

/*
 * This typedef'd enum serves as a bridge to ensure that both the main program
 * and the parser module are on the same page regarding the type of command
 * currently being parsed.
 */
typedef enum {
    A_COMMAND,     /* @Xxx where Xxx is either a symbol or a decimal number */
    C_COMMAND,     /* dest=comp;jump */
    L_COMMAND      /* (Xxx) where Xxx is a symbol */
} CommandType;

/*
 * Initializes the parser to read from the stream `filename`.
 * Returns `NULL` on failure.
 */
void *
parser_init(char *filename);

/*
 * Reads the next command from the input setting `errno` on error.
 */
void
parser_advance(void);

/*
 * Returns `false` if there are no more commands or if an error occurred while
 * reading the stream.
 */
bool
parser_has_more_commands(void);

/*
 * Returns the type of the current command, as defined in the `CommandType`
 * enumeration.
 */
CommandType
parser_get_command_type(void);

/*
 * Returns a pointer to the symbol or decimal of the current command @Xxx or
 * (Xxx).  Should be called strictly only on `A_COMMAND` or `L_COMMAND`.  
 */
const char *
parser_symbol(void);  

/*
 * Returns a pointer to the `dest` field of the current C-instruction.  If the
 * field is empty returns a pointer to the empty string.  
 * Should be called strictly first on C_COMMAND.
 */
const char *
parser_dest(void);   

/*
 * Returns a pointer to the `comp` field of the current C-instruction.  
 * Should be called strictly second on C_COMMAND. 
 */
const char *
parser_comp(void);

/*
 * Returns a pointer to the `jump` field of the current C-instruction.  If the
 * field is empty returns a pointer to the empty string.  
 * Should be called strictly third on C_COMMAND. 
 */
const char *
parser_jump(void);

/*
 * Repositions the parser at beginning of the file. 
 */
void
parser_rewind(void);

/*
 * Releases memory and closes the stream associated with the parser.
 */
void
parser_destroy(void);

#endif /* PARSER_H */
