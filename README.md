# CSC209 Project

`pwc` is a multi-process word-count indexer written in C for CSC209 Category 1.
The parent process reads a file containing input filenames, forks one worker per
input file, receives each worker's local word-frequency table over a pipe, and
then merges the results into one final report.

## Build

```sh
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

## Example

```sh
./pwc inputs.txt
./pwc -f -i -m 2 -k 5 inputs.txt
```
