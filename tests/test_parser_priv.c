#define _POSIX_C_SOURCE 200809L
#include "minunit.h"
#include "../src/parser.c"

static char* bad_file = "./tests/resources/parser-tester.file";
static char* asm_file = "./tests/resources/parser-tester.asm";

/* Called before each test case is executed. */
void test_setup(void) { }

/* Called after each test case is executed. */
void test_teardown(void) { }

MU_TEST(test_is_extension_asm)
{
    mu_check(is_extension_asm(asm_file) == true);
    mu_check(is_extension_asm("Test") == false);
    mu_check(is_extension_asm(bad_file) == false);
    mu_check(is_extension_asm("Test.") == false);
}

MU_TEST(test_discard_leading_withe_spaces)
{
    char str0[] = "\n";
    char str1[] = "  \n";
    char str2[] = "\t  Not blank\n";

    token = str0;
    discard_leading_withe_spaces();
    mu_assert_string_eq("", token);

    token = str1;
    discard_leading_withe_spaces();
    mu_assert_string_eq("", token);

    token = str2;
    discard_leading_withe_spaces();
    mu_assert_string_eq("Not blank\n", token);
}

MU_TEST(test_is_blank_line)
{
    char str0[] = "";
    char str1[] = "\n";
    char str2[] = "  Not blank";

    token = str0;
    mu_check(is_blank_line() == true);

    token = str1;
    mu_check(is_blank_line() == false);

    token = str2;
    mu_check(is_blank_line() == false);
}

MU_TEST(test_is_comment_line)
{
    char str0[] = "// Yes";
    char str1[] = "No\n";
    char str2[] = "@2\n";

    line_len = 6;
    token = str0;
    mu_check(is_comment_line() == true);

    line_len = 3;
    token = str1;
    mu_check(is_comment_line() == false);

    line_len = 3;
    token = str2;
    mu_check(is_comment_line() == false);
}

MU_TEST(test_parser_init)
{
    mu_check(parser_init(NULL) == NULL); 
    mu_check(parser_init("") == NULL); 

    mu_check(parser_init(bad_file) == NULL); 

    mu_check(parser_init(asm_file) != NULL); 
    mu_check(buffer == NULL);
    mu_check(token == NULL);
    mu_check(line_len == 0);
    mu_check((int)command == -1);
}

MU_TEST(test_parser_advance)
{
    /* Test reading a line */
    parser_init(asm_file);
    parser_advance();
    mu_assert_string_eq("// This file is part of www.nand2tetris.org\n",
                        buffer);
    mu_assert_string_eq("// This file is part of www.nand2tetris.org\n",
                        token);
    mu_assert_int_eq(strlen("// This file is part of www.nand2tetris.org\n"),
                        (int)line_len);
    mu_assert_int_eq(1, (int)line_num);

    parser_destroy();

    /* Test reaching EOF condition.  Test file has 13 lines */
    parser_init(asm_file);
    for (int i = 0; i < 14; i++) {
        parser_advance();
        mu_assert_int_eq(i + 1, (int)line_num);
    }
    mu_check(line_len == -1);
    mu_check(feof(fp));

    parser_destroy();
}

MU_TEST(test_parser_has_more_commands)
{
    parser_init(asm_file);

    parser_advance();
    mu_check(parser_has_more_commands() == true);
    /* The parser should have buffered line 8 */
    mu_assert_string_eq("@2\n", token);
    mu_assert_int_eq(strlen("    @2\n"), (int)line_len);
    mu_assert_int_eq(8, (int)line_num);

    parser_advance();
    mu_check(parser_has_more_commands() == true);
    /* Has buffered line 9? */
    mu_assert_string_eq("D=A\n", token);
    mu_assert_int_eq(strlen("    D=A\n"), (int)line_len);
    mu_assert_int_eq(9, (int)line_num);

    for (int i = 0; i < 5; i++) {
        parser_advance();
        mu_assert_int_eq(i + 10, (int)line_num);
    }
    mu_check(parser_has_more_commands() == false);

    parser_destroy();
}

