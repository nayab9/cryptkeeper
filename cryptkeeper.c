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
 * "Encrypto Patronus!" - A once famous Wizard
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
    char *keyfile;
    char *source = NULL;
    int num_threads = 0;
    int length = argc - 1;
    int loop, i, j;
        
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
            long bufsize = ftell(fp);
            if (bufsize == -1) 
            {
                fprintf(stderr, "Error reading keyfile, exiting.. \n");
                return 2;                    
            }
    
            source = (char *)malloc(sizeof(char) * (bufsize + 1));
    
            if (fseek(fp, 0L, SEEK_SET) != 0) 
            {
                fprintf(stderr, "Error reading keyfile, exiting.. \n");
                return 2;  
            }
    
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if (newLen == 0) 
            {
                fprintf(stderr, "Error opening keyfile, exiting.. \n");
                return 2;    
            } 
            else 
            {
                source[++newLen] = '\0';
            }
        }
        fclose(fp);
    }
    else
    {
        fprintf(stderr, "Error opening keyfile, exiting.. \n");
        return 2;       
    }

    //assign a pointer to the keyfile
    //this will be used from here on out for encrypting
    char *buff = source;

    //retrive the blocksize based on the keyfile
    int blocksize = strlen(buff);
    
    //if threads specified is more than the blocksize
    //force threads to blocksize, since thats the max
    //number of threads we can utilize
    if (num_threads > blocksize)
    {
        num_threads = blocksize;
    }
    
    //this is where we store the XOR'd values to printout
    char text[blocksize];

    //read by bytes, since we can't rely on characters themselves (i.e Unicode vs ASCII)
    int result = fread(text, sizeof(char), blocksize, stdin);
    
    //initialize the struct blocks and threads needed for processing
    block encrypt_go[blocksize];
    pthread_t threads[num_threads];
    
    //so long as enough blocksize bytes are available, continue looping
	//no threading will be used if they don't want it (i.e. threads > 2)
    while ( num_threads >= 2 && result == blocksize )
    {
        //loop through block and encrypt
        for(i=0; i <= blocksize; i++)
        {
            int index = i % num_threads;
            int temp_i = i;
            
    		if (temp_i < result)
    		{
            	encrypt_go[temp_i].thread_id = temp_i;
            	encrypt_go[temp_i].data_value = text[temp_i];
            	encrypt_go[temp_i].key_value = buff[temp_i];
    		}

            //whats needed here is the wrap up processing if i > num_threads
            if ((temp_i >= num_threads && index == 0) ||
                 blocksize == num_threads && num_threads == temp_i )
                 //|| temp_i == result && result )
            {
                //printf("I should wrap it up right MEOW!\n") ;   
                //this will seg fault if num_threads is less than blocksize
                //wait for threads to finish processing
                for (j = 0; j < num_threads; j++)
                {
                    void *returnValue;
 		//printf("I did qualify for wrapping up, my i value is %d, and index is %d\n", temp_i, index);                       
                    if (pthread_join( threads[j], &returnValue ))
                    {
                        fprintf(stderr, "error joining thread \n");
                        return 2;
                    }
                    else
                    {
                        block *return_block = (block *) returnValue;
                        int encrypted_value = (int) return_block->encrypted_value;
                        int returned_thread = (int) return_block->thread_id;
                        
                        //printf("This is the thread id return value of the thread       #%d: %x\n", j, (int) return_block->thread_id);
                        //printf("This is the data value return value of the thread      #%d: %x\n", j, (int) return_block->data_value);
                        //printf("This is the key value return value of the thread       #%d: %x\n", j, (int) return_block->key_value);
                        //printf("This is the encrypted value return value of the thread #%d: %x\n", j, (int) return_block->encrypted_value);
                        text[returned_thread] = encrypted_value;
                        //printf("%x", text[returned_thread]);
                    }
                }
               
    			if (temp_i < result)
    			{ 
    				//printf("I'm still not done the block, I'm creating a new thread overlapping at index: %d, with i value: %d", index, temp_i);
    		           if (pthread_create( &threads[index], NULL, encrypt, (void *) &encrypt_go[temp_i] ))
    		            {
    		                fprintf(stderr, "Error creating thread \n");
    		                return 1;
    		            }
    		            else
    		            {
    		                //printf("I sent this through to the thread #%d: %x\n", index, encrypt_go[index].data_value);
    		            } 
    			}                                   
                
            }
            else
            {
                //printf("I didn't qualify for wrapping up, my i value is %d, and index is %d\n", temp_i, index);
                if (pthread_create( &threads[index], NULL, encrypt, (void *) &encrypt_go[temp_i] ))
                {
                    fprintf(stderr, "Error creating thread \n");
                    return 1;
                }
                else
                {
                    //printf("I sent this through to the thread #%d: %x\n", index, encrypt_go[index].data_value);
                }                
                
            }

        }
            
       //printf ("the entire string is what? %s\n", text);
    	printf("%s", text);

        result = fread(text, sizeof(char), blocksize, stdin);
        //printf("The next readin is %d\n", result);
    }

    //we've reached the end of the plaintext file
    //if there are any leftover data pieces (which will be smaller than block size)
    //finish them off so we can complete the process.

    //so long as enough blocksize bytes are available, continue looping
    while ( result > 0 )
    {
        ////printf("The leftover plaintext read in is: %s\n", text);
        ////printf("the result of the read was: %d\n", result);

            //XOR the block read in with our keyfile provided, print to stdout
            for(i=0; i < result; i++)
            {
                text[i] ^= buff[i];
                printf ("%c", text[i]);
                //printf("The XOR result of block %d is %x\n", i, text[i]);
                //printf("%c\n", text[i]);
            }
    
        result = fread(text, sizeof(char), blocksize, stdin);
        //printf("The next readin is %d\n", result);
    }
 

    free(source); /* Don't forget to call free() later! */

}
