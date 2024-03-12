#include "minunit.h"
#include "../src/hackassembler.c"

void test_setup(void)
{
    OutputStream = tmpfile();
    return;
}

void test_teardown(void)
{
    fclose(OutputStream);
    return;
}

MU_TEST(test_num_to_address)
{
    mu_assert_int_eq(0x0000, num_to_address("0"));
    mu_assert_int_eq(0x0AAA, num_to_address("2730"));
    mu_assert_int_eq(0, num_to_address("0"));
    mu_assert_int_eq(ERROR, num_to_address("-10"));
    mu_assert_int_eq(ERROR, num_to_address("65535"));
    mu_assert_int_eq(ERROR, num_to_address("100000"));
}

MU_TEST(test_write_to_binary_stream)
{
    size_t i; 
    int c;
    char *str = "0000000000000000\n" 
                "1110000000000000\n" 
                "1011111011101111\n"
                "0000000000000001\n";

    Instruction = 0x0000;
    write_to_binary_stream();
    Instruction = 0xE000;
    write_to_binary_stream();
    Instruction = 0xBEEF;
    write_to_binary_stream();
    Instruction = 0x0001;
    write_to_binary_stream();

    rewind(OutputStream);

    for (i = 0; ((c = fgetc(OutputStream)) != EOF); i++) {
        mu_assert_int_eq(*(str + i), c);
    }
}

MU_TEST_SUITE(test_suite) 
{
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
	MU_RUN_TEST(test_num_to_address);
	MU_RUN_TEST(test_write_to_binary_stream);
}

int main(int argc, char *argv[]) 
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();

	return MU_EXIT_CODE;
}
