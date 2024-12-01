#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <locale.h>
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


char ask_int_no_buffer(int min,int max) {
#ifdef _WIN32
    char Inpt;
    int IntInpt;

    do{
        Inpt=getch();  // Windows: Use getch() from conio.h
        char str[2] = {Inpt, '\0'}; 
        IntInpt=atoi(str);
        if (Inpt=='\033'){
            return Inpt;
        }else if(IntInpt==0 || (IntInpt<min || IntInpt>max)){
            printf("Incorrect key pressed!\n");
            IntInpt=0;
        }
    }while(IntInpt==0);
    return Inpt;
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
    char DropTablesIds[20][200];    //20 locations, 200 char per location
}NPC;

typedef struct {
    char Name[60];
    char Notes[20][200];    //20 notes, 200 char per note
}ITEM;


typedef struct {
    char Name[50];
    char ItemIds[20][200];    //20 notes, 200 char per note
    char ItemPercs[20][200]; //20 percentages, 1 for each item id
}LOOT;

//------------------------------------------------------------------------------------Useful functions

void serialize_notes(PG *pg, char *result, size_t size) {
    result[0] = '\0'; // Clear the result string
    for (int i = 0; i < 20; i++) {
        if (strlen(pg->Notes[i]) > 0) { // Only add non-empty notes
            // Check for available space
            if (strlen(result) + strlen(pg->Notes[i]) + 2 >= size) {
                fprintf(stderr, "Buffer overflow risk in serialize_notes\n");
                break; // Exit to avoid overflow
            }

            // Concatenate the note
            strncat(result, pg->Notes[i], size - strlen(result) - 1);

            // Add delimiter if not the last note
            if (i < 19 && strlen(pg->Notes[i + 1]) > 0) {
                strncat(result, "|", size - strlen(result) - 1);
            }
        }
    }
}


void serialize_notes_npc(NPC *pg, char *result, size_t size) {
    result[0] = '\0'; // Clear the result string
    for (int i = 0; i < 20; i++) {
        if (strlen(pg->Notes[i]) > 0) { // Only add non-empty notes
            // Check for available space
            if (strlen(result) + strlen(pg->Notes[i]) + 2 >= size) {
                fprintf(stderr, "Buffer overflow risk in serialize_notes\n");
                break; // Exit to avoid overflow
            }

            // Concatenate the note
            strncat(result, pg->Notes[i], size - strlen(result) - 1);

            // Add delimiter if not the last note
            if (i < 19 && strlen(pg->Notes[i + 1]) > 0) {
                strncat(result, "|", size - strlen(result) - 1);
            }
        }
    }
}


void serialize_notes_item(ITEM *pg, char *result, size_t size) {
    result[0] = '\0'; // Clear the result string
    for (int i = 0; i < 20; i++) {
        if (strlen(pg->Notes[i]) > 0) { // Only add non-empty notes
            // Check for available space
            if (strlen(result) + strlen(pg->Notes[i]) + 2 >= size) {
                fprintf(stderr, "Buffer overflow risk in serialize_notes\n");
                break; // Exit to avoid overflow
            }

            // Concatenate the note
            strncat(result, pg->Notes[i], size - strlen(result) - 1);

            // Add delimiter if not the last note
            if (i < 19 && strlen(pg->Notes[i + 1]) > 0) {
                strncat(result, "|", size - strlen(result) - 1);
            }
        }
    }
}

void serialize_locations(NPC *npc, char *result, size_t size) {
    result[0] = '\0'; // Clear the result string
    for (int i = 0; i < 20; i++) {
        if (strlen(npc->Locations[i]) > 0) { // Only add non-empty notes
            // Check for available space
            if (strlen(result) + strlen(npc->Locations[i]) + 2 >= size) {
                fprintf(stderr, "Buffer overflow risk in serialize_notes\n");
                break; // Exit to avoid overflow
            }

            // Concatenate the note
            strncat(result, npc->Locations[i], size - strlen(result) - 1);

            // Add delimiter if not the last note
            if (i < 19 && strlen(npc->Locations[i+1]) > 0) {
                strncat(result, "|", size - strlen(result) - 1);
            }
        }
    }
}

