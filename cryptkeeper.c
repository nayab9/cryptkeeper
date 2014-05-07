/*****************************************************************
 * 
 * The Cryptkeeper
 * 
 * This utility is a simple demonstration of XOR encryption.
 * Given a specified input of threads and a key file, the utility
 * will proceed to XOR encrypt stdin data.
 * 
 * More information and detailed process can be found on github:
 * https://github.com/nayab9/cryptkeeper
 * 
 * "Encrypto Patronum!" - A once famous Wizard
 * 
 * ****************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/**************************************************
 * 
 * This struct stores thread information
 * as the data is processed.
 * Extra information is stored here for learning and
 * debugging purposes.
 * 
 * ***************************************************/
typedef struct block_info
{
    int thread_id;
    int data_value;
    int key_value;
    int encrypted_value;
}block;

/**************************************************
 * 
 * Thread function *encrypt():
 * 
 * Given a struct block, this function does the 
 * thread processing.
 * 
 * Each block contains the data value and encryption
 * value, it will XOR them together and return the block
 * 
 * EXCEPTION CASE:
 * In the case where the encryption and data value are
 * identical, we hit a gotcha - they cancel eachother
 * out and equal 0. Thus I leave this value unecrypted
 * for the purpose of this demonstration.
 * 
 * ***************************************************/
void *encrypt(void *thread_ptr)
{
    block *data = (block *) thread_ptr;

    if ( data->key_value != data->data_value )
    {
        data->encrypted_value = data->key_value ^ data->data_value;
    }
    else
    {
        data->encrypted_value = data->data_value;
    }

    return (void * ) data;
}

/**************************************************
 * 
 * main():
 * 
 * The main processing is done as follows:
 * 1. Read command line inputs -> assign threads and key
 * 2. Read the key file (static file, does not change)
 *   - key file indicates blocksize
 * 3. Read data input from stdin in bytes (blocksize)
 *   - spawn threads as required
 *   - join threads as required
 * 4. Output data to stdout
 * 
 * Detailed explanations can be found here:
 * https://github.com/nayab9/cryptkeeper
 * 
 * ***************************************************/
