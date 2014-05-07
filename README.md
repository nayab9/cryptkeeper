##Cryptkeeper

This is an experimental utility to apply an XOR encryption scheme to data.
Given any input data and thread numbers to use via stdin, the utility will encrypt and output the data to stdout.
A keyfile and number of threads must be specified upon running the application.

###Usage:

> cryptkeeper [-n threads] [-k key]

Where **threads** is the number of threads and **key** is the key used for encryption.

Example usage:

> cat data | ./cryptkeeper -n 2 -k key > encrypted


###Compile Instructions:

> gcc cryptkeeper.c -o cryptkeeper -lpthread

###Debug Instructions:

> gcc -Wall -g -O0 cryptkeeper.c -o cryptkeeper -lpthread
> valgrind --tool=memcheck --track-origins=yes --leak-check=full ./cryptkeeper -n 2 -k key > encrypted

> scan-build gcc cryptkeeper.c -o cryptkeeper -lpthread

###Screenshot Example
![Loading Screenshot](../screenshots/screenshots/screenshot.png?raw=true)

###Detailed process:

* The blocksize used is the size of the key provided.
* The block is created by reading bytes, to avoid issues with character representations (i.e. ASCII vs UTF)
* Given input [1...N] and threads T, the utility will read the input data and XOR them with the key file provided.
* The key will repeat itself after every block is complete.
* The data can be very large, as such can never be read into memory larger than blocksize

Simple readable working example:

> hexdump -C key

01 02 03 04

> hexdump -C data

05 06 07 08 09 0a 0b 0c

> cat data | ./cryptkeeper -n 2 -k key > encrypted

> hexdump -C encryped

04 04 04 04 08 08 08 08

Since 2 threads were requested, each block of 4 must be processed twice using two threads, each encrypting and returning a value.
Once the threads are complete, we move to the next block.

Block Start
* Thread1 = [ 01 xor 05 ]
* Thread2 = [ 02 xor 06 ]
* Join Thread 1 and Thread 2
* Thread1 = [ 03 xor 07 ]
* Thread2 = [ 04 xor 08 ]
* Join Thread 1 and Thread 2  

Block Complete

Continue until there are no more blocks to read in.

Reversing the encrypted data back into the utility with the key will result in the original data.
