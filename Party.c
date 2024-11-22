#include <stdio.h>
#include <stdlib.h>

int main(){

    ///Variables
    const int IsPartyFile=0;
    const char *Filename = "Data folder/Party.txt";


    //Check file existence or create one
    FILE *file = fopen(Filename,"r");
    if (file==NULL){
        printf("Party file not found, creating one..\n");
        file=fopen(Filename,"w");
        if (file!=NULL){
        printf("File created successfully.");
        } else {
            printf("Error when creating file");
            exit(1);
        }
    //File found
    } else {
        printf("Party file found..");
    }







}