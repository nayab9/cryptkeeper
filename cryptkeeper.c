#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    
    //read from the key file specified
 
    char *source = NULL;
    FILE *fp = fopen(argv[2], "r");
    if (fp != NULL) {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            long bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }
    
            /* Allocate our buffer to that size. */
            source = (char *)malloc(sizeof(char) * (bufsize + 1));
    
            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }
    
            /* Read the entire file into memory. */
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if (newLen == 0) {
                fputs("Error reading file", stderr);
            } else {
                source[++newLen] = '\0'; /* Just to be safe. */
            }
        }
        fclose(fp);
    }

    char *buff = source;


    //printf("You have chosen to utilize %s number of threads for this process.\n", argv[1]);
    //printf("The key file chosen was %s.\n", buff);
    //printf("");

    int blocksize = strlen(buff);
        //printf("blocksize is: %d\n", blocksize);
    char text[blocksize];
    
    char* plaintext = text;
    char *buff2 = plaintext;
    int i;
    
    //read bytes, since we can't rely on characters themselves (i.e Unicode vs ASCII)
    int result = fread(text, sizeof(char), blocksize, stdin);
    
    ///printf("The initial read was: %d\n", result);
    
    //so long as enough blocksize bytes are available, continue looping
    while ( result == blocksize )
    {
        //printf("The plaintext read in is: %s\n", plaintext);
        //printf("the result of the read was: %d\n", result);

            //XOR the block read in with our keyfile provided, print to stdout
            //spawn thread for this work
            for(i=0; i < blocksize; i++)
            {
                text[i] ^= buff[i];
                //printf("The XOR result of block %d is %x\n", i, text[i]);
                //printf("%c\n", text[i]);
            }
            
    printf ("%s", text);
    
        result = fread(text, sizeof(char), blocksize, stdin);
        //printf("The next readin is %d\n", result);
    }

    
    //return and put values somewhere


    free(source); /* Don't forget to call free() later! */

}
