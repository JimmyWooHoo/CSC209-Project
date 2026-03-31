# Parallel Word Counter

`pwc` is a multi-process word-frequency counter written in C.
It reads a list of input text files, spawns one worker process per file, and uses
pipes to return each worker's local word-count result to the parent. The parent
merges the results, sorts them, applies optional filters, and prints the final output.

## Overview

This project focuses on process creation, pipe-based communication, concurrent
worker execution, and resource cleanup. The application logic is intentionally
simple so the systems-programming design is the main focus.


## Build

```sh
make
```

To rebuild from scratch:

```sh
make clean
make
```

## Run

```sh
./pwc [-f | -rf | -a | -ra] [-i] [-m N] [-k K] <filelist>
```

Sorting options:

- `-f`: sort by frequency descending, then alphabetically
- `-rf`: sort by frequency ascending, then alphabetically
- `-a`: sort alphabetically ascending
- `-ra`: sort alphabetically descending
- `-i`: treat words case-insensitively
- `-m N`: only print words with count at least `N`
- `-k K`: only print the first `K` results after sorting

The `<filelist>` argument must be a text file containing one input filename per
line.

### Input Requirements

- The file list must contain at least 3 input filenames.
- Each listed file is processed by its own worker process.
- Words are split using whitespace.
- In case-insensitive mode, words are converted to lowercase before counting.

If the file list contains fewer than 3 filenames, the program exits with:

```text
Error: file list must contain at least 3 input files.
```

## Example

```sh
./pwc inputs.txt
./pwc -f -i -m 2 -k 5 inputs.txt
```

## Sample Files

The repository includes a small sample data set:

- `file1.txt`
- `file2.txt`
- `file3.txt`
- `file4.txt`
- `inputs.txt`

`inputs.txt` should contain one filename per line and can be used for quick
demo runs.

## Testing

Run the following commands sequentially.

### 1. Build Check

```sh
make clean
make
```

Expected result:

- The program builds successfully with no warnings.

### 2. Sorting Checks

```sh
./pwc inputs.txt
./pwc -a inputs.txt
./pwc -ra inputs.txt
./pwc -rf inputs.txt
```

These commands verify:

- default frequency-descending order
- alphabetical order
- reverse alphabetical order
- frequency-ascending order

### 3. Feature Checks

```sh
./pwc -i inputs.txt
./pwc -m 3 inputs.txt
./pwc -k 3 inputs.txt
./pwc -f -i -m 2 -k 4 inputs.txt
```

These commands verify:

- case-insensitive mode
- minimum-count filtering
- top-k limiting
- combining multiple options in one run

### 4. Case-Insensitive Test

Create three files so the program still satisfies the 3-worker minimum:

```sh
printf 'Apple apple APPLE\n' > case1.txt
printf 'Banana banana\n' > case2.txt
printf 'APPLE Banana\n' > case3.txt
printf 'case1.txt\ncase2.txt\ncase3.txt\n' > case_inputs.txt

./pwc case_inputs.txt
./pwc -i case_inputs.txt
```

Expected behavior:

- without `-i`, capitalized words are counted separately
- with `-i`, they are merged into lowercase counts

### 5. Error-Handling Checks

```sh
./pwc
./pwc -m
./pwc -k nope inputs.txt
./pwc missing_list.txt

printf 'does_not_exist.txt\nfile1.txt\nfile2.txt\n' > bad_inputs.txt
./pwc bad_inputs.txt

printf 'file1.txt\n' > one_input.txt
./pwc one_input.txt
```

These commands verify:

- missing required arguments
- missing numeric values
- invalid numeric values
- missing file-list file
- missing worker input file
- enforcement of the minimum 3-file requirement

### 6. Empty-File Check

```sh
touch empty1.txt empty2.txt empty3.txt
printf 'empty1.txt\nempty2.txt\nempty3.txt\n' > empty_inputs.txt
./pwc empty_inputs.txt
```

Expected behavior:

- the program should not crash
- it should simply print no output

### 7. Cleanup Temporary Test Files

```sh
rm -f case1.txt case2.txt case3.txt case_inputs.txt
rm -f bad_inputs.txt one_input.txt
rm -f empty1.txt empty2.txt empty3.txt empty_inputs.txt
```

## Notes

- Run commands one at a time. Do not run test commands in parallel with
  `make clean`, because the binary may be deleted during the test.
- The program is intended for plain text input files and whitespace-delimited
  words.
