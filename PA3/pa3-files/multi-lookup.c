//Max Harris CSCI 3753
//PA3

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h> // to make pthreads, duh...
#include <unistd.h>

#include "queue.h"
#include "util.h"

#include "multi-lookup.h"

#define MINARGS 3 
#define USAGE "<inputFilePath> <outputFilePath>" 
#define BUFSIZE 1024 
#define MAX_NAME_LIMIT 225 
#define INPUTFS "%1024s"
#define QUEUE_SIZE 50

int main(int argc, char* argv[]){ 

	int infiles = argc-2; 
	int i,t,rc; 
	int finished=1;
	int MAX_RESOLVER_THREADS = sysconf(_SC_NPROCESSORS_ONLN); // Get the number of logical CPUs.

	FILE* inputFiles[infiles]; 
	FILE* outputfile=NULL; 
	
	pthread_t request_threads[infiles];
	pthread_t resolver_threads[MAX_RESOLVER_THREADS];
	pthread_mutex_t queuelock;
	pthread_mutex_t filelock;
	queue shared;

	in_params inparameters[infiles];
	out_params outparameters[MAX_RESOLVER_THREADS];
	
    /* Check Arguments -- Move this code to before assigning input files above */
	if(argc < MINARGS){
		fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
		return EXIT_FAILURE;
    }
	
	if(queue_init(&shared, QUEUE_SIZE)==QUEUE_FAILURE) {
		fprintf(stderr,"Error creating the Queue\n");
		return EXIT_FAILURE; 
	}
	
    // ######################################
    // #####    CREATE A QUEUE MUTEX    #####
    // ######################################
    
	rc=pthread_mutex_init(&queuelock, NULL); // NULL for attributes uses default attributes
    
	if(rc){ 
		fprintf(stderr, "Error creating the Queue Mutex\n");
		fprintf(stderr, "Error No: %d\n",rc);  
		return EXIT_FAILURE; 
	}

    // ######################################
    // #####    CREATE A FILE MUTEX     #####
    // ######################################
    
	rc=pthread_mutex_init(&filelock,NULL); // NULL for attributes uses default attributes
    
	if(rc){ 
		fprintf(stderr, "Error creating the output file Mutex\n");
		fprintf(stderr, "Error No: %d\n",rc);  
		return EXIT_FAILURE; 
	}
	
    
    // ######################################
    // #####    OPEN THE OUTPUT FILE    #####
    // ######################################

	outputfile = fopen(argv[(argc-1)], "w");
	    if(!outputfile) {
            fprintf(stderr,"Error Opening Output File\n");
            return EXIT_FAILURE;
        }

    
    // ######################################
    // #####    OPEN ALL INPUT FILES    #####
    // ######################################

 
	for (i = 1; i < argc-1; i++) {
	    inputFiles[i-1] = fopen(argv[i], "r");
	    if(!inputFiles[i-1]) { 
	       fprintf(stderr, "Error Opening Input File: %s\n", argv[i]);
	       return EXIT_FAILURE;
	    }
    }

    
    // ##########################################################
    // #####    CREATE and RUN PTHREADS FOR INPUT FILES     #####
    // ##########################################################

    
    for(t=0;t < infiles ;t++){
        FILE* curFile=inputFiles[t];
        inparameters[t].queueL=&queuelock; 
        inparameters[t].file_name=curFile; 
        inparameters[t].q=&shared;
        
        rc = pthread_create(&(request_threads[t]), NULL, WriteThreads, &inparameters[t]);
        printf("Creating Request Thread %d\n",t);
        if (rc) {
                fprintf(stderr,"ERROR; return code from pthread_create() is %d\n", rc);
                exit(EXIT_FAILURE);
        }
    }

    // ##########################################
    // #####    CREATE and RUN PTHREADS     #####
    // ##########################################


    for(t=0; t < MAX_RESOLVER_THREADS; t++) {
		outparameters[t].queueL=&queuelock; 
		outparameters[t].file_name=outputfile; 
		outparameters[t].outL=&filelock; 
		outparameters[t].q=&shared;
		outparameters[t].finished=&finished;
        rc = pthread_create(&(resolver_threads[t]), NULL, ReadThreads, &(outparameters[t]));
        printf("Creating Resolver Thread %d\n",t);
        
        if (rc) {
            fprintf(stderr,"ERROR; return code from pthread_create() is %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }

	
	for(t=0;t<infiles;t++){
        pthread_join(request_threads[t],NULL);
	printf("Request Thread %d has writen to the buffer\n",t);
    }

	finished=0;

	for(t=0;t<MAX_RESOLVER_THREADS;t++){
        pthread_join(resolver_threads[t],NULL);
	printf("Resolver Thread %d has read from the buffer\n",t);
    }


	if(fclose(outputfile)){
		fprintf(stderr, "Error Closing output file \n");
	}
 
	queue_cleanup(&shared); 
	pthread_mutex_destroy(&queuelock);
	pthread_mutex_destroy(&filelock);
	printf("Finished!\n");

	return EXIT_SUCCESS;
}

void* WriteThreads(void* p) {
	char hostname[MAX_NAME_LIMIT];
	in_params* parameters = p;
	FILE* fName= parameters->file_name;
	pthread_mutex_t* queuelock = parameters-> queueL; 
	queue* shared= parameters->q;
	char* payload;
	int writesuccess = 0;
	int rc = 0;
	while(fscanf(fName, INPUTFS, hostname) > 0){
		while(!writesuccess){ 		
			rc=pthread_mutex_lock(queuelock);
			if(rc){
				fprintf(stderr, "Queue Mutex lock error %d\n",rc); 
			}
			if(queue_is_full(shared)){
				rc=pthread_mutex_unlock(queuelock);
				if(rc){
					fprintf(stderr,"Queue Mutex unlock error %d\n", rc);
				}				

				usleep((rand()%100)*1000); 
			}
			else {
				payload=malloc(MAX_NAME_LIMIT);
				if(payload==NULL) {
					fprintf(stderr, "Malloc returned an error\n");
					fprintf(stderr, "Warning results non-deterministic\n");
				}
				
				payload=strncpy(payload, hostname, MAX_NAME_LIMIT); 
				if(queue_push(shared,payload)==QUEUE_FAILURE){
					fprintf(stderr, "Queue Push error\n");
				}
			
				rc=pthread_mutex_unlock(queuelock);
				if(rc){
					fprintf(stderr,"Queue Mutex unlock error %d\n", rc);
				} 
				 
				writesuccess=1;
			}
		}
		
		writesuccess=0;
	}

	if(fclose(fName)){
		fprintf(stderr, "Error closing input file \n");
	}
	return NULL;
}
void* ReadThreads(void* p) {
	out_params* parameters = p;
	FILE* outfile= parameters->file_name;
	pthread_mutex_t* queuelock = parameters-> queueL; 
	pthread_mutex_t* filelock = parameters->outL;
	queue* shared= parameters->q;
	int* finished= parameters->finished;
	char* payload;
	char ipaddress[INET6_ADDRSTRLEN];
	int rc = 0;

	while(*finished || !queue_is_empty(shared)){
			rc=pthread_mutex_lock(queuelock);
		if(rc){
			fprintf(stderr, "Queue Mutex lock error %d\n",rc); 
		}
	 
		payload=queue_pop(shared);
		if(payload==NULL)
		{
			rc=pthread_mutex_unlock(queuelock);
			if(rc){
				fprintf(stderr, "Queue Mutex unlock error %d\n", rc);
			}		
			usleep((rand()%100)*1000);
		}
		else {
			rc=pthread_mutex_unlock(queuelock);
			if(rc){
				fprintf(stderr, "Queue Mutex unlock error %d\n", rc);
			}		
			if(dnslookup(payload, ipaddress, sizeof(ipaddress))
			 	 == UTIL_FAILURE){ 
                		strncpy(ipaddress, "", sizeof(ipaddress));
                		fprintf(stderr, "dnslookup error: %s\n", payload);
			}		
			rc=pthread_mutex_lock(filelock);
			if(rc){
				fprintf(stderr, "File Mutex lock error %d\n", rc);
			}
			rc=fprintf(outfile,"%s,%s\n", payload, ipaddress);
			if(rc<0){
				fprintf(stderr, "Output file write error\n");
			}			
			rc=pthread_mutex_unlock(filelock);
			if(rc){
				fprintf(stderr, "File Mutex unlock error %d\n", rc);
			}
			free(payload);
			payload=NULL;
		}

	}

	return NULL;
}
