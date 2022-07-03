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
void saveData(user newUser){
    FILE* f = fopen("userdata.txt","a");
    fprintf(f,"%s\n",convertDatatoString(newUser));
    fclose(f);
}
int addNewUser(user newUser){
    for (int i=0;i<numUsers;++i)
     if (strcmp(newUser.username,Data[i].username)==0) return 0;
    Data[numUsers++] = newUser;
    saveData(newUser);
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



