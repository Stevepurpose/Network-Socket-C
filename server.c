#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define PORT "3490" //The port users connect to
#define BACKLOG 10   //pending connections on queue



void sigchld_handler(int s){
    (void)s;
 //waitpid() might overwrite errno, so we save and restore it
int saved_errno = errno;
while(waitpid(-1, NULL, WNOHANG) > 0);
errno = saved_errno;
}

//getsocket addr, IP address
void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family==AF_INET){
    return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void){
    int sockfd, new_fd; //listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;  //connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv; //For error checking the value getaddrinfo returns

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE; //Use my IP

//getaddrinfo returns 0 on success and a non zero number on failure. 
if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo))!= 0){
    //gai_strerror converts rv to a human readable string to show cause, print to the standard error stream
    fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(rv));
    return 1;
}
//if no error, servinfo points to a linked list of struct addrinfo, with each containing a struct sockaddress
//loop through all the results and bind to the first we can
//This result is the linkedlist servinfo points to, servinfo is the head
//so we loop through the linkedlist of struct addrinfo, with p the temporary pointer
for(p = servinfo; p!=NULL; p = p->ai_next){

if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))== -1){
    perror("server: socket");
    continue;
}

//if no error in socket() call, we set socket options
//SO_REUSEADDR allows reuse of address immediately socket is closed. Even if it is in a TIME_WAIT state
if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))== -1){
    perror("setsockopt");
    exit(1);
}

//if no error in setting options, the socket is bound to a specific address 
if(bind(sockfd, p->ai_addr, p->ai_addrlen)==-1){
    close(sockfd);
    perror("server:bind");
    continue;
}

break; // break out of loop on successful  or unsuccessful bind
}
freeaddrinfo(servinfo);

//unsuccessful bind
if(p==NULL){
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
}

//successful bind, listen for incoming connections
if(listen(sockfd, BACKLOG)== -1){
    perror("listen");
    exit(1);
}


//use a type of struct sigaction, manipulate the action taken by the process on interrupt by signals
sa.sa_handler = sigchld_handler; //sa_handler is pointer to a signal handler function
sigemptyset(&sa.sa_mask); //initialize an empty signal set, clear previous signals
sa.sa_flags =SA_RESTART;
if(sigaction(SIGCHLD, &sa, NULL)== -1){  //there is a sigaction function that is a pointer 
// to another signal handler function
perror("sigaction");
exit(1);

}

printf("server waiting for connections\n");
while(1){
    //main accept() loop
 sin_size = sizeof their_addr;
 new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
if(new_fd == -1){
    perror("accept");
    continue;
}

//convert binary representation of IP Address to string
inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
printf("server: got connection from %s\n", s);


//create child process for each client connection
//sever handles multiple clients without blocking the main process

if(!fork()){
   //! before fork() only executes the code in the child process a
    close(sockfd); //child process close original listening socket
    if(send(new_fd, "Hello, world!", 13, 0)== -1);
       perror("send");
    close(new_fd);//on successful send
    exit(0);//exit child process
}

close(new_fd); //parent process executes this code closing socket connected to client and continues
//looping to accept more connections
}
return 0;
}