void serialize_drop_tables(NPC *npc, char *result, size_t size) {
    result[0] = '\0'; // Clear the result string
    for (int i = 0; i < 20; i++) {
        if (strlen(npc->DropTablesIds[i]) > 0) { // Only add non-empty notes
            // Check for available space
            if (strlen(result) + strlen(npc->DropTablesIds[i]) + 2 >= size) {
                fprintf(stderr, "Buffer overflow risk in serialize_notes\n");
                break; // Exit to avoid overflow
            }

            // Concatenate the note
            strncat(result, npc->DropTablesIds[i], size - strlen(result) - 1);

            // Add delimiter if not the last note
            if (i < 19 && strlen(npc->DropTablesIds[i+1]) > 0) {
                strncat(result, "|", size - strlen(result) - 1);
            }
        }
    }
}

void serialize_item_ids(LOOT *drop, char *result, size_t size) {
    result[0] = '\0'; // Clear the result string
    for (int i = 0; i < 20; i++) {
        if (strlen(drop->ItemIds[i]) > 0) { // Only add non-empty notes
            // Check for available space
            if (strlen(result) + strlen(drop->ItemIds[i]) + 2 >= size) {
                fprintf(stderr, "Buffer overflow risk in serialize_notes\n");
                break; // Exit to avoid overflow
            }

            // Concatenate the note
            strncat(result, drop->ItemIds[i], size - strlen(result) - 1);

            // Add delimiter if not the last note
            if (i < 19 && strlen(drop->ItemIds[i+1]) > 0) {
                strncat(result, "|", size - strlen(result) - 1);
            }
        }
    }
}

void serialize_drop_percentages(LOOT *drop, char *result, size_t size) {
    result[0] = '\0'; // Clear the result string
    for (int i = 0; i < 20; i++) {
        if (strlen(drop->ItemPercs[i]) > 0) { // Only add non-empty notes
            // Check for available space
            if (strlen(result) + strlen(drop->ItemPercs[i]) + 2 >= size) {
                fprintf(stderr, "Buffer overflow risk in serialize_notes\n");
                break; // Exit to avoid overflow
            }

            // Concatenate the note
            strncat(result, drop->ItemPercs[i], size - strlen(result) - 1);

            // Add delimiter if not the last note
            if (i < 19 && strlen(drop->ItemPercs[i+1]) > 0) {
                strncat(result, "|", size - strlen(result) - 1);
            }
        }
    }
}

void print_spacer(){
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}



void clear_console() {
    // Use ANSI escape sequences to clear the screen
    printf("\033[2J\033[H");
    fflush(stdout); // Ensure the output is immediately flushed
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
        printf("PG data inserted successfully.\n\n");
    }

    // Finalize and return
    sqlite3_finalize(stmt);
    return rc;
}


int insert_NPC(sqlite3 *db, NPC *npc) {
    const char *sql_insert = "INSERT INTO NPCs (Name, CurrentHp, MaxHp, Notes, locations, drop_tables_ids) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc;

    // Serialize Notes into a single string
    char notes_serialized[4096];
    serialize_notes_npc(npc, notes_serialized, sizeof(notes_serialized));

    char locations_serialized[4096];
    serialize_locations(npc, locations_serialized, sizeof(notes_serialized));

    char drop_tables_ids_serialized[4096];
    serialize_drop_tables(npc, drop_tables_ids_serialized, sizeof(notes_serialized));

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, npc->Name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, npc->CurrentHp);
    sqlite3_bind_int(stmt, 3, npc->MaxHp);
    sqlite3_bind_text(stmt, 4, notes_serialized, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, locations_serialized, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, drop_tables_ids_serialized, -1, SQLITE_STATIC);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("NPC data inserted successfully.\n\n");
    }

    // Finalize and return
    sqlite3_finalize(stmt);
    return rc;
}


int insert_item(sqlite3 *db, ITEM *pg) {
    const char *sql_insert = "INSERT INTO Items (Name, Notes) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    int rc;

    // Serialize Notes into a single string
    char notes_serialized[4096];
    serialize_notes_item(pg, notes_serialized, sizeof(notes_serialized));

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, pg->Name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, notes_serialized, -1, SQLITE_STATIC);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("PG data inserted successfully.\n\n");
    }

    // Finalize and return
    sqlite3_finalize(stmt);
    return rc;
}


int insert_drop_table(sqlite3 *db, LOOT *pg) {
    const char *sql_insert = "INSERT INTO DropTables (Name, ItemIds, ItemPercs) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc;

    // Serialize Notes into a single string
    char item_ids_serialized[4096];
    char item_percs_serialized[4096];
    serialize_item_ids(pg, item_ids_serialized, sizeof(item_ids_serialized));
    serialize_drop_percentages(pg, item_percs_serialized, sizeof(item_percs_serialized));
    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, pg->Name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, item_ids_serialized, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, item_percs_serialized, -1, SQLITE_STATIC);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("PG data inserted successfully.\n\n");
    }

    // Finalize and return
    sqlite3_finalize(stmt);
    return rc;
}