int main(int argc, char *argv[])
{
    char *keyfile = NULL;
    char *source;
    char *buff = "";
    int blocksize = 1;
    int num_threads = 1;
    int length = argc - 1;
    int loop, i, j;
    long bufsize = -1;
    size_t newLen = 0;
        
    //valid input parameters
    char *key_switch = "-k";
    char *thread_switch = "-n";

    //find input parameters and capture them   
    for (loop = 0; loop < length; loop++)
    {
        if (strcmp (argv[loop], key_switch) == 0)
        {
            keyfile = argv[loop+1];
        }
        
        if (strcmp (argv[loop], thread_switch) == 0)
        {
            sscanf (argv[loop+1], "%d", &num_threads);
        }
        
        /*
        if (strcmp (argv[loop], thread_switch) == 0)
        {
            //add more switch options here...
        }
        */
    }
    
    //read from the key file specified
    FILE *fp = fopen(keyfile, "r");
    if (fp != NULL) 
    {
        if (fseek(fp, 0L, SEEK_END) == 0) 
        {
            bufsize = ftell(fp);
            if (bufsize == -1) 
            {
                fprintf(stderr, "Error reading keyfile, exiting.. \n");
                return 2;                    
            }
            else
            {
                //allocate the memory to store our key
                source = (char *)malloc(sizeof(char) * (bufsize + 1));
            }
    
            if (fseek(fp, 0L, SEEK_SET) != 0) 
            {
                fprintf(stderr, "Error reading keyfile, exiting.. \n");
                return 2;  
            }
            else
            {
                //read the actual key into memory
                newLen = fread(source, sizeof(char), bufsize+1, fp);
                
                //assign a pointer to the keyfile
                //this will be used from here on out for the encrypting
                //simple obfuscation
                buff = source;
                
                //retrive the blocksize based on the keyfile bytes
                blocksize = strlen(buff)+1;
            }
            
            if (newLen == 0) 
            {
                fprintf(stderr, "Error opening keyfile, exiting.. \n");
                return 2;    
            } 
            else 
            {
                source[newLen] = '\0';
            }
    
            fclose(fp);
    
        }
    }
    else
    {
        fprintf(stderr, "Error opening keyfile, exiting.. \n");
        return 2;       
    }

    //if threads specified is more than the blocksize
    //force threads to blocksize, since thats the max
    //number of threads we can utilize
    if (num_threads > blocksize)
    {
        num_threads = blocksize;
    }
    
    //this is where we store the XOR'd values to printout
    char text[blocksize];

    /* Enable for valgrind debug purposes, no stdin data
    FILE *pfile = fopen ("data", "rb");
    if (pfile == NULL)
    {
        fputs("File error", stderr);
        return 1;
    }
    */

    //read by bytes, since we can't rely on characters themselves (i.e Unicode vs ASCII)
    int result = fread(text, sizeof(char), blocksize, stdin);

    //initialize the struct blocks and threads needed for processing
    block encrypt_go[blocksize];
    pthread_t threads[num_threads];
    
    //so long as enough blocksize bytes are available, continue looping
	//no threading will be used if they don't want it (i.e. threads > 2)
    while ( num_threads >= 2 && result == blocksize )
    {
        //loop through the block (of bytes) that was read in
        for(i=0; i <= blocksize; i++)
        {
            //index is the modulus of i, this allows us to keep track of
            //which thread we are processing when the blocksize is greater than
            //the number of threads that are requsted
            //example: 2 threads requsted, but our blocksize is 4, thus
            //we are limited to using 2 threads, twice (thread 1 and 2, twice)
            int index = i % num_threads;
            
            //local scope reference to i, to be multi-thread safe
            int temp_i = i;
            
            //safety check, only fill data the size of the blocks
    		if (temp_i < result)
    		{
            	encrypt_go[temp_i].thread_id = temp_i;
            	encrypt_go[temp_i].data_value = text[temp_i];
            	encrypt_go[temp_i].key_value = buff[temp_i];
    		}

            //if we have less threads than the blocksize, we wrap up the thread
            //processing before processing the rest of the block
            if ( (temp_i >= num_threads && index == 0) ||
                 (blocksize == num_threads && num_threads == temp_i) )
            {
                //for each thread spawned, we will attempt to join them together
                for (j = 0; j < num_threads; j++)
                {
                    void *returnValue;
                    if (pthread_join( threads[j], &returnValue ))
                    {
                        fprintf(stderr, "Error joining thread, exiting.. \n");
                        return 2;
                    }
                    else
                    {
                        block *return_block = (block *) returnValue;
                        int encrypted_value = (int) return_block->encrypted_value;
                        int returned_thread = (int) return_block->thread_id;

                        text[returned_thread] = encrypted_value;
                    }
                }
                //if our block size is larger than number of threads
                //we must now processing the rest of the block
            	if (temp_i < result)
            	{ 
                       if (pthread_create( &threads[index], NULL, encrypt, (void *) &encrypt_go[temp_i] ))
                        {
                            fprintf(stderr, "Error creating thread, exiting... \n");
                            return 1;
                        }
            	}                                   
                
            }
            //we still have threads which can process some of our blockdata, continue.
            else
            {
                if (pthread_create( &threads[index], NULL, encrypt, (void *) &encrypt_go[temp_i] ))
                {
                    fprintf(stderr, "Error creating thread \n");
                    return 1;
                }
            }

        }
            
        //block processing complete, print to stdout
    	printf("%s", text);
    	
    	//read the next available block (bytes of blocksize)
        result = fread(text, sizeof(char), blocksize, stdin);
        
        /*******************************************
         * Circular shift:
         * 
         * Shift the bits left by one, while retaining
         * the lost bit and placing it as the LSB
         * *****************************************/
        /*
    	//shift the bits in the keyfile to the left by one after each block is processed
    	int loop;

    	for (loop = 0; loop < strlen(buff); loop++)
    	{
	        //y = (x << shift) | ( (x >> (sizeof(x)* CHAR_BIT - shift)) 
    		buff[loop] = (buff[loop] << 1) | (buff[loop] >> (sizeof(buff[loop]) * 8 - 1));
    	}

        */
    }

    //in the event that there is leftover data at the end of a finite size file
    //we will processing the remaining blocks individually without threads, to save overhead
    while ( result > 0 )
    {
        //XOR the block read in with our keyfile provided, print to stdout
        for(i=0; i < result; i++)
        {
            text[i] ^= buff[i];
            printf ("%c", text[i]);
        }
        
        result = fread(text, sizeof(char), blocksize, stdin);
    }
    
    free(source);

    return 0;
}
