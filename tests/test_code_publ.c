#include "minunit.h"
#include "../include/code.h"

void test_setup(void) { }
void test_teardown(void) { }

MU_TEST(test_code_dest)
{
    mu_assert_int_eq(0x00, code_dest(""));
    mu_assert_int_eq(0x08, code_dest("M"));
    mu_assert_int_eq(0x10, code_dest("D"));
    mu_assert_int_eq(0x18, code_dest("MD"));
    mu_assert_int_eq(0x20, code_dest("A"));
    mu_assert_int_eq(0x28, code_dest("AM"));
    mu_assert_int_eq(0x30, code_dest("AD"));
    mu_assert_int_eq(0x38, code_dest("AMD"));

    mu_assert_int_eq(ERROR, code_comp("Some nonsense"));
}

MU_TEST(test_code_comp)
{
    mu_assert_int_eq(0x0A80, code_comp("0"));
    mu_assert_int_eq(0x0FC0, code_comp("1"));
    mu_assert_int_eq(0x0E80, code_comp("-1"));
    mu_assert_int_eq(0x0300, code_comp("D"));
    mu_assert_int_eq(0x0C00, code_comp("A"));
    mu_assert_int_eq(0x0C00, code_comp("A"));
    mu_assert_int_eq(0x1C00, code_comp("M"));
    mu_assert_int_eq(0x0340, code_comp("!D"));
    mu_assert_int_eq(0x0C40, code_comp("!A"));
    mu_assert_int_eq(0x1C40, code_comp("!M"));
    mu_assert_int_eq(0x0CC0, code_comp("-A"));
    mu_assert_int_eq(0x1CC0, code_comp("-M"));
    mu_assert_int_eq(0x07C0, code_comp("D+1"));
    mu_assert_int_eq(0x0DC0, code_comp("A+1"));
    mu_assert_int_eq(0x1DC0, code_comp("M+1"));
    mu_assert_int_eq(0x0380, code_comp("D-1"));
    mu_assert_int_eq(0x0C80, code_comp("A-1"));
    mu_assert_int_eq(0x1C80, code_comp("M-1"));
    mu_assert_int_eq(0x0080, code_comp("D+A"));
    mu_assert_int_eq(0x1080, code_comp("D+M"));
    mu_assert_int_eq(0x04C0, code_comp("D-A"));
    mu_assert_int_eq(0x14C0, code_comp("D-M"));
    mu_assert_int_eq(0x01C0, code_comp("A-D"));
    mu_assert_int_eq(0x11C0, code_comp("M-D"));
    mu_assert_int_eq(0x0000, code_comp("D&A"));
    mu_assert_int_eq(0x1000, code_comp("D&M"));
    mu_assert_int_eq(0x0540, code_comp("D|A"));
    mu_assert_int_eq(0x1540, code_comp("D|M"));

    mu_assert_int_eq(ERROR, code_comp("Some nonsense"));
}

MU_TEST(test_code_jump)
{
    mu_assert_int_eq(0x0, code_jump(""));
    mu_assert_int_eq(0x1, code_jump("JGT"));
    mu_assert_int_eq(0x2, code_jump("JEQ"));
    mu_assert_int_eq(0x3, code_jump("JGE"));
    mu_assert_int_eq(0x4, code_jump("JLT"));
    mu_assert_int_eq(0x5, code_jump("JNE"));
    mu_assert_int_eq(0x6, code_jump("JLE"));
    mu_assert_int_eq(0x7, code_jump("JMP"));

    mu_assert_int_eq(ERROR, code_comp("Some nonsense"));
}

MU_TEST_SUITE(test_suite) 
{
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
	MU_RUN_TEST(test_code_dest);
	MU_RUN_TEST(test_code_comp);
	MU_RUN_TEST(test_code_jump);
}

int main(int argc, char *argv[]) 
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();

	return MU_EXIT_CODE;
}
