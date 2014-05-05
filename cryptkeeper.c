#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct block_info
{
    int thread_id;
    int data_value;
    int key_value;
    int encrypted_value;
}block;

void *encrypt(void *thread_ptr)
{
    block *data = (block *) thread_ptr;
    
    //printf("This is my passed thread id: %x\n", data->thread_id);
    //printf("This is my passed data value: %x\n", data->data_value);
    ////printf("This is my passed key value: %x\n", data->key_value);
    data->encrypted_value = data->key_value ^ data->data_value;
    //printf("This is the new encrypted value: %x\n", data->encrypted_value);
    
    return (void * ) data;
}

int main(int argc, char *argv[])
{
    //put a switch statement here for input paramaters, assign variables as needed.
    int num_threads = 0;
    
    sscanf (argv[1], "%d", &num_threads);
    
    pthread_t threads[num_threads];
    
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

    //printf("You have chosen to utilize %d number of threads for this process.\n", num_threads);
    //printf("The key file chosen was %s.\n", buff);
    //printf("");

    int blocksize = strlen(buff);
    //printf("blocksize is: %d\n", blocksize);
    char text[blocksize];
    
    char* plaintext = text;
    char *buff2 = plaintext;
    int i, j;
    
    //read bytes, since we can't rely on characters themselves (i.e Unicode vs ASCII)
    int result = fread(text, sizeof(char), blocksize, stdin);
    
    //printf("The initial read was: %d\n", result);
    
    block encrypt_go[blocksize];
    
    //so long as enough blocksize bytes are available, continue looping
	//no threading will be used if they don't want it (i.e. threads > 2)
    while ( num_threads >= 2 && result == blocksize )
	//while ( num_threads >= 2 && blocksize > result )
    {
        //printf("The plaintext read in is: %s\n", plaintext);
        //printf("the result of the read was: %d\n", result);

            //XOR the block read in with our keyfile provided, print to stdout
            //spawn thread for this work

            
            for(i=0; i <= blocksize; i++)
	    //for(i=0; i <= result; i++)
            {
                //printf("What if I just print what text[i] actually is here before processing?: %x\n", text[i]);
                //text[i] ^= buff[i];
                //printf("The XOR result of block %d is %x\n", i, text[i]);
                //printf("%c\n", text[i]);
                int index = i % num_threads;
                int temp_i = i;
                
		if (temp_i < result)
		{
                	encrypt_go[temp_i].thread_id = temp_i;
                	encrypt_go[temp_i].data_value = text[temp_i];
                	encrypt_go[temp_i].key_value = buff[temp_i];
		}
 
                //printf("the thread value i am now going to pass is: %x\n", encrypt_go[index].thread_id);
                //printf("the data value i am now going to pass is: %x\n", encrypt_go[index].data_value);
               // printf("the key value i am now going to pass is: %x\n", encrypt_go[index].key_value);
                
                //@@@@@@@@@@@@
                //index is going to overlap
                
                //whats needed here is the wrap up processing if i > num_threads
                if ((temp_i >= num_threads && index == 0) ||
                     blocksize == num_threads && num_threads == temp_i)) //|| temp_i == result && result )
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
                            
                            //printf("This is the thread id return value of the thread #%d: %x\n", j, (int) return_block->thread_id);
                            //printf("This is the data value return value of the thread #%d: %x\n", j, (int) return_block->data_value);
                            //printf("This is the key value return value of the thread #%d: %x\n", j, (int) return_block->key_value);
                            //printf("This is the encrypted value return value of the thread #%d: %x\n", j, (int) return_block->encrypted_value);
                            text[returned_thread] = encrypted_value;
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

/*
	//shift the bits in the keyfile to the left by one after each block is processed
	int loop;
	printf("Old key file value: %s\n", buff);
	int shift_left;
	int shift_right;
	long or;

	int z = 0;
	for (z = 0; z < 2; z++)
	{
	for (loop = 0; loop < strlen(buff); loop++)
	{
		printf("Old key file value at %d: %d and hex: %x\n", loop, buff[loop], buff[loop]);
		shift_left = buff[loop] << 1;
		printf("Old key file shift_left %d: %d and hex: %x\n", loop, shift_left, shift_left);
		shift_right = buff[loop] >> sizeof(buff[loop]) * 8 - 1;
		printf("Old key file shift_right %d: %d and hex: %x\n", loop, shift_right, shift_right);
		or = shift_left | shift_right;
		
		//shift = (buff[loop] << 1) | (buff[loop] >> (sizeof(buff[loop]) * 8 - 1)) & (0x7F >> (sizeof(buff[loop])*8 - 1));
		//y = (x << shift) | ( (x >> (sizeof(x)*CHAR_BIT - shift)) 
		
		buff[loop] = or;
		printf("shifted key file value at %d: %d and hex: %x\n", loop, buff[loop], buff[loop]);
	}
}
	printf("New key file value: %s\n", buff);
*/
        result = fread(text, sizeof(char), blocksize, stdin);
        //printf("The next readin is %d\n", result);
    }

    //we've reached the end of the plaintext file
    //if there are any leftover data pieces (which will be smaller than block size)
    //finish them off so we can complete the process.
    
    if ( 0 < result && result < blocksize )
    {
        //so long as enough blocksize bytes are available, continue looping
        while ( result > 0 )
        {
            ////printf("The leftover plaintext read in is: %s\n", text);
            ////printf("the result of the read was: %d\n", result);

                //XOR the block read in with our keyfile provided, print to stdout
                for(i=0; i < result; i++)
                {
                    text[i] ^= buff[i];
                    //printf("The XOR result of block %d is %x\n", i, text[i]);
                    //printf("%c\n", text[i]);
                }
                
            printf ("%s", text);
        
            result = fread(text, sizeof(char), blocksize, stdin);
            //printf("The next readin is %d\n", result);
        }
 
    }


    free(source); /* Don't forget to call free() later! */

}
