/* 
 * Hack Assembler implementation, as described in Chapter 6 of "The Elements of
 * Computing Systems: Building a Modern Computer from First Principles" by Noam
 * Nisan and Shimon Shocken.
 */
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code.h"
#include "common/shared_defs.h"
#include "parser.h"
#include "symboltable.h"

#define WORD_WIDTH 16               /* Bits per instruction */


/********************************************************** Data Declarations */

static FILE *OutputStream;
static uint16_t BaseAddress, Instruction, InstructionNumber;


/******************************************************* Private Declarations */

void process_l_instructions(void);
void process_a_or_c_instruction(void);
void open_output_stream(char *);
uint16_t num_to_address(const char *);
void write_to_binary_stream(void);
void die(void);


/**************************************************** Private Implementations */


/*
 * Main routine implemented as described in section 6.3.5 "Assembler for
 * Programs with Symbols", following a two passes approach. 
 */

#ifndef MINUNIT_MINUNIT_H
int 
main(int argc, char *argv[])
{

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input.asm>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (parser_init(argv[1]) == NULL) {
        exit(EXIT_FAILURE);
    }
    
    /* 
     * First pass:
     */
    open_output_stream(argv[1]);      /* Set up output stream and environment */
    InstructionNumber = 0;
    errno = 0;
    symbol_table_init();
    if (errno != 0) {
        die();
    }

    for ( ; ; ) {
        parser_advance();
        if (errno != 0) {
            die();
        }
        if (parser_has_more_commands() && errno == 0){
            process_l_instructions();
            if (errno != 0) {
                die();
            }
        } else {
            break;
        }

    }

    if (errno != 0) {
        die();
    }


    /* 
     * Second pass:
     */
    parser_rewind();                       /* Set environment for second pass */
    InstructionNumber = 0;
    BaseAddress = 15;

    for ( ; errno == 0 && Instruction != ERROR; ) {
        parser_advance();
        if (errno != 0) {
            die();
        }
        if (parser_has_more_commands() && errno == 0){
            process_a_or_c_instruction();
        } else {
            break;
        }
    }

    if (Instruction == ERROR || errno != 0) {
        die();
    }
    symbol_table_destroy();
    parser_destroy();
    fclose(OutputStream);
    return EXIT_SUCCESS;
}

#endif /* MINUNIT_MINUNIT_H */

/*
 * Handles labels, generating the symbol table.  In case an error occurs while
 * adding a symbol, leaves `errno` set at returning.  The controlling loop can
 * then halt the translation process immediately.
 */
void process_l_instructions(void)
{
    if (parser_get_command_type() == L_COMMAND) {
        symbol_table_add_entry(parser_symbol(), InstructionNumber);
    } else {
        InstructionNumber++;
    }
}

/*
 * Handles A and C instructions according to the specifications.  
 * It resets the `Instruction` variable to 0x0 at each call. After the routine
 * completes, it writes an `Instruction` to the stream, even if the `ERROR`
 * macro was returned or an error occurred in the translation process.  
 * This behavior allows the controlling loop to halt translation upon an error.
 */
void process_a_or_c_instruction(void)
{
    const char *tkn;
    bool num, sym;

    Instruction = 0x0;

    switch (parser_get_command_type()) {
    case A_COMMAND:
        tkn = parser_symbol();
        sym = isalpha(*tkn);
        num = isdigit(*tkn);
        
        if (sym && !num) {
            if (symbol_table_contains(tkn)) {
                Instruction = symbol_table_get_addr(tkn);
            } else {
                symbol_table_add_entry(tkn, ++BaseAddress);
                if (errno != 0) {
                    Instruction = ERROR;
                } else {
                    Instruction = BaseAddress;
                }
            }
        } else { 
            Instruction = num_to_address(tkn);
        }
        write_to_binary_stream();
        break;

    case C_COMMAND:
        Instruction |= code_dest(parser_dest());
        Instruction |= code_comp(parser_comp());
        Instruction |= code_jump(parser_jump());
        Instruction |= 0xE000;
        write_to_binary_stream();
        break;

    default:
        break;
    }
}

/*
 * Opens a file stream for writing after setting an appropriate filename.
 */
void open_output_stream(char *dotasm)
{
    char *dothack, *ext;
    char *newext; 
    
    newext = ".hack";
    ext = strrchr(dotasm, '.');
    *ext = '\0';

    if ((dothack = malloc(strlen(dotasm) + sizeof(newext))) == NULL) {
        perror("malloc dothack");
        exit(EXIT_FAILURE);
    }
    sprintf(dothack, "%s%s", dotasm, newext);

    OutputStream = fopen(dothack , "w");
    free(dothack);
    if (OutputStream == NULL) {
        perror("open_OutputStream");
        exit(EXIT_FAILURE);
    }
}

/*
 * Converts a string to an unsigned 16 bit, integer. It returns `ERROR` on
 * overflow or underflow.
 */
uint16_t num_to_address(const char *s)
{
    long val;
    char *endptr;

    errno = 0;
    val = strtol(s, &endptr, 10);
    if (errno != 0) {
        perror("strtol");
        return ERROR;
    }
    if (endptr == s || val < 0 || val >= ERROR) {
        return ERROR;
    }
    return (uint16_t)val;
}

/*
 * Writes the codified binary `Instruction` to `OutputStream` and increments the
 * instruction counter `InstructionNumber`.
 */
void write_to_binary_stream(void)
{
    uint16_t mask; 
    uint8_t i;
    
    mask = 0x8000;

    for (i = 0; i < WORD_WIDTH; i++) {
        fputc((Instruction & mask) ? '1' : '0', OutputStream);
        mask >>= 1;
    }
    fputc('\n', OutputStream);
    InstructionNumber++;
}

/*
 * Releases resources allocated by the program on abnormal termination. By
 * setting `errno` before calling `parser_destroy()`, the routine prints a
 * message with the current line being parsed.
 */
void die(void)
{

    fprintf(stderr, "Instruction %d.\n", InstructionNumber);
    symbol_table_destroy();
    errno = ENOTRECOVERABLE;
    parser_destroy();
    if (fclose(OutputStream) == EOF) {
        perror("die");
    }
    exit(EXIT_FAILURE);
}
