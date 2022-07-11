#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include "checkLogin.c"
#include "board.c"
#define MAX_LENGTH_USER_NAME 20
#define MAX_LENGTH_PASS_WORD 16
#define RED   "\x1B[31m"
#define RESET "\x1B[0m"
#define GREEN  "\x1B[32m"
int isGameOver;
void playGame(int sockfd);
void *on_signal(void *sockfd)
{
  char buffer[64];
  int n;
  int socket = *(int *)sockfd;
  int *player = (int *)malloc(sizeof(int *));

  while (1)
  {
    bzero(buffer, 64);
    n = read(socket, buffer, 64);

    if (n < 0)
    {
      perror("ERROR reading from socket");
      exit(1);
    }

    if (buffer[0] == 'i' || buffer[0] == 'e' || buffer[0] == 'm' || buffer[0] == '\0')
    {
      if (buffer[0] == 'i')
      {
        if (buffer[2] == 't')
        {
          printf("\nMake your move: (lose -> Give Up || draw -> Ask For Draw)\n");
        }
        if (buffer[2] == 'n')
        {
          printf("\nWaiting for opponent...\n");
        }
        if (buffer[2] == 'p')
        {
          *player = atoi(&buffer[3]);
          if (*player == 2)
          {
            printf("You're blacks (%c)\n", buffer[3]);
          }
          else
          {
            printf("You're whites (%c)\n", buffer[3]);
          }
        }
        if (buffer[2] == 'o')
        {
          isGameOver = 1;
          if (buffer[3] == 'w')
          {
            isGameOver = 1;
            printf("You WIN! =)\n");
          }
          else if (buffer[3] == 'l')
          {
            printf("You LOSE! =(\n");
          } else if (buffer[3] == 'd'){
            printf("DRAW! \n");
          }
          bzero(buffer,64);
          n = read(socket, buffer, 64);
          if (n < 0)
          {
            perror("[-] Disconnected Server!");
            exit(1);
          }
          int eloChange;
          sscanf(buffer,"%d",&eloChange);
          if (eloChange >=  0){
          printf("Your Elo +%d\n",eloChange);
          } else
          printf("Your Elo %d \n",eloChange);
          return;
        }
        if (buffer[2] == 'd'){
          printf("Your Opponent want to draw. Do you accept? (y/Y for Yes, others for No)!\n");
        }
      }
      else if (buffer[0] == 'e')
      {
        // Syntax errors
        if (buffer[2] == '0')
        {
          switch (buffer[3])
          {
          case '0':
            printf(RED "  ↑ ('-' missing)\n" RESET);
            break;
          case '1':
            printf(RED "↑ (should be letter)\n" RESET);
            break;
          case '2':
            printf(RED "   ↑ (should be letter)\n" RESET);
            break;
          case '3':
            printf(RED " ↑ (should be number)\n" RESET);
            break;
          case '4':
            printf(RED " ↑ (out of range)\n" RESET);
            break;
          case '5':
            printf(RED "   ↑ (should be number)\n" RESET);
            break;
          case '6':
            printf(RED "   ↑ (out of range)\n" RESET);
            break;
          case '7':
            printf(RED "(out of range)\n" RESET);
            break;
          case '8':
            printf(RED "     ↑ ('-' missing)\n" RESET);
            break;
          case '9':
            printf(RED "      ↑ (should be number)\n" RESET);
            break;
          }
        }
        else if (buffer[2] == '1')
        {
          switch (buffer[3])
          {
          case '0':
            printf(RED "      ↑ (out of range)\n" RESET);
            break;
          case '1':
            printf(RED "(syntax error)\n" RESET);
            break;
          }
        }

        printf("\n");
      }
      else if (buffer[0] == 'm')
      {
        // Syntax errors
        if (buffer[2] == '0')
        {
          switch (buffer[3])
          {
          case '0':
            printf(RED "(no piece is selected)\n" RESET);
            break;
          case '1':
            printf(RED "(can not move piece of your opponent)\n" RESET);
            break;
          case '2':
            printf(RED "(can not eat piece of yours)\n" RESET);
            break;
          case '3':
            printf(RED "(you has castled or king/rook has moved)\n" RESET);
            break;
          case '4':
            printf(RED "(spaces between king and rook are not clear)\n" RESET);
            break;
          }
        }
        else if (buffer[2] == '1')
        {
          switch (buffer[3])
          {
          case '0':
            printf(RED "(invalid move of king)\n" RESET);
            break;
          }
        }
        else if (buffer[2] == '2')
        {
          switch (buffer[3])
          {
          case '0':
            printf(RED "(invalid move of queen)\n" RESET);
            break;
          case '1':
            printf(RED "(rect is not clear)\n" RESET);
            break;
          case '2':
            printf(RED "(diagonal is not clear)\n" RESET);
            break;
          }
        }
        else if (buffer[2] == '3')
        {
          switch (buffer[3])
          {
          case '0':
            printf(RED "(invalid move of rook)\n" RESET);
            break;
          case '1':
            printf(RED "(rect is not clear)\n" RESET);
            break;
          }
        }
        else if (buffer[2] == '4')
        {
          switch (buffer[3])
          {
          case '0':
            printf(RED "(invalid move of bishop)\n" RESET);
            break;
          case '1':
            printf(RED "(diagonal is not clear)\n" RESET);
            break;
          }
        }
        else if (buffer[2] == '5')
        {
          switch (buffer[3])
          {
          case '0':
            printf(RED "(invalid move of knight)\n" RESET);
            break;
          }
        }
        else if (buffer[2] == '6')
        {
          switch (buffer[3])
          {
          case '0':
            printf(RED "(invalid move of pawn)\n" RESET);
            break;
          }
        }

        printf("\n");
      }
      // Check if it's an informative or error message
    }
    else
    {
      // Print the board
      system("clear");

      // printf("%s\n", buffer);
      
      if (*player == 1)
      {
        print_board_buff(buffer);
      }
      else
      {
        print_board_buff_inverted(buffer);
      }
    }

    bzero(buffer, 64);
  }

  // pthread_exit(NULL);
}
void CreateRoom(int sockfd){
  char buffer[10];
  recv(sockfd,buffer,9,0);
  printf("Create New Room\nRoom ID: %s\n",buffer);
  printf("Waiting For Player ... \n");
  playGame(sockfd);
}
void JoinRoom(int sockfd){
  char buffer[2048];
  recv(sockfd,buffer,2048,0);
  if (buffer[0] == 'n'){
    printf("There are no room ready!\n");
    printf("Automatically create new room\n");
    CreateRoom(sockfd);
    return;
  }
  printf("Room List:\n%s",buffer);
  int roomID;
  scanf("%d",&roomID);
  sprintf(buffer,"%d",roomID);
  send(sockfd,buffer,9,0);
  printf("Start Playing \n");
  playGame(sockfd);
}
void JoinOrCreateRoom(int sockfd){
  int res =0;
  while (res!=1 && res !=2){
    printf("1. Join Room\n");
    printf("2. Create Room\n");
    scanf("%d",&res);
  }
  char c[2];
  sprintf(c,"%d",res);
  send(sockfd,c,1,0);
  if (res == 1) JoinRoom(sockfd);
  else CreateRoom(sockfd);
}
void Lobby(int sockfd){
  printf("Wait for Data\n");
  int readsize;
  char buffer[2048];
  readsize = recv(sockfd, buffer,2048,0);
  if (readsize < 0){
    printf("[-] Disconnect\n");
    exit(0);
  }
  user data = convertStringtoData(buffer);
  printf("UserName: %s\n",data.name);
  printf("Rank: %s\n",rankNames[getRank(data.elo)]);
  printf("Elo : %d\n",data.elo);  
  JoinOrCreateRoom(sockfd);
}