MU_TEST(test_parser_get_command_type)
{
    char str0[] = "@2\n";
    char str1[] = "(LABEL)\n";
    char str2[] = "D=A\n";
    char str3[] = "Nonsense\n";
    char str4[] = "44\n";

    token = str0;
    mu_assert_int_eq(A_COMMAND, parser_get_command_type());
    mu_assert_int_eq(A_COMMAND, command);
    token = str1;
    mu_assert_int_eq(L_COMMAND, parser_get_command_type());
    mu_assert_int_eq(L_COMMAND, command);
    token = str2;
    mu_assert_int_eq(C_COMMAND, parser_get_command_type());
    mu_assert_int_eq(C_COMMAND, command);
    token = str3;
    mu_assert_int_eq(C_COMMAND, parser_get_command_type());
    mu_assert_int_eq(C_COMMAND, command);
    token = str4;
    mu_assert_int_eq(C_COMMAND, parser_get_command_type());
    mu_assert_int_eq(C_COMMAND, command);
}

MU_TEST(test_parser_symbol)
{
    char str0[] = "@2";
    char str1[] = "@19  \n";
    char str2[] = "@R0\n";
    char str3[] = "@R2\t// Comment";
    char str4[] = "@CUSTOM_LABEL";
    char str5[] = "(LABEL)";
    char str6[] = "(LABEL_)\n";
    char str7[] = "(__LABEL)\t\t // Comment";
    char str8[] = "88";
    char str9[] = "Nonsense";

    command = A_COMMAND;

    token = str0;
    mu_assert_string_eq("2", parser_symbol());
    token = str1;
    mu_assert_string_eq("19", parser_symbol());
    token = str2;
    mu_assert_string_eq("R0", parser_symbol());
    token = str3;
    mu_assert_string_eq("R2", parser_symbol());
    token = str4;
    mu_assert_string_eq("CUSTOM_LABEL", parser_symbol());

    command = L_COMMAND;

    token = str5;
    mu_assert_string_eq("LABEL", parser_symbol());
    token = str6;
    mu_assert_string_eq("LABEL_", parser_symbol());
    token = str7;
    mu_assert_string_eq("__LABEL", parser_symbol());
    token = str8;
    mu_assert_string_eq("88", parser_symbol());
    token = str9;
    mu_assert_string_eq("Nonsense", parser_symbol());
}

MU_TEST(test_parser_dest)
{
    char str0[] = "M=D\n";
    char str1[] = "0;JMP  \n";
    char str2[] = "D=D-M\n";
    char str3[] = "0;JMPR2\t// Comment";

    command = C_COMMAND;

    token = str0;
    mu_assert_string_eq("M", parser_dest());

    token = str1;
    mu_assert_string_eq("", parser_dest());

    token = str2;
    mu_assert_string_eq("D", parser_dest());

    token = str3;
    mu_assert_string_eq("", parser_dest());
}

MU_TEST(test_parser_comp)
{
    char str0[] = "D+1\n";
    char str1[] = "0;JMP  \n";
    char str2[] = "D-M\n";
    char str3[] = "0;JMP\t// Comment";
    char str4[] = "Bad Input";

    command = C_COMMAND;

    token = str0;
    mu_assert_string_eq("D+1", parser_comp());
    
    token = str1;
    mu_assert_string_eq("0", parser_comp());
    
    token = str2;
    mu_assert_string_eq("D-M", parser_comp());
    
    token = str3;
    mu_assert_string_eq("0", parser_comp());
    
    token = str4;
    mu_assert_string_eq("Bad", parser_comp());
}

MU_TEST(test_parser_jump)
{
    char str1[] = "JMP\n";
    char str2[] = "\n";
    char str3[] = "JGT\t// Comment";
    char str4[] = "\t// Comment";
    char str5[] = "";

    command = C_COMMAND;

    token = str1;
    mu_assert_string_eq("JMP", parser_jump());

    token = str2;
    mu_assert_string_eq("", parser_jump());

    token = str3;
    mu_assert_string_eq("JGT", parser_jump());

    token = str4;
    mu_assert_string_eq("", parser_jump());

    token = str5;
    mu_assert_string_eq("", parser_jump());
}

MU_TEST_SUITE(test_suite) 
{
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    MU_RUN_TEST(test_is_extension_asm);
    MU_RUN_TEST(test_discard_leading_withe_spaces);
    MU_RUN_TEST(test_is_blank_line);
    MU_RUN_TEST(test_is_comment_line);
    MU_RUN_TEST(test_parser_init);
    MU_RUN_TEST(test_parser_advance);
    MU_RUN_TEST(test_parser_has_more_commands);
    MU_RUN_TEST(test_parser_get_command_type);
    MU_RUN_TEST(test_parser_symbol);
    MU_RUN_TEST(test_parser_dest);
    MU_RUN_TEST(test_parser_comp);
    MU_RUN_TEST(test_parser_jump);
}

int main(int argc, char *argv[]) 
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();

	return MU_EXIT_CODE;
}
