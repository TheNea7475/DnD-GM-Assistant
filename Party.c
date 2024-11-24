#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include "sqlite3.h"

#ifdef _WIN32
#include <conio.h>  // For Windows
#else
#include <termios.h>  // For Linux/macOS
#include <unistd.h>
#endif

//------------------------------------------------------------------------------------Conditional compilation

char get_char_no_buffer() {
#ifdef _WIN32
    return getch();  // Windows: Use getch() from conio.h
#else
    struct termios oldt, newt;
    char ch;
    
    // Disable canonical mode and echo
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();  // Read one character

    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

//------------------------------------------------------------------------------------Structures declaration
typedef struct {
    char Name[50];
    int CurrentHp;
    int MaxHp;
    char Notes[20][200];    //20 notes, 200 char per note
}PG;

typedef struct {
    char Name[50];
    int CurrentHp;
    int MaxHp;
    char Notes[20][200];    //20 notes, 200 char per note
    char Locations[20][200];    //20 locations, 200 char per location
}NPC;


//------------------------------------------------------------------------------------Useful functions

void serialize_notes(PG *pg, char *result, size_t size) {
    result[0] = '\0'; // Clear the result string
    for (int i = 0; i < 20; i++) {
        if (strlen(pg->Notes[i]) > 0) { // Only add non-empty notes
            strncat(result, pg->Notes[i], size - strlen(result) - 1);
            if (i < 19 && strlen(pg->Notes[i + 1]) > 0) {
                strncat(result, "|", size - strlen(result) - 1);
            }
        }
    }
}

void serialize_locations(NPC *npc, char *result, size_t size) {
    result[0] = '\0'; // Clear the result string
    for (int i = 0; i < 20; i++) {
        if (strlen(npc->Locations[i]) > 0) { // Only add non-empty locations
            strncat(result, npc->Locations[i], size - strlen(result) - 1);
            if (i < 19 && strlen(npc->Locations[i + 1]) > 0) {
                strncat(result, "|", size - strlen(result) - 1);
            }
        }
    }
}


void GetUserInp(char *FieldName, char *txt, int size, int IsInt, int *Success, long *num){
    while (*FieldName != '\0'){
        putchar(*FieldName);
        FieldName++;
    }
    printf("> ");
    fgets(txt,size,stdin);
    char *ptr=txt;
    while (*ptr != '\0'){
        if (*ptr == '\n'){
            *ptr = '\0';
            break;
        }
        ptr++;
    }
    //check if string or not
    char *endptr;
    *num = strtol(txt,&endptr,10);
    if (*endptr == '\0' && IsInt==1){
        *Success=1;
        //If not string, the correct value is stored in num as integer 
    }else if (IsInt==0){
        *Success=1;
    }else {
        *Success=0;
        printf("Incorrect data\n");
    }

}


//------------------------------------------------------------------------------------Database functions

int insert_pg(sqlite3 *db, PG *pg) {
    const char *sql_insert = "INSERT INTO Party (Name, CurrentHp, MaxHp, Notes) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc;

    // Serialize Notes into a single string
    char notes_serialized[4096];
    serialize_notes(pg, notes_serialized, sizeof(notes_serialized));

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, pg->Name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, pg->CurrentHp);
    sqlite3_bind_int(stmt, 3, pg->MaxHp);
    sqlite3_bind_text(stmt, 4, notes_serialized, -1, SQLITE_STATIC);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("PG data inserted successfully.\n");
    }

    // Finalize and return
    sqlite3_finalize(stmt);
    return rc;
}




int retrieve_party(sqlite3 *db) {
    const char *sql_select = "SELECT * FROM Party;";
    sqlite3_stmt *stmt;
    int rc;

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Execute the statement and loop through rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Retrieve column values for each row
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        int current_hp = sqlite3_column_int(stmt, 2);
        int max_hp = sqlite3_column_int(stmt, 3);
        const char *notes = (const char*)sqlite3_column_text(stmt, 4); // Notes are stored as a serialized string

        // Print the row data
        printf("ID: %d\n", id);
        printf("Name: %s\n", name);
        printf("Current HP: %d\n", current_hp);
        printf("Max HP: %d\n", max_hp);
        printf("Notes: %s\n\n", notes);  // You can later split/deserialize the notes string if necessary
    }

    // Finalize the statement
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

//------------------------------------------------------------------------------------MAIN

int main(){

    //Db opening
    sqlite3 *db;
    char *err_msg = NULL;
    int rc;

    rc = sqlite3_open("Data folder/Database.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Creating Party & Npcs table if not exists
    const char *sql_create = 
        "CREATE TABLE IF NOT EXISTS Party ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT, "
        "currenthp INTEGER, "
        "maxhp INTEGER, "
        "notes TEXT, "
        "note_count INTEGER);";
    rc = sqlite3_exec(db, sql_create, 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }  

    // Creating NPCs table if not exists
    const char *sql_create2 = 
        "CREATE TABLE IF NOT EXISTS NPCs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT, "
        "currenthp INTEGER, "
        "maxhp INTEGER, "
        "notes TEXT, "
        "note_count INTEGER);";
    rc = sqlite3_exec(db, sql_create2, 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }


    int getcinp;
    int Quit=1;
    while (Quit==1)
    {
        printf("\n1. See party\n2. Edit party\nESC. quit\n\n");
        //1 = 49
        //2 = 50
        //3 = 51
        //ESC = 27
        getcinp=get_char_no_buffer();



        if (getcinp==49){
            retrieve_party(db);

        } else if (getcinp==50){
            retrieve_party(db);
            printf("\n1. Add party member\n2. Edit party member\n3. Delete party member\nESC. back\n\n");
            //1 = 49
            //2 = 50
            //3 = 51
            //ESC = 27
            getcinp=get_char_no_buffer();

            //Adding party member
            if (getcinp==49)
                {
                PG pg;
                long num=0;
                char UserInput[500];
                char Name[20];
                long MaxHp;
                long Hp;
                char Notes[20][200];
                int isValid=0;
                

                do {
                    GetUserInp("Name",Name,sizeof(Name),0,&isValid,&num);
                } while (isValid==0);
                strcpy(pg.Name,Name);


                do {
                    GetUserInp("Current Hp",UserInput,sizeof(UserInput),1,&isValid,&Hp);
                } while (isValid==0);
                pg.CurrentHp=Hp;

                do {
                    GetUserInp("Max Hp",UserInput,sizeof(UserInput),1,&isValid,&MaxHp);
                } while (isValid==0);
                pg.MaxHp=MaxHp;

                int i=0;
                int Esc=0;
                char numstring[3];
                while(i<20 && Esc==0){
                    do {
                        char NoteNum[]="Note #n ";
                        char NumChar= '0' + (i+1);
                        NoteNum[6]=NumChar;

                        GetUserInp(NoteNum,Notes[i],sizeof(Notes[i]),0,&isValid,&num);
                    } while (isValid==0);
                    strcpy(pg.Notes[i],Notes[i]);
                    i++;
                    printf("\n1. Add note #%d\nESC. Complete pg creation\n",(i+1));
                    getcinp=get_char_no_buffer();
                    if(getcinp==27){
                        Esc=1;
                    };
                }
                insert_pg(db, &pg);
                retrieve_party(db);

            }//Update party member
            else if (getcinp==50){

            //Delete party member
            }else if (getcinp==51){

            //Quit party edit menu
            }else if (getcinp==27){

            }

        //Outher menu quit
        } else if (getcinp==27){
            Quit=0;
        }
    }//Loop main menu

    // Close database
    sqlite3_close(db);  

}//Main func end