#!/bin/env sh

# Runs the assembler on each .asm file supplied for testing.  
# Compares the output against correctly compiled files. Aborts and exits on 
# failure, removes the produced output on success.

test_files_folder="tests/resources/asm-files"
comparison_folder="tests/resources/expected-output"

for asm_file in "$test_files_folder"/*.asm; do

  file=$(basename "$asm_file")
  file_no_ext="${file%.asm}"

  ./bin/hackassembler "$asm_file" 

  diff "$test_files_folder/$file_no_ext.hack" "$comparison_folder/$file_no_ext.hack" > /dev/null 2>&1
  if [ ! $? -eq 0 ]; then
    echo "Failed comparison: $test_files_folder/$file_no_ext.hack $comparison_folder/$file_no_ext.hack"
    exit 1
  else
    rm "$test_files_folder/$file_no_ext.hack"
  fi
done
