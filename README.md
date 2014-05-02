This is an experimental utility to apply simple encryption schemes to data.

Given any size input data via stdin, the utility will encrypt and output the data to stdout.

A keyfile and number of threads must be specified upon running the application.

Usage:

>> cryptkeeper [-n #] [-k key]

Where -n is the number of threads and -k is the keyfile.

Example usage:

cat data.txt | cryptkeeper -n 2 -k keyfile.txt > cryptofied


Compile Instructions:

>> gcc cryptkeeper.c -o cryptkeeper