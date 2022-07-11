#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checkLogin.c"

user Data[1000000];
int numUsers;

void readData(){
    FILE* f = fopen("userdata.txt","r");
    numUsers = 0;
    while (!feof(f)){
        fscanf(f,"%s%s%d%*c%[^\n]s",Data[numUsers].username,Data[numUsers].password,&Data[numUsers].elo,Data[numUsers].name);
        numUsers ++;
    }
    fclose(f);
}
void appendData(user newUser){
    FILE* f = fopen("userdata.txt","a");
    if (numUsers > 0) fprintf(f,"\n");
    fprintf(f,"%s",convertDatatoString(newUser));
    fclose(f);
}
void saveData(){
    FILE* f = fopen("userdata.txt","w");
    for (int i=0;i<numUsers;++i){
        fprintf(f,"%s",convertDatatoString(Data[i]));
        if (i<numUsers-1) fprintf(f,"\n");
        //printf("%d-%s\n",i,convertDatatoString(Data[i]));
        //printf("%d - %s\n",i,convertDatatoString(Data[i]));
    }
    fclose(f);
}
void changeElo(user loginData){
    printf("NumUsers = %d\n",numUsers);

    for (int i=0;i<numUsers;++i){
        user userData = Data[i];
        if (strcmp(userData.username,loginData.username) == 0 && strcmp(userData.password,loginData.password) ==0)
            Data[i] = loginData;    
    }
    saveData();
}
int addNewUser(user newUser){
    for (int i=0;i<numUsers;++i)
     if (strcmp(newUser.username,Data[i].username)==0) return 0;
    Data[numUsers++] = newUser;
    appendData(newUser);
    return 1;
}
user getDataFromUserNameAndPassWord(user loginData){
    for (int i=0;i<numUsers;++i){
        user userData = Data[i];
        if (strcmp(userData.username,loginData.username) == 0 && strcmp(userData.password,loginData.password) ==0)
            return userData;
    }
    return loginData;
}