int retrieve_party(sqlite3 *db) {
    const char *sql_select = "SELECT * FROM Party;";
    sqlite3_stmt *stmt;
    int rc;
    int isEmpty=1;
    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Execute the statement and loop through rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        isEmpty=0;
        // Retrieve column values for each row
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        int current_hp = sqlite3_column_int(stmt, 2);
        int max_hp = sqlite3_column_int(stmt, 3);
        const char *notes = (const char*)sqlite3_column_text(stmt, 4); // Notes are stored as a serialized string

        // Print the row data
        printf("%d. ", id);
        printf("%s ", name);
        printf("HP: %d", current_hp);
        printf("/%d\n", max_hp);
        printf("Notes: %s\n\n", notes);  // You can later split/deserialize the notes string if necessary
    }

    //Check epmty tables
    if (isEmpty==1){
        printf("No data registered!\n");
    }

    // Finalize the statement
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rc;
}


int retrieve_npcs(sqlite3 *db) {
    const char *sql_select = "SELECT * FROM NPCs;";
    sqlite3_stmt *stmt;
    int rc;
    int isEmpty=1;
    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Execute the statement and loop through rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        isEmpty=0;
        // Retrieve column values for each row
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        int current_hp = sqlite3_column_int(stmt, 2);
        int max_hp = sqlite3_column_int(stmt, 3);
        const char *notes = (const char*)sqlite3_column_text(stmt, 4); // Notes are stored as a serialized string
        const char *locations = (const char*)sqlite3_column_text(stmt, 6); //stored as a serialized string
        const char *drop_tables_ids = (const char*)sqlite3_column_text(stmt, 8); //stored as a serialized string
        // Print the row data
        printf("%d. ", id);
        printf("%s ", name);
        printf("HP: %d", current_hp);
        printf("/%d\n", max_hp);
        printf("Notes: %s\n", notes);  // You can later split/deserialize the notes string if necessary
        printf("Locations: %s\n", locations);  // You can later split/deserialize the notes string if necessary
        printf("DropTables: %s\n\n", drop_tables_ids);  // You can later split/deserialize the notes string if necessary
    }

    if (isEmpty==1){
        printf("No data registered!\n");
    }

    // Finalize the statement
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rc;
}


int retrieve_items(sqlite3 *db) {
    const char *sql_select = "SELECT * FROM Items;";
    sqlite3_stmt *stmt;
    int rc;
    int isEmpty=1;
    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Execute the statement and loop through rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        isEmpty=0;
        // Retrieve column values for each row
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        const char *notes = (const char*)sqlite3_column_text(stmt, 2); // Notes are stored as a serialized string

        // Print the row data
        printf("%d. ", id);
        printf("%s\n", name);
        printf("Notes: %s\n\n", notes);  // You can later split/deserialize the notes string if necessary
    }


    if (isEmpty==1){
        printf("No data registered!\n");
    }
    // Finalize the statement
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rc;
}


int retrieve_drop_tables(sqlite3 *db) {
    const char *sql_select = "SELECT * FROM DropTables;";
    sqlite3_stmt *stmt;
    int rc;
    int isEmpty=1;
    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Execute the statement and loop through rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        isEmpty=0;
        // Retrieve column values for each row
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        const char *notes = (const char*)sqlite3_column_text(stmt, 2); // Notes are stored as a serialized string
        const char *percentages = (const char*)sqlite3_column_text(stmt, 3);

        // Print the row data
        printf("%d. ", id);
        printf("%s\n", name);
        printf("Items: %s\n", notes);  // You can later split/deserialize the notes string if necessary
        printf("Percentages: %s\n\n", percentages);
    }


    if (isEmpty==1){
        printf("No data registered!\n");
    }
    // Finalize the statement
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rc;
}


