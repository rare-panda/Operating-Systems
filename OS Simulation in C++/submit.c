#include <pthread.h>
#include <semaphore.h>
#include "simos.h"


//===============================================================
// The interface to interact with clients for program submission
// --------------------------
// Should change to create server socket to accept client connections
// -- Best is to use the select function to get client inputs
// Should change term.c to direct the terminal output to client terminal
//===============================================================

sem_t submit_mutex, submit_empty;


//=========================================================================
// terminal queue 
// implemented as a list with head and tail pointers
//=========================================================================

typedef struct SubmitQnodeStruct
{
  int sockfd;
  char *str;
  struct SubmitQnodeStruct *next;
} SubmitQnode;

SubmitQnode *submitQhead = NULL;
SubmitQnode *submitQtail = NULL;


// dump terminal queue is not called inside the terminal thread,
// only called by admin.c
void dump_submitIO_queue (FILE *outf)
{ SubmitQnode *node;

  fprintf (outf, "******************** Submit Queue Dump\n");
  node = submitQhead;
  while (node != NULL)
  { fprintf (outf, "%d %s\n", node->sockfd, node->str);
    node = node->next;
  }
  fprintf (outf, "\n");
}

// insert terminal queue is called by the main thread when
//    terminal output is needed (only in cpu.c, process.c)
void insert_submitIO (sockfd, outstr)
int sockfd;
char *outstr;
{ SubmitQnode *node;
  if (submitDebug) fprintf (bugF, "Insert submit queue %d %s\n", sockfd, outstr);
  node = (SubmitQnode *) malloc (sizeof (SubmitQnode));
  node->sockfd = sockfd;
  node->str = outstr;
  node->next = NULL;
  if (submitQtail == NULL) // termQhead would be NULL also
    { submitQtail = node; submitQhead = node; }
  else // insert to tail
    { submitQtail->next = node; submitQtail = node; }
  if (submitDebug) dump_submitIO_queue (bugF);
}

// remove the termIO job from queue and call terminal_output for printing
// after printing, put the job to endIO list and set endIO interrupt
void handle_submitIO ()
{ SubmitQnode *node;
  sem_wait (&submit_empty);
  sem_wait (&submit_mutex);
  if (submitDebug) dump_submitIO_queue (bugF);
  while(submitQhead != NULL)
  { node = submitQhead;
    //printf("submit process called with %s\n",node->str);
    submit_process (node->str, node->sockfd);

    if (submitDebug)
      fprintf (bugF, "Remove submit queue %d %s\n", node->sockfd, node->str);
    submitQhead = node->next;
    if (submitQhead == NULL) submitQtail = NULL;
    free(node->str);free (node);
    if (submitDebug) dump_submitIO_queue (bugF);
  }
  sem_post (&submit_mutex);
}



void *submitIO (void *port_num)
{
  int sockfd, newsockfd, portno, clilen;
  char buffer[1500];
  struct sockaddr_in serv_addr, cli_addr;
  int ret,fdmax;
  fd_set master;
  fd_set read_fds;
  
  FD_ZERO(&master);

  FD_ZERO(&read_fds);
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");
  bzero ((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi((char *)port_num);
  //printf("INT:%d\n",portno);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
      error("ERROR binding");
  
  if(listen(sockfd, 10) == -1)
  {
    error("Server-listen() error lol!");
	exit(1);
  }
  
  FD_SET(sockfd, &master);
  /* keep track of the biggest file descriptor */
  fdmax = sockfd; /* so far, it's this one*/

  while (systemActive) 
  {
	/* copy it */
    read_fds = master;

    if(select(fdmax+1, &read_fds, NULL, NULL, NULL) < 0)
    {
      error("Server-select() error lol!");
      exit(1);
    }
    int i;
	
	/*run through the existing connections looking for data to be read*/
    for(i = 0; i <= fdmax; i++)
    {
      if(FD_ISSET(i, &read_fds))
      {
		  /* we got one... */
        if(i == sockfd)
        {
			clilen = sizeof(cli_addr);
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0) 
			  error ("ERROR accepting");
			else
			{
			  FD_SET(newsockfd, &master); /* add to master set */
			  if(newsockfd > fdmax)
			  {
				/* keep track of the maximum */
				fdmax = newsockfd;
			  }
			}
		}
		else
        {
			bzero(buffer,1500);
			int nbytes;
            if((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0)
            {
	      //printf("%d\n",nbytes);
              /* got error or connection closed by client */

			  /* close it... */
              close(i);
			
              /* remove from master set */
              FD_CLR(i, &master);
            }
			else
			{
			       // printf("%d\n",nbytes);
				char *str = (char *)malloc(10);
				sprintf(str, "prog%d", i);
			        //printf("Received file %s\n",str);
			        //printf("Received Content %s\n",buffer);
				FILE *fp = fopen(str,"w+");
				ret = fputs(buffer,fp);
				if(ret == EOF)
					error("Error writing to buffer\n");
				fclose(fp);
				sem_wait (&submit_mutex);
				if(!systemActive && submitQtail == NULL)
					break;
				//printf("Before Insert:%d %s\n",i, str);
				insert_submitIO (i, str);
				sem_post (&submit_empty);
				sem_post (&submit_mutex);
				set_interrupt(submitInterrupt);
				//printf("Interrupt Set\n");
			}
		}
	  }
	  
	}
  }
}


pthread_t submitThread;

void start_submission_manager (char *port_num)
{ char fname[100];
  sem_init( &submit_mutex, 0, 1 );
  sem_init( &submit_empty, 0, 0 );
  pthread_create (&submitThread, NULL, submitIO, port_num);
  fprintf (infF, "Submit Manager thread has been created succesfully\n");
 // printf("PortNum:%s\n",port_num);
}

// cleaning up and exit the terminal thread, invoked by system.c
void end_submission_manager()
{
  int ret;
  sem_post (&submit_empty);
  pthread_cancel (submitThread);
  sem_close(&submit_mutex);
  sem_close(&submit_empty);
  fprintf (infF, "Submit Manager thread has terminated %d\n",ret);
}


