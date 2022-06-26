#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#define MAX_LENGTH_USER_NAME 20
#define MIN_LENGTH_USER_NAME 6
#define MAX_LENGTH_PASS_WORD 16
#define MIN_LENGTH_PASS_WORD 6
#define MAX_LENGTH_NAME 30
//-----------RANK----------------//
#define RANK_NOVICE 0
#define RANK_BEGINNER 800
#define RANK_INTERMEDIATE 1100
#define RANK_INTERMEDIATE_2 1400
#define RANK_ADVANCED 1700
#define RANK_EXPERT 2000
//-------------RANK ARRAY------------------
int ranks[] = {RANK_NOVICE,RANK_BEGINNER,RANK_INTERMEDIATE,RANK_INTERMEDIATE_2,RANK_ADVANCED,RANK_EXPERT};
char* rankNames[]= {"NOVICE","BEGINNER","INTERMEDIATE","INTERMEDIATE 2","ADVANCED","EXPERT"};
typedef struct User{
  int elo;
  char username[MAX_LENGTH_USER_NAME];
  char password[MAX_LENGTH_PASS_WORD];
  char name[MAX_LENGTH_NAME];
  int socket;
} user;
//typedef struct room{
//  int player1ID;
//  int player2ID;
//  int isExist;
//  char infor[1024];
//} room;

int getRank(int elo){
  int i;
  for (i=5;i>=0;i--)
    if (elo>=ranks[i])
    return i;
}

int chooseInitRank(){
  while (1){
    printf("CHOOSE YOUR BEGIN RANK (1-6)\n");
    printf("1. NOVICE\n");
    printf("2. BEGINNER\n");
    printf("3. INTERMEDIATE\n");
    printf("4. INTERMEDIATE 2\n");
    printf("5. ADVANCED\n");
    printf("6. EXPERT\n");
    int choice;
    scanf("%d",&choice);
    if (1<=choice && choice <=6){
      return choice-1;
    } else {
      printf("Note: Please enter a number between 1 and 6!\n");
    }
  }
}
int checkUserNameValid(char* username){
  if (strlen(username)<MIN_LENGTH_USER_NAME || strlen(username)>MAX_LENGTH_USER_NAME) return 0;
  for (int i =0;i<strlen(username);++i){
    char c= *(username+i);
    if (i == 0 && isalpha(c) == 0) return 0;
    if (isdigit(c) == 0 && isalpha(c)==0) return 0;
  }
  return 1;
}
int checkPassWord(char* password){
  if (strlen(password) < MIN_LENGTH_PASS_WORD || strlen(password)>MAX_LENGTH_PASS_WORD) return 0;
  for (int i =0;i<strlen(password);++i){
    char c= *(password+i);
    if (isprint(c) == 0) return 0;
  }
  return 1;
}
user* Login(){
  printf("Username: ");
  fflush(stdin);
  char username[2*MAX_LENGTH_USER_NAME];
  scanf("%*c%[^\n]s",username);
  fflush(stdin);
  char pass[2*MAX_LENGTH_PASS_WORD];
  strcpy(pass,getpass("Password: "));
  if (checkUserNameValid(username)==0 || checkPassWord(pass) == 0){
    printf("Error: Username or Password is not valid!\n");
    return NULL;
  }
  user* logindata = (user*) malloc(sizeof(user));
  strcpy(logindata->username,username);
  strcpy(logindata->password,pass);
  logindata->elo = -1;
  
  return logindata;
}
user* Register(){
  char checkpass[2*MAX_LENGTH_PASS_WORD];
  user* newUserData = Login();
  if (newUserData == NULL) return NULL;
  else {
    strcpy(checkpass, getpass("Confirm Password: "));
    if (strcmp(newUserData->password,checkpass) == 0) {
    printf("Name: ");
    scanf("%*c%[^\n]s",newUserData->name);
    newUserData->elo = ranks[chooseInitRank()];
    return newUserData;
    } 
    else {
      printf("Error: Password and Confirm Password are not similar!");
      free(newUserData);
      return NULL;
    }
  }
}
user* loginOrRegister(){
  while (1){
    printf("1. Login\n");
    printf("2. Register\n");
    int choice;
    user* userData;
    scanf("%d",&choice);
    switch (choice) {
      case 1: userData = Login();
      if (userData != NULL)
        return userData;
      break;
      case 2: userData = Register();
      if (userData != NULL)
        return userData;
      break;
    }
  }
}
char* convertDatatoString(user userData){
    char* res = (char*) malloc(2048);
    sprintf(res,"%s %s %d %s",userData.username,userData.password,userData.elo,userData.name);
    return res;
}
user convertStringtoData(char* str){
    user newData;
    sscanf(str,"%s%s%d%*c%[^\n]s",newData.username,newData.password,&newData.elo,newData.name);
    return newData;
}