int *retrieve_party_ids(sqlite3 *db, int *id_count) {
    const char *sql_select = "SELECT id, name FROM Party;"; // Query to get IDs and Names
    sqlite3_stmt *stmt;
    int rc;

    // Temporary array for IDs (initial size 10, resized dynamically if needed)
    int *ids = malloc(10 * sizeof(int));
    if (ids == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    int capacity = 10;
    *id_count = 0;

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        free(ids);
        return NULL;
    }

    // Execute the statement and process rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Retrieve the ID column
        int id = sqlite3_column_int(stmt, 0); // ID column

        // Store ID in the array
        if (*id_count >= capacity) {
            // Resize array if capacity is exceeded
            capacity *= 2;
            int *new_ids = realloc(ids, capacity * sizeof(int));
            if (new_ids == NULL) {
                fprintf(stderr, "Memory reallocation failed\n");
                free(ids);
                sqlite3_finalize(stmt);
                return NULL;
            }
            ids = new_ids;
        }
        ids[(*id_count)++] = id;

        // Retrieve and print the Name column
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        printf("\n%d. ", id);
        printf("%s\n\n", name);
    }

    // Finalize the statement
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        free(ids);
        sqlite3_finalize(stmt);
        return NULL;
    }

    sqlite3_finalize(stmt);
    return ids;
}


int *retrieve_npc_ids(sqlite3 *db, int *id_count) {
    const char *sql_select = "SELECT id, name FROM NPCs;"; // Query to get IDs and Names
    sqlite3_stmt *stmt;
    int rc;

    // Temporary array for IDs (initial size 10, resized dynamically if needed)
    int *ids = malloc(10 * sizeof(int));
    if (ids == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    int capacity = 10;
    *id_count = 0;

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        free(ids);
        return NULL;
    }

    // Execute the statement and process rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Retrieve the ID column
        int id = sqlite3_column_int(stmt, 0); // ID column

        // Store ID in the array
        if (*id_count >= capacity) {
            // Resize array if capacity is exceeded
            capacity *= 2;
            int *new_ids = realloc(ids, capacity * sizeof(int));
            if (new_ids == NULL) {
                fprintf(stderr, "Memory reallocation failed\n");
                free(ids);
                sqlite3_finalize(stmt);
                return NULL;
            }
            ids = new_ids;
        }
        ids[(*id_count)++] = id;

        // Retrieve and print the Name column
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        printf("\n%d. ", id);
        printf("%s\n", name);
    }

    // Finalize the statement
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        free(ids);
        sqlite3_finalize(stmt);
        return NULL;
    }

    sqlite3_finalize(stmt);
    return ids;
}


int *retrieve_items_ids(sqlite3 *db, int *id_count) {
    const char *sql_select = "SELECT id, name FROM Items;"; // Query to get IDs and Names
    sqlite3_stmt *stmt;
    int rc;

    // Temporary array for IDs (initial size 10, resized dynamically if needed)
    int *ids = malloc(10 * sizeof(int));
    if (ids == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    int capacity = 10;
    *id_count = 0;

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        free(ids);
        return NULL;
    }

    // Execute the statement and process rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Retrieve the ID column
        int id = sqlite3_column_int(stmt, 0); // ID column

        // Store ID in the array
        if (*id_count >= capacity) {
            // Resize array if capacity is exceeded
            capacity *= 2;
            int *new_ids = realloc(ids, capacity * sizeof(int));
            if (new_ids == NULL) {
                fprintf(stderr, "Memory reallocation failed\n");
                free(ids);
                sqlite3_finalize(stmt);
                return NULL;
            }
            ids = new_ids;
        }
        ids[(*id_count)++] = id;

        // Retrieve and print the Name column
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        printf("\n%d. ", id);
        printf("%s\n", name);
    }

    // Finalize the statement
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        free(ids);
        sqlite3_finalize(stmt);
        return NULL;
    }

    sqlite3_finalize(stmt);
    return ids;
}


int *retrieve_DropTables_ids(sqlite3 *db, int *id_count) {
    const char *sql_select = "SELECT id, name FROM DropTables;"; // Query to get IDs and Names
    sqlite3_stmt *stmt;
    int rc;

    // Temporary array for IDs (initial size 10, resized dynamically if needed)
    int *ids = malloc(10 * sizeof(int));
    if (ids == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    int capacity = 10;
    *id_count = 0;

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        free(ids);
        return NULL;
    }

    // Execute the statement and process rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Retrieve the ID column
        int id = sqlite3_column_int(stmt, 0); // ID column

        // Store ID in the array
        if (*id_count >= capacity) {
            // Resize array if capacity is exceeded
            capacity *= 2;
            int *new_ids = realloc(ids, capacity * sizeof(int));
            if (new_ids == NULL) {
                fprintf(stderr, "Memory reallocation failed\n");
                free(ids);
                sqlite3_finalize(stmt);
                return NULL;
            }
            ids = new_ids;
        }
        ids[(*id_count)++] = id;

        // Retrieve and print the Name column
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        printf("\n%d. ", id);
        printf("%s\n", name);
    }

    // Finalize the statement
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        free(ids);
        sqlite3_finalize(stmt);
        return NULL;
    }

    sqlite3_finalize(stmt);
    return ids;
}

