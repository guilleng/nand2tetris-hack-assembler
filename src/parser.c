#define _POSIX_C_SOURCE 200809L     /* getline() */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"


/********************************************************** Data declarations */

static ssize_t line_len;               /* Buffer length */
static CommandType command;            /* Current instruction type */
static int line_num;                   /* Line number */
static char *buffer;                   /* Pointer to the current buffer */
static char *token;                    /* Pointer to the token being parsed */
static FILE *fp;                       /* File stream */


/******************************************************* Private Declarations */

static inline bool is_extension_asm (const char *);
static inline void discard_leading_withe_spaces(void);
static inline bool is_blank_line (void);
static inline bool is_comment_line (void);


/***************************************************** Public Implementations */

/*
 * Initializes the parser to read from the stream `filename`, after checking the
 * file's extension. 
 */
void *
parser_init(char *filename)
{
    FILE *file;

    if (filename == NULL) {
        return NULL;
    }
    if (!is_extension_asm(filename)) {
        perror("parser_init invalid filetype");
        return NULL;
    }
    if ((file = fopen(filename , "r")) == NULL) {
        perror("parser_init");
        return NULL;
    }

    line_len = 0;
    line_num = 0;
    command = -1;
    buffer = NULL;
    token = NULL;
    fp = file;

    return &line_len;
}

/*
 * Frees any memory in use by the line buffer, setting it to NULL to avoid a
 * possible double free.  Reads the next line from the input. 
 */
void 
parser_advance(void)
{
    ssize_t len = 0;
    size_t bufsize = 0;
    char *line = NULL;

    assert(line_len != -1);

    if (feof(fp)) {
        return;
    }

    if (ferror(fp)) {
        perror("parser_advance");
        errno = ENOTRECOVERABLE;
        return;
    }

    free(buffer);
    buffer = NULL;  /* Double free if there is an error reading the line */

    if ((len = getline(&line, &bufsize, fp)) == -1 && ferror(fp)) {
        free(line);
        perror("parser_advance, getline()");
        errno = ENOTRECOVERABLE;
        return;
    }

    buffer = token = line;
    line_len = len;
    line_num++;

}

/*
 * This routine advances us to the first non-white character of an uncommented
 * line.
 *
 * This function is *tightly coupled* with `discard_line_leading_white_spaces`,
 * `is_blank_line`, `is_comment_line`, and `parser_advance`. 
 *
 * Returns false at EOF or when an error occurs while reading the stream.
 * Returns true if there are commands in the input. The function calls itself
 * recursively on blank and comment lines, effectively skipping them.
 */
bool 
parser_has_more_commands(void)
{

    if (line_len == -1 && feof(fp)) {
        return false;
    }

    discard_leading_withe_spaces();
    if (is_blank_line() || is_comment_line()) {
        errno = 0;
        parser_advance();
        if (errno != 0) {
            errno = ENOTRECOVERABLE;
            return false;
        }
        parser_has_more_commands();
    }

    return true;
}

/*
 * Returns the type of the current command. 
 */
CommandType 
parser_get_command_type(void)
{

    assert(token != NULL);
    assert(*token);

    if (*token == '@') {
        command = A_COMMAND;
        return A_COMMAND;
    } else if (*token == '(') {
        command = L_COMMAND;
        return L_COMMAND;
    } else {
        command = C_COMMAND;
        return C_COMMAND;
    }
}

/*
 * Returns a pointer to the Symbol or Decimal Xxx field of the current A or L
 * command.   @Xxx or (Xxx).  
 */
const char *
parser_symbol(void)
{
    char *p;

    assert(command != C_COMMAND);
    assert(token != NULL);
    assert(*token);
    p = NULL;

    if (command == A_COMMAND) {
        p = strtok(token, "@ \t\n\v\f\r");
    }
    if (command == L_COMMAND) {
        p = strtok(token, "()");
    }
    if (p) {
        while (*(token)++); 
        return p;
    }

    return token;
}

/*
 * Returns a string to the C-instruction's `dest` field.  Called strictly first
 * in a C-instruction.
 */
const char *
parser_dest(void)
{
    char *p, *q;

    assert(command == C_COMMAND);
    assert(token != NULL);
    assert(*token);

    if ((p = strchr(token, '='))) {
        *p = '\0';
        q = token;
        token = p + 1;
        return q;
    }

    return "";
}

/*
 * Returns a string to the C-instruction's `comp` field.  Called strictly after
 * `parser_dest()`
 */
const char *
parser_comp(void)
{
    char *p, *q;
    
    assert(command == C_COMMAND);
    assert(token != NULL);
    assert(*token);

    if ((p = strchr(token, ';'))) {
        *p = '\0';
        q = token;
        token = p + 1;
        return q;
    } else {
        p = strtok(token, " \t\n\v\f\r");
    }

    assert(p != NULL);
    while (*(token)++) {
        ;
    }

    return p;
}

/*
 * Return a string to the C-instruction's jump field. Called strictly after
 * `parser_comp()`.
 */
const char *
parser_jump(void)
{
    char *p;

    assert(command == C_COMMAND);
    assert(token != NULL);

    if ((p = strtok(token, " \t\n\v\f\r")) && *p == 'J') {
        return p;
    }
    return "";
}

/*
 * Sets the parser for the second pass.
 */
void 
parser_rewind(void)
{

    rewind(fp);
    buffer = NULL;
    token = NULL;
    line_len = 0;
    line_num = 0;
    command = -1;

}

/*
 * Releases memory and closes the stream associated with the parser.  
 */
void 
parser_destroy(void)
{

    if (errno != 0) {
        perror("Aborted translation");
        fprintf(stderr, "Parsing line %d.\n", line_num - 1); 
    }

    free(buffer);
    if (fclose(fp) == EOF) {
        perror("parser_destroy");
    }

}


/**************************************************** Private implementations */

/*
 * Checks whether `filename` has '.asm' extension.
 */
static inline bool 
is_extension_asm(const char *filename)
{
    const char *p;
    
    p = strrchr(filename, '.');

    if (p == NULL) {
        return false;
    }
    return strncmp(p, ".asm", 5) == 0;
}

/*
 * The `parser_has_more_commands` implementation strictly requires this three
 * functions to be called in the order in which they are declared.  The names 
 * are self-explanatory.
 */
static inline void 
discard_leading_withe_spaces(void)
{
    while (token && isspace(*(token))) {
        token++;
    }
}

static inline bool 
is_blank_line(void)
{
    return strncmp(token, "", 1) == 0;
}

static inline bool 
is_comment_line(void)
{
    assert(line_len >= 2);
    return (*token) == '/' && (*(token + 1)) == '/';
}
