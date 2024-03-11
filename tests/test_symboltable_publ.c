#include "minunit.h"
#include <errno.h>
#include "../include/symboltable.h"
#include "../include/common/shared_defs.h"

void test_setup(void)
{
    symbol_table_init();
}

void test_teardown(void)
{
    symbol_table_destroy();
}

MU_TEST(test_symbol_table_init)
{
    mu_check(symbol_table_contains("SP") == true);
    mu_check(symbol_table_contains("LCL") == true);
    mu_check(symbol_table_contains("THAT") == true);
    mu_check(symbol_table_contains("SCREEN") == true);
    mu_check(symbol_table_contains("LOL") == false);
    mu_check(symbol_table_contains("wut?") == false);

    mu_assert_int_eq(0x0001, symbol_table_get_addr("R1"));
    mu_assert_int_eq(0x000F, symbol_table_get_addr("R15"));
    mu_assert_int_eq(ERROR, symbol_table_get_addr("ACK"));
    mu_assert_int_eq(0x0000, symbol_table_get_addr("SP"));
    mu_assert_int_eq(0x0002, symbol_table_get_addr("R2"));
    mu_assert_int_eq(0x0002, symbol_table_get_addr("ARG"));
}

MU_TEST(test_symbol_table_program_symbols)
{
    symbol_table_add_entry("LOOP", 0x0A8);
    mu_check(symbol_table_contains("LOOP") == true);
    mu_check(symbol_table_get_addr("LOOP") == 0x00A8);

    symbol_table_add_entry("LABEL", 0x002B);
    mu_check(symbol_table_contains("LABEL") == true);
    mu_check(symbol_table_get_addr("LABEL") == 0x002B);

    symbol_table_add_entry("LABEL", 0x002B);
    mu_check(errno != 0);
    errno = 0;

    symbol_table_add_entry("MAIN", 0xBEEF);
    mu_check(symbol_table_contains("MAIN") == true);
    mu_check(symbol_table_get_addr("MAIN") == 0xBEEF);

    mu_check(symbol_table_contains("nope") == false);
}

MU_TEST_SUITE(test_suite) 
{
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
	MU_RUN_TEST(test_symbol_table_init);
	MU_RUN_TEST(test_symbol_table_program_symbols);
}

int main(int argc, char *argv[]) 
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();

	return MU_EXIT_CODE;
}
