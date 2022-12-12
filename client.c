/*
Authors: Viraj Wickramasinghe | Jay Girishkumar Patel
Section: 3
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#define LENGTH 2048

//global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

//flushes the output buffer of a stream
void str_overwrite_stdout() {
  fflush(stdout);
}

//add null character to new lines
void str_trim_lf(char * arr, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

//catch CTRL+C from user
void catch_ctrl_c_and_exit(int sig) {
  flag = 1;
}

//get user input from the terminal
void send_msg_handler() {
  char message[LENGTH] = {};
  char buffer[LENGTH + 32] = {};

  while (1) {
    str_overwrite_stdout();
    fgets(message, LENGTH, stdin);

    str_trim_lf(message, LENGTH);
    char command[50];
    strcpy(command, message);
    system(command);

//quit the session
    if (strcmp(message, "quit") == 0) {
      break;
    } else {
      sprintf(buffer, "%s: %s\n", name, message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

    bzero(message, LENGTH);
    bzero(buffer, LENGTH + 32);
  }
  catch_ctrl_c_and_exit(2);
}

//recieve the message from the server
void recv_msg_handler() {
  char message[LENGTH] = {};
  while (1) {
    int receive = recv(sockfd, message, LENGTH, 0);
    if (receive > 0) {

      str_overwrite_stdout();
    } else if (receive == 0) {
      break;
    } else {
    }
    memset(message, 0, sizeof(message));
  }
}

//main method to handle the process
int main(int argc, char ** argv) {
  //check for number of arguments
  if (argc != 3) {
    printf("Usage: %s <IP> <port>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char * ip = argv[1];
  int port = atoi(argv[2]);

//signal to catch the CTRL+C
  signal(SIGINT, catch_ctrl_c_and_exit);

  printf("Please enter your name: ");
  fgets(name, 32, stdin);
  str_trim_lf(name, strlen(name));

  //user input validation
  if (strlen(name) > 32 || strlen(name) < 2) {
    printf("Name must be less than 30 and more than 2 characters.\n");
    return EXIT_FAILURE;
  }

//define socket
  struct sockaddr_in server_addr;
  
  int fd = open("test.txt", O_RDWR, 0777);
  char buff[4];
  long int n = read(fd, buff, 4);
  close(fd);

  //socket configuration
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  //redirection to next server
  if (strcmp(buff, "4003") == 0) {
    port = atoi(buff);
  }

  server_addr.sin_port = htons(port);

  //server connection
  int err = connect(sockfd, (struct sockaddr * ) & server_addr, sizeof(server_addr));
  if (err == -1) {
    printf("ERROR: connect\n");
    return EXIT_FAILURE;
  }

  // Send name
  send(sockfd, name, 32, 0);

  printf("=== WELCOME ===\n");

  //thread to invoke send messages from client
  pthread_t send_msg_thread;
  if (pthread_create( & send_msg_thread, NULL, (void * ) send_msg_handler, NULL) != 0) {

    printf("ERROR: pthread\n");
    return EXIT_FAILURE;
  }
  //thread to invoke recieve messages from server
  pthread_t recv_msg_thread;
  if (pthread_create( & recv_msg_thread, NULL, (void * ) recv_msg_handler, NULL) != 0) {
    printf("ERROR: pthread\n");
    return EXIT_FAILURE;
  }

  while (1) {
    if (flag) {
      printf("\nBye\n");
      break;
    }
  }

  close(sockfd);

  return EXIT_SUCCESS;
}
