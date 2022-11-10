#include <stdio.h>
#include <pthread.h>
#include "meeting_request_formats.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>
#include "queue_ids.h"
#ifndef darwin
size_t                        /* O - Length of string */
strlcpy(char       *dst,      /* O - Destination string */
        const char *src,      /* I - Source string */
        size_t      size)     /* I - Size of destination string buffer */
{
    size_t    srclen;         /* Length of source string */

    /*
     * Figure out how much room is needed...
     */

    size --;

    srclen = strlen(src);

    /*
     * Copy the appropriate amount...
     */

    if (srclen > size)
        srclen = size;

    memcpy(dst, src, srclen);
    dst[srclen] = '\0';

    return (srclen);
}
#endif

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sendLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reqThread = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t conditionVariables[200] = { PTHREAD_COND_INITIALIZER };
int *response; 
int done;
void* send_message(void* rbuf);
void* receive_message();
void print_response(meeting_request_buf *rbuf);


/*
request_mtgs reads meeting requests from stdin for employees.
Each request is checked against an employee's existing schedule for conflicts
by sending the request to the CalendarManager program via the System V queue.
The CalendarManager will respond with a message that indicates whether the meeting
was added to the calendar by indicating availability (availability of 0 means conflict,
availability of 1 means added). The request_mtgs will print out the result (conflict or added) of each meeting request.
*/

void* send_message(void* input){

    key_t key;
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    int return_code;
    int *status = malloc(sizeof(int));
     
    meeting_request_buf *rbuf = (meeting_request_buf*)input;

   

    key = ftok(FILE_IN_HOME_DIR,QUEUE_NUMBER);

    if ((msqid = msgget(key, msgflg)) < 0) {
        int errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("(msgget)");
        fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
    }

#ifdef DEBUG
    else
        fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", msqid);
#endif

    // We'll send message type 2
    rbuf->mtype = 2;
    rbuf->request_id= rbuf->request_id;
    strncpy(rbuf->empId,rbuf->empId,EMP_ID_MAX_LENGTH);
    strncpy(rbuf->description_string,rbuf->description_string,DESCRIPTION_MAX_LENGTH);
    strncpy(rbuf->location_string,rbuf->location_string,LOCATION_MAX_LENGTH);
    strncpy(rbuf->datetime,rbuf->datetime,DATETIME_LENGTH);
  
 

    pthread_mutex_lock(&sendLock);
    return_code = msgsnd(msqid, rbuf, SEND_BUFFER_LENGTH, IPC_NOWAIT);
    pthread_mutex_unlock(&sendLock);

    if(return_code < 0) {
        int errnum = errno;
        fprintf(stderr,"%d, %ld, %d, %ld\n", msqid, rbuf->mtype, rbuf->request_id, SEND_BUFFER_LENGTH);
        perror("(msgsnd)");
        fprintf(stderr, "Error sending msg: %s\n", strerror( errnum ));
        exit(1);
    }
    
    else {
       // wait for response 
       pthread_mutex_lock(&lock);
        while(done == 0 ){
             pthread_cond_wait(&conditionVariables[rbuf->request_id], &reqThread); 
        }
        pthread_mutex_unlock(&lock);
        
        
            if (response[rbuf->request_id] == 1){
                printf("Meeting request %d for employee %s was accepted (%s @ %s starting %s for %d minutes)\n", rbuf->request_id, rbuf->empId, rbuf->description_string, rbuf->location_string, rbuf->datetime, rbuf->duration);
            }
            else if (response[rbuf->request_id] == 0 ) {
                printf("Meeting request %d for employee %s was rejected due to conflict (%s @ %s starting %s for %d minutes)\n", rbuf->request_id, rbuf->empId, rbuf->description_string, rbuf->location_string, rbuf->datetime, rbuf->duration);
            }
            else {
                fprintf(stderr, "Received unrecognized status code: %d\n", response[rbuf->request_id]);
            }
      
     

    }
    
    }





void* receive_message(){
  while(1){
    
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    int ret;
  
    meeting_response_buf mybuf; 



    key = ftok(FILE_IN_HOME_DIR,QUEUE_NUMBER);
    if (key == 0xffffffff) {
        fprintf(stderr,"Key cannot be 0xffffffff..fix queue_ids.h to link to existing file\n");
        //return 1;
    }

    
     if ((msqid = msgget(key, msgflg)) < 0) {
        int errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("(msgget)");
        fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
    }

       
        ret = msgrcv(msqid, &mybuf, sizeof(meeting_response_buf) - sizeof(long), 1, 0);

        pthread_mutex_lock(&reqThread);

        response[mybuf.request_id] = mybuf.avail;
        int errnum = errno;
        if (ret < 0 && errno !=EINTR){
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("Error printed by perror");
        fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum )); }
 
 
    done =1;
    pthread_cond_signal(&conditionVariables[mybuf.request_id]);
    pthread_mutex_unlock(&reqThread);
    }
}

 

int main(int argc, char **argv){

char *messageLine = (char *)malloc(sizeof(char)*SEND_BUFFER_LENGTH);
meeting_request_buf *rbuf = malloc(sizeof(struct meetingrequestbuf)); 
response  = malloc(sizeof(int) * 201);


pthread_t thread[200];
int k=0;
int j=0;
pthread_t final_thread;
pthread_t reader_thread;
size_t size = SEND_BUFFER_LENGTH; 

pthread_create(&reader_thread, NULL, (void*)receive_message, NULL);

 while (getline(&messageLine, &size, stdin)){       
    k++;
    meeting_request_buf *rbuf = malloc(sizeof(struct meetingrequestbuf));
    
    // remove quotations
    for (int i = 0; i < SEND_BUFFER_LENGTH; i ++) {
            if (messageLine[i] != '"' && messageLine[i] != '\\') { 
                 messageLine[j++] = messageLine[i];
            } else if (messageLine[i+1] == '"' && messageLine[i] == '\\') { 
                 messageLine[j++] = '"';
            } else if (messageLine[i+1] != '"' && messageLine[i] == '\\') { 
                 messageLine[j++] = '\\';
            }
            }

    
    sscanf(messageLine,"%d,%[^,],%[^,],%[^,],%[^,],%d", &rbuf->request_id, rbuf->empId, rbuf->description_string, rbuf->location_string, rbuf->datetime, &rbuf->duration);  // parse each line
    if (rbuf->request_id == 0){ break;}

    pthread_create(&thread[k-1], NULL, send_message, rbuf);

    }
    
for(int index = 0; index < k - 1; index++) {
 pthread_join(thread[index], NULL);
}
    // send final message to signal end of input on java side
    //pthread_create(&final_thread, NULL, send_message, rbuf);

    // wait for last request and response reader to finish 
   // pthread_join(final_thread, NULL);
    pthread_join(reader_thread, NULL);
  

    // cleanup
    free(messageLine);
    free(rbuf);
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&sendLock);
    pthread_mutex_destroy(&reqThread);
    return 0;
}

// things to do:
// deallocate mem
// destroy locks and condiion variables
// fix input to accomodate comma in middle of string
// formatting
// back up files 
// terminate on 0