int delete_pg(sqlite3 *db, int id) {
    char *err_msg = NULL;
    int rc;
    char sql[256];

    // Prepare SQL DELETE statement
    snprintf(sql, sizeof(sql), "DELETE FROM Party WHERE id = %d;", id);

    // Execute the SQL statement
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    printf("\nDeleted successfully.\n");
    return SQLITE_OK;
}

int delete_npc(sqlite3 *db, int id) {
    char *err_msg = NULL;
    int rc;
    char sql[256];

    // Prepare SQL DELETE statement
    snprintf(sql, sizeof(sql), "DELETE FROM NPCs WHERE id = %d;", id);

    // Execute the SQL statement
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    printf("\nDeleted successfully.\n");
    return SQLITE_OK;
}


int delete_item(sqlite3 *db, int id) {
    char *err_msg = NULL;
    int rc;
    char sql[256];

    // Prepare SQL DELETE statement
    snprintf(sql, sizeof(sql), "DELETE FROM Items WHERE id = %d;", id);

    // Execute the SQL statement
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    printf("\nDeleted successfully.\n");
    return SQLITE_OK;
}

int delete_DropTable(sqlite3 *db, int id) {
    char *err_msg = NULL;
    int rc;
    char sql[256];

    // Prepare SQL DELETE statement
    snprintf(sql, sizeof(sql), "DELETE FROM DropTables WHERE id = %d;", id);

    // Execute the SQL statement
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    printf("\nDeleted successfully.\n");
    return SQLITE_OK;
}

//------------------------------------------------------------------------------------MAIN

