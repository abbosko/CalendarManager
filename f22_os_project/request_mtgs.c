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
sem_t mutex;
sem_t fill;
sem_t empty;

/* 
request_mtgs reads meeting requests from stdin for employees.
Each request is checked against an employee's existing schedule for conflicts 
by sending the request to the CalendarManager program via the System V queue. 
The CalendarManager will respond with a message that indicates whether the meeting 
was added to the calendar by indicating availability (availability of 0 means conflict,
availability of 1 means added). The request_mtgs will print out the result (conflict or added) of each meeting request.
*/
/*
void parse_message(char* messageLine, meeting_request_buf *rbuf) {

sscanf(messageLine,"%d,%[^,],%[^,],%[^,],%[^,],%d", &rbuf->request_id, rbuf->empId, rbuf->description_string, rbuf->location_string, rbuf->datetime, &rbuf->duration);  // parse each line
if use call with no & 
}

*/
void * send_message(char* messageLine){
    meeting_request_buf *rbuf = malloc(sizeof(struct meetingrequestbuf));
    key_t key;
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    int return_code;
    int status;

    
    // set buffer values
    sscanf(messageLine,"%d,%[^,],%[^,],%[^,],%[^,],%d", &rbuf->request_id, rbuf->empId, rbuf->description_string, rbuf->location_string, rbuf->datetime, &rbuf->duration);  // parse each line

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
    strncpy(rbuf->empId,rbuf->empId,EMP_ID_MAX_LENGTH);
    strncpy(rbuf->description_string,rbuf->description_string,DESCRIPTION_MAX_LENGTH);
    strncpy(rbuf->location_string,rbuf->location_string,LOCATION_MAX_LENGTH);
    strncpy(rbuf->datetime,rbuf->datetime,DATETIME_LENGTH);
    // strncpy(rbuf.empId,"1234",EMP_ID_MAX_LENGTH);
    // strncpy(rbuf.description_string,"Test description",DESCRIPTION_MAX_LENGTH);
    // strncpy(rbuf.location_string,"zoom link",LOCATION_MAX_LENGTH);
    // strncpy(rbuf.datetime,"2022-12-19T15:30",16);
    // rbuf.duration=60;

    // Send a message.
    sem_wait(&empty);
    sem_wait(&mutex);
    return_code = msgsnd(msqid, &rbuf, SEND_BUFFER_LENGTH, IPC_NOWAIT);
    sem_post(&mutex);
    sem_post(&fill);


    if(return_code < 0) {
        int errnum = errno;
        fprintf(stdout,"%d, %ld, %d, %ld\n", msqid, rbuf->mtype, rbuf->request_id, SEND_BUFFER_LENGTH);
        perror("(msgsnd)");
        fprintf(stderr, "Error sending msg: %s\n", strerror( errnum ));
        exit(1);
    }

    else {
    status = receive_message(key, msgflg);
    print_response(rbuf, status);
    //fprintf(stdout,"msgsnd--mtg_req: reqid %d empid %s descr %s loc %s date %s duration %d \n",
    //rbuf->request_id,rbuf->empId,rbuf->description_string,rbuf->location_string,rbuf->datetime,rbuf->duration);
    }

    pthread_exit(0);
}

int receive_message(key_t key, int msgflg){

    int msqid;
    meeting_response_buf rbuf;

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
    // msgrcv to receive mtg request response type 1
    int ret;
    do {
      ret = msgrcv(msqid, &rbuf, sizeof(rbuf)-sizeof(long), 1, 0);//receive type 1 message

      int errnum = errno;
      if (ret < 0 && errno !=EINTR){
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("Error printed by perror");
        fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum ));
      }
    } while ((ret < 0 ) && (errno == 4));


    fprintf(stderr,"msgrcv-mtgReqResponse: request id %d  avail %d: \n",rbuf.request_id,rbuf.avail);
    return rbuf.avail
}

void print_response(meeting_request_buf *rbuf, int status) {
    if (status == 1){
    printf("Meeting request %d for employee %s was accepted (%s @ %s starting %s for %d minutes", rbuf->request_id, rbuf->empId, rbuf->description_string, rbuf->location_string, rbuf->datetime, rbuf->duration);
    }
    else if (status ==0 ) {
    printf("Meeting request %d for employee %s was rejected due to conflict (%s @ %s starting %s for %d minutes", rbuf->request_id, rbuf->empId, rbuf->description_string, rbuf->location_string, rbuf->datetime, rbuf->duration);
    }
    else {
       fprintf(stderr, "Received unrecognized status code: %d\n", status);
    }
}

int main(int argc, char **argv){

char *messageLine = (char *)malloc(sizeof(char)*INPUT_TOTAL_LENGTH);
sem_init(&mutex, 1);
sem_init(&empty,0);
sem_init(&fill, 0);
/*
int request_id;
char *employee_id = (char *)malloc(sizeof(char)*EMP_ID_FIELD_LENGTH);
char *description= (char *)malloc(sizeof(char)*DESCRIPTION_FIELD_LENGTH);
char *location= (char *)malloc(sizeof(char)*LOCATION_FIELD_LENGTH);
char *datetime= (char *)malloc(sizeof(char)*DATETIME_LENGTH);
int duration = (int *)malloc(sizeof(int)*10);
*/
key_t key;
pthread_t thread;
meeting_request_buf *rbuf = malloc(sizeof(struct meetingrequestbuf));


 while (1) {
   fgets(messageLine, INPUT_TOTAL_LENGTH, stdin);          // read in each line
   if (feof(stdin) || messageLine[0]=='0') break;          // request id of 0 mens end of input


    pthread_create(&thread, NULL, (void*)send_message, messageLine);
    pthread_join(thread, NULL);
 }
    exit(0);
}