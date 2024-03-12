CC := gcc
CFLAGS := -std=c99 -Og -Wall -Wextra -Wpedantic

TESTFLAGS := \
		-ggdb3 -Wconversion -Wshadow \
		-Wno-unused-function -Wno-unused-parameter -Wno-unused-variable \
		-fsanitize=address,undefined,leak
TESTLDFLAGS := -lasan -lm -lrt

TARGET_EXEC := hackassembler

BUILD_DIR := ./bin
INC_DIR := ./include
OBJ_DIR := ./obj
INSTALL_DIR := $(HOME)/.local/bin
SRC_DIR := ./src
TEST_DIR := ./tests
TEST_BIN := ./tests/bin

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/test_*.c)
# Excludes $(TARGET_EXEC).o, all the test binaries define a `main()` function.
TEST_OBJS := $(filter-out $(TEST_BIN)/$(notdir $(TARGET_EXEC)).o, $(patsubst $(SRC_DIR)/%.c, $(TEST_BIN)/%.o, $(SRCS)))
TEST_EXEC := $(patsubst $(TEST_DIR)/%.c, $(TEST_BIN)/%, $(TEST_SRCS))

ifndef VERBOSE
MAKEFLAGS += --quiet --no-print-directory
endif

all: $(BUILD_DIR)/$(TARGET_EXEC)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/$(TARGET_EXEC).o: $(SRC_DIR)/$(TARGET_EXEC).c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

install: $(BUILD_DIR)/$(TARGET_EXEC)
	install -D $(BUILD_DIR)/$(TARGET_EXEC) $(INSTALL_DIR)/$(TARGET_EXEC)
	make clean

uninstall:
	rm $(INSTALL_DIR)/$(TARGET_EXEC)

# The recipe filters out the object file corresponding to the current target,
# otherwise we will attempt at linking it it twice.
$(TEST_BIN)/test_%_priv: $(TEST_DIR)/test_%_priv.c $(SRC_DIR)/%.c $(TEST_OBJS) | $(TEST_BIN)
	$(CC) $(CFLAGS) -I$(INC_DIR) $< $(filter-out $(TEST_BIN)/$*.o, $(TEST_OBJS)) -o $@ 

$(TEST_BIN)/test_%_publ: $(TEST_DIR)/test_%_publ.c $(SRC_DIR)/%.c $(INC_DIR)/%.h $(TEST_OBJS) | $(TEST_BIN)
	$(CC) $(CFLAGS) -I$(INC_DIR) $< $(TEST_OBJS) -o $@ 

$(TEST_BIN)/%.o: $(SRC_DIR)/%.c | $(TEST_BIN)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR) $(OBJ_DIR) $(TEST_BIN):
	mkdir -p $@

.PHONY: all clean compare install uninstall tests

.PRECIOUS: $(TEST_OBJS)

# Tests all units, output shown only in case of failure. (No news is good news.)
tests: CFLAGS += $(TESTLDFLAGS)
tests: CFLAGS += $(TESTFLAGS)
tests: $(TEST_EXEC)
	for test in $^; do \
		./$$test > /dev/null 2>&1; \
		if [ ! $$? -eq 0 ]; then \
			echo "\nFail: $$test\nExit code: $$exit_code\n"; \
			exit 1; \
		fi; \
	done;

# Test a specific module or interface showing detailed output.
test_%: CFLAGS += $(TESTLDFLAGS) 
test_%: CFLAGS += $(TESTFLAGS) 
test_%: $(TEST_BIN)/test_%
	./$(TEST_BIN)/$@ 

# Run assembler output comparison script.
compare: $(BUILD_DIR)/$(TARGET_EXEC)
	./tests/compare.sh

clean:
	rm -rf $(BUILD_DIR) $(OBJ_DIR) $(TEST_BIN)