int main(){

    setlocale(LC_ALL, ""); // Use system locale
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
        "id INTEGER PRIMARY KEY, "
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
        "id INTEGER PRIMARY KEY, "
        "name TEXT, "
        "currenthp INTEGER, "
        "maxhp INTEGER, "
        "notes TEXT, "
        "note_count INTEGER, "
        "locations TEXT, "
        "locations_count INTEGER, "
        "drop_tables_ids TEXT);";
    rc = sqlite3_exec(db, sql_create2, 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }


    // Creating Items table if not exists
    const char *sql_create3 = 
        "CREATE TABLE IF NOT EXISTS Items ("
        "id INTEGER PRIMARY KEY, "
        "name TEXT, "
        "notes TEXT, "
        "note_count INTEGER);";
    rc = sqlite3_exec(db, sql_create3, 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }


    const char *sql_create4 = 
        "CREATE TABLE IF NOT EXISTS DropTables ("
        "id INTEGER PRIMARY KEY, "
        "name TEXT, "
        "ItemIds TEXT, "
        "ItemPercs TEXT);";
    rc = sqlite3_exec(db, sql_create4, 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }


    int getcinp;
    int Quit=1;
    int QuitParty=1;
    int QuitNpcs=1;
    int QuitItems=1;
    int QuitDrops=1;

    while (Quit==1)
    {
        printf("\n1. Party\n2. NPCs\n3. Items\n4. Drop tables\nESC. quit\n\n");
        //1 = 49
        //2 = 50
        //3 = 51
        //ESC = 27
        getcinp=ask_int_no_buffer(1,4);
        clear_console();
        
        
        //Party menu
        if (getcinp==49){
            QuitParty=1;
            while (QuitParty==1)
            {
                printf("\n1. See party\n2. Edit party\nESC. back\n\n");
                //1 = 49
                //2 = 50
                //3 = 51 eccetera
                //ESC = 27
                getcinp=ask_int_no_buffer(1,2);
                clear_console();


                //View party
                if (getcinp==49){
                    retrieve_party(db);

                //Party edit
                } else if (getcinp==50){

                    //Party edit loop
                    do{
                        printf("\n1. Add party member\n2. Edit party member\n3. Delete party member\nESC. back\n\n");
                        //1 = 49
                        //2 = 50
                        //3 = 51
                        //ESC = 27
                        getcinp=ask_int_no_buffer(1,3);

                        //vars used by getinput
                        long num=0;
                        char UserInput[500];


                        //Adding party member
                        if (getcinp==49)
                            {
                            PG pg;
                            memset(pg.Notes, 0, sizeof(pg.Notes));
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
                                getcinp=ask_int_no_buffer(1,1);
                                if(getcinp==27){
                                    Esc=1;
                                };
                            }
                            clear_console();
                            insert_pg(db, &pg);
                            retrieve_party(db);

                        }//Update party member
                        else if (getcinp==50){
                        
                        //Delete party member
                        }else if (getcinp==51){
                            int idcount;
                            int *ids;
                            int Success=0;
                            long id;
                            int IsInIds=0;

                            //---------------------------------------------------Need a check to verify if the list is empty

                            ids=retrieve_party_ids(db,&idcount);
                            do{
                                GetUserInp("Select pg Id",UserInput,sizeof(UserInput),1,&Success,&id);
                                for (int k=0;k<sizeof(ids);k++){
                                    if (ids[k]==id){
                                        IsInIds=1;
                                        break;
                                    }
                                }
                                if (IsInIds==0){
                                    printf("Id not in list\n");
                                }
                            }while (Success==0 || IsInIds==0);  //Loop while theres an error in formatting or the id isnt into idsù
                            clear_console();
                            delete_pg(db,id);

                        }
                    
                    } while (getcinp!=27);//Quit party edit menu

                //Party menu quit
                } else if (getcinp==27){
                    QuitParty=0;
                }
            }//Loop party menu

        //NPC Menu
        }else if (getcinp==50){
            QuitNpcs=1;
            while (QuitNpcs==1)
            {
                printf("\n1. See NPCs\n2. Edit NPCs\nESC. back\n\n");
                //1 = 49
                //2 = 50
                //3 = 51 eccetera
                //ESC = 27
                getcinp=ask_int_no_buffer(1,2);
                clear_console();


                //View NPCS
                if (getcinp==49){
                    retrieve_npcs(db);

                //NPC edit
                } else if (getcinp==50){

                    //NPCS edit loop
                    do{
                        printf("\n1. Add NPC\n2. Edit NPC\n3. Delete NPC\nESC. back\n\n");
                        //1 = 49
                        //2 = 50
                        //3 = 51
                        //ESC = 27
                        getcinp=ask_int_no_buffer(1,3);
                        clear_console();

                        //vars used by getinput
                        long num=0;
                        char UserInput[500];


                        //Adding NPC member
                        if (getcinp==49)
                            {
                            NPC npc;
                            memset(npc.Notes, 0, sizeof(npc.Notes));
                            memset(npc.Locations, 0, sizeof(npc.Locations));
                            memset(npc.DropTablesIds, 0, sizeof(npc.DropTablesIds));
                            char Name[20];
                            long MaxHp;
                            long Hp;
                            char Notes[20][200];
                            int isValid=0;
                            

                            do {
                                GetUserInp("Name",Name,sizeof(Name),0,&isValid,&num);
                            } while (isValid==0);
                            strcpy(npc.Name,Name);


                            do {
                                GetUserInp("Current Hp",UserInput,sizeof(UserInput),1,&isValid,&Hp);
                            } while (isValid==0);
                            npc.CurrentHp=Hp;

                            do {
                                GetUserInp("Max Hp",UserInput,sizeof(UserInput),1,&isValid,&MaxHp);
                            } while (isValid==0);
                            npc.MaxHp=MaxHp;

                            //Notes
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
                                strcpy(npc.Notes[i],Notes[i]);
                                i++;
                                printf("\n1. Add note #%d\nESC. Complete NPC creation\n",(i+1));
                                getcinp=ask_int_no_buffer(1,1);
                                if(getcinp==27){
                                    Esc=1;
                                };
                            }

                            i=0;
                            Esc=0;
                            //Locations
                            while(i<20 && Esc==0){
                                do {
                                    char NoteNum[]="Location #n ";
                                    char NumChar= '0' + (i+1);
                                    NoteNum[10]=NumChar;

                                    GetUserInp(NoteNum,Notes[i],sizeof(Notes[i]),0,&isValid,&num);
                                } while (isValid==0);
                                strcpy(npc.Locations[i],Notes[i]);
                                i++;
                                printf("\n1. Add location #%d\nESC. Complete NPC creation\n",(i+1));
                                getcinp=ask_int_no_buffer(1,1);
                                if(getcinp==27){
                                    Esc=1;
                                };
                            }

                            i=0;
                            Esc=0;
                            //DroptableIds
                            while(i<20 && Esc==0){
                                do {
                                    char NoteNum[]="Drop Table Id #n ";
                                    char NumChar= '0' + (i+1);
                                    NoteNum[15]=NumChar;

                                    GetUserInp(NoteNum,Notes[i],sizeof(Notes[i]),0,&isValid,&num);
                                } while (isValid==0);
                                strcpy(npc.DropTablesIds[i],Notes[i]);
                                i++;
                                printf("\n1. Add drop table #%d\nESC. Complete NPC creation\n",(i+1));
                                getcinp=ask_int_no_buffer(1,1);
                                if(getcinp==27){
                                    Esc=1;
                                };
                            }


                            clear_console();
                            insert_NPC(db, &npc);
                            retrieve_npcs(db);

                        }//Update NPC
                        else if (getcinp==50){
                        
                        //Delete NPC
                        }else if (getcinp==51){
                            int idcount;
                            int *ids;
                            int Success=0;
                            long id;
                            int IsInIds=0;

                            //---------------------------------------------------Need a check to verify if the list is empty

                            ids=retrieve_npc_ids(db,&idcount);
                            do{
                                GetUserInp("Select NPC Id",UserInput,sizeof(UserInput),1,&Success,&id);
                                for (int k=0;k<sizeof(ids);k++){
                                    if (ids[k]==id){
                                        IsInIds=1;
                                        break;
                                    }
                                }
                                if (IsInIds==0){
                                    printf("Id not in list\n");
                                }
                            }while (Success==0 || IsInIds==0);  //Loop while theres an error in formatting or the id isnt into idsù
                            clear_console();
                            delete_npc(db,id);

                        }
                    
                    } while (getcinp!=27);//Quit NPC edit menu

                //NPC menu quit
                } else if (getcinp==27){
                    QuitNpcs=0;
                }
            }//Loop NPC menu

        //Items Menu
        }else if (getcinp==51){
            QuitItems=1;
            while (QuitItems==1)
            {
                printf("\n1. See Items\n2. Edit Items\nESC. back\n\n");
                //1 = 49
                //2 = 50
                //3 = 51 eccetera
                //ESC = 27
                getcinp=ask_int_no_buffer(1,2);
                clear_console();


                //View Item
                if (getcinp==49){
                    retrieve_items(db);

                //Item edit
                } else if (getcinp==50){

                    //Item edit loop
                    do{
                        printf("\n1. Add Item\n2. Edit Item\n3. Delete Item\nESC. back\n\n");
                        //1 = 49
                        //2 = 50
                        //3 = 51
                        //ESC = 27
                        getcinp=ask_int_no_buffer(1,3);
                        clear_console();

                        //vars used by getinput
                        long num=0;
                        char UserInput[500];


                        //Adding Item member
                        if (getcinp==49)
                            {
                            ITEM Item;
                            memset(Item.Notes, 0, sizeof(Item.Notes));
                            char Name[60];
                            long MaxHp;
                            long Hp;
                            char Notes[20][200];
                            int isValid=0;
                            

                            do {
                                GetUserInp("Name",Name,sizeof(Name),0,&isValid,&num);
                            } while (isValid==0);
                            strcpy(Item.Name,Name);

                            //Notes
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
                                strcpy(Item.Notes[i],Notes[i]);
                                i++;
                                printf("\n1. Add note #%d\nESC. Complete Item creation\n",(i+1));
                                getcinp=ask_int_no_buffer(1,1);
                                if(getcinp==27){
                                    Esc=1;
                                };
                            }

                            clear_console();
                            insert_item(db, &Item);
                            retrieve_items(db);

                        }//Update Item
                        else if (getcinp==50){
                        
                        //Delete Item
                        }else if (getcinp==51){
                            int idcount;
                            int *ids;
                            int Success=0;
                            long id;
                            int IsInIds=0;

                            //---------------------------------------------------Need a check to verify if the list is empty

                            ids=retrieve_items_ids(db,&idcount);
                            do{
                                GetUserInp("Select Item Id",UserInput,sizeof(UserInput),1,&Success,&id);
                                for (int k=0;k<sizeof(ids);k++){
                                    if (ids[k]==id){
                                        IsInIds=1;
                                        break;
                                    }
                                }
                                if (IsInIds==0){
                                    printf("Id not in list\n");
                                }
                            }while (Success==0 || IsInIds==0);  //Loop while theres an error in formatting or the id isnt into idsù
                            clear_console();
                            delete_item(db,id);

                        }
                    
                    } while (getcinp!=27);//Quit Item edit menu

                //Item menu quit
                } else if (getcinp==27){
                    QuitItems=0;
                }
            }//Loop Item menu

        //Drop tables Menu
        }else if (getcinp==52){
            QuitDrops=1;
            while (QuitDrops==1)
            {
                printf("\n1. See Drop Tables\n2. Edit Drop Tables\nESC. back\n\n");
                //1 = 49
                //2 = 50
                //3 = 51 eccetera
                //ESC = 27
                getcinp=ask_int_no_buffer(1,2);
                clear_console();


                //View Item
                if (getcinp==49){
                    retrieve_drop_tables(db);

                //Item edit
                } else if (getcinp==50){

                    //Item edit loop
                    do{
                        printf("\n1. Create a drop table\n2. Edit table\n3. Delete drop table\nESC. back\n\n");
                        //1 = 49
                        //2 = 50
                        //3 = 51
                        //ESC = 27
                        getcinp=ask_int_no_buffer(1,3);
                        clear_console();

                        //vars used by getinput
                        long num=0;
                        char UserInput[500];


                        //Creating Drop Table member
                        if (getcinp==49)
                            {
                            LOOT DropTable;
                            memset(DropTable.ItemIds, 0, sizeof(DropTable.ItemIds));
                            memset(DropTable.ItemPercs, 0, sizeof(DropTable.ItemPercs));
                            char Name[60];
                            char ItemIds[20][200];
                            char ItemPercs[20][200];
                            int isValid=0;
                            

                            do {
                                GetUserInp("Drop table name",Name,sizeof(Name),0,&isValid,&num);
                            } while (isValid==0);
                            strcpy(DropTable.Name,Name);

                            //Item Ids adding to drop table
                            int i=0;
                            int Esc=0;
                            int isValid2=0;
                            char numstring[3];
                            while(i<20 && Esc==0){
                                do {
                                    char ItemNum[]="Item #n id";
                                    char ItemPercN[]="Item #n drop percentage (0-100)";
                                    char NumChar= '0' + (i+1);
                                    ItemNum[6]=NumChar;
                                    ItemPercN[6]=NumChar;
                                    int idCount;
                                    int isInIds=0;
                                    int *ids;

                                    //ask item id until its in items ids
                                    do{

                                        ids=retrieve_items_ids(db,&idCount);
                                        GetUserInp(ItemNum,ItemIds[i],sizeof(ItemIds[i]),1,&isValid,&num);

                                        for (int k=0;k<sizeof(ids);k++){
                                            if (ids[k]==num){
                                                isInIds=1;
                                            }
                                        }

                                        if (isInIds==0){
                                            printf("Item Id not found in items list!\n");
                                            isValid=0;
                                        }
                                    }while(isInIds==0);


                                    GetUserInp(ItemPercN,ItemPercs[i],sizeof(ItemPercs[i]),1,&isValid2,&num);

                                    
                                } while (isValid==0 || isValid2==0);
                                strcpy(DropTable.ItemIds[i],ItemIds[i]);
                                strcpy(DropTable.ItemPercs[i],ItemPercs[i]);
                                i++;
                                printf("\n1. Add Item #%d to table\nESC. Complete drops creation\n",(i+1));
                                getcinp=ask_int_no_buffer(1,1);
                                if(getcinp==27){
                                    Esc=1;
                                };
                            }

                            clear_console();
                            insert_drop_table(db, &DropTable);
                            retrieve_drop_tables(db);

                        }//Update Drop Table
                        else if (getcinp==50){
                        
                        //Delete Drop Table
                        }else if (getcinp==51){
                            int idcount;
                            int *ids;
                            int Success=0;
                            long id;
                            int IsInIds=0;

                            //---------------------------------------------------Need a check to verify if the list is empty

                            ids=retrieve_DropTables_ids(db,&idcount);
                            do{
                                GetUserInp("Select Drop table Id",UserInput,sizeof(UserInput),1,&Success,&id);
                                for (int k=0;k<sizeof(ids);k++){
                                    if (ids[k]==id){
                                        IsInIds=1;
                                        break;
                                    }
                                }
                                if (IsInIds==0){
                                    printf("Id not in list\n");
                                }
                            }while (Success==0 || IsInIds==0);  //Loop while theres an error in formatting or the id isnt into idsù
                            clear_console();
                            delete_DropTable(db,id);

                        }
                    
                    } while (getcinp!=27);//Quit Item edit menu

                //Item menu quit
                } else if (getcinp==27){
                    QuitDrops=0;
                }
            }//Loop Item menu


        //Quit main menu
        }else if (getcinp==27){
            Quit=0;
        }
    }//Loop main menu

    // Close database
    sqlite3_close(db);  

}//Main func end