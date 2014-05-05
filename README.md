##Cryptkeeper

This is an experimental utility to apply an XOR encryption scheme to data.
Given any size input data and threads to use via stdin, the utility will encrypt and output the data to stdout.
A keyfile and number of threads must be specified upon running the application.

###Usage:

> cryptkeeper [-n #] [-k key]

Where -n is the number of threads and -k is the keyfile.

Example usage:

> cat data | cryptkeeper -n 2 -k keyfile > cryptofied


###Compile Instructions:

> gcc cryptkeeper.c -o cryptkeeper -lpthread


###Detailed process:

* The blocksize used is the size of the keyfile provided.
* Assume blocksize is 4 bytes of data [k1, k2, k3, k4] represent the hexvalues of the key file.
* Given input [1...N] and threads T, the utility will read the input data and XOR them with the key file provided.
* The key will repeat itself after every block is complete.

Working example:

> hexdump -C keyfile

01 02 03 04

> hexdump -C datafile

05 06 07 08 09 0a 0b 0c

> cat datafile | ./cryptkeeper -n 2 -k keyfile > encrypto_patronus

> hexdump -C encrypto_patronus

04 04 04 04 08 08 08 08

Since 2 threads were requested, each block of 4 must be processed twice using two threads, each encrypting and returning a value.
Once the threads are complete, we move to the next block.

Block Start
* Thread1 = [ 01 xor 04 ]
* Thread2 = [ 02 xor 06 ]
* Join Thread 1 and Thread 2
* Thread1 = [ 03 xor 07 ]
* Thread2 = [ 04 xor 08 ]
* Join Thread 1 and Thread 2  

Block Complete

continue..

Note that since the data provided can be very large, the utility will read by blocksize, process and continue to the next block infinitely.