void playGame(int sockfd){
  //
  isGameOver = 0;
  pthread_t tid[1];
  pthread_create(&tid[0], NULL, &on_signal, &sockfd);
  char buffer[64];
  int n;
   while (1) {
    if (isGameOver == 1){
      //printf("END GAME!\n");
      break;
     } 
     bzero(buffer, 64);
     fgets(buffer, 64, stdin);
     /* Send message to the server */
     if (isGameOver == 0 && buffer[0] != '\n' && buffer[0] != ' '){
      n = write(sockfd, buffer, strlen(buffer));
     if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
     }
     }
   }
   //printf("END GAME!\n");
   Lobby(sockfd);
}
int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent * server;

   //setlocale(LC_ALL, "en_US.UTF-8");
   char buffer[64];

   if (argv[2] == NULL) {
     portno = 80;
   } else {
     portno = atoi(argv[2]);
   }

   printf("Connecting to %s:%d\n", argv[1], portno);

   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   server = gethostbyname(argv[1]);

   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }

   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);

   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }

   /* Now ask for a message from the user, this message
      * will be read by server
   */


   // Response thread
   while (1){
       user* data = loginOrRegister();
       char* str = convertDatatoString(*data);
       send(sockfd,str,2048,0);
       char buffer[2048];
       recv(sockfd,buffer,1,0);
       if (buffer[0] == '0') printf("NOT OK!\n");
       else {
          break;
       }
  }
   Lobby(sockfd); 
   // Response thread
   return 0;
}
