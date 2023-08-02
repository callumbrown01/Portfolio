#define  _GNU_SOURCE

// the maximum length of the score and player fields (player is 9 but requires a divider, score is 10)
#define FIELD_SIZE_   10
// maximum line length in the score file
#define REC_SIZE_     (FIELD_SIZE_*2 + 1)

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h> // for setuid() and similar
#include <string.h>
#include <sys/types.h> // for off_t
#include <sys/stat.h>

// a struct for passing a record between functions 
struct player_score_record {
    char name[FIELD_SIZE_]; 
    char score[FIELD_SIZE_]; // string of length FIELD_SIZE_
    int line_number; // line number of the player's record in the file (at last line + 1 if not in the file)
};

// receives the the player record from get_score() along with the file path of the scores file.
// opens the file, opens a new temporary file, and copies each line across until it reaches the
// line_number in the player record struct, where it will instead copy in the record instead of the
// line from the original file. If the while loop exits, the player must not exist in the file and
// so their record is written in to the last line of the temp file, which then replaces the original file.
int update_record(const char * filename, struct player_score_record cpr) {

    // opening a second temp file which will replace the score file
    FILE * fdptr;
    FILE * ftmp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int count = 0;
    int newentry = 1;

    //  Open all required files 
    fdptr  = fopen(filename, "rwb");
    // create a new temporary file 
    ftmp = fopen("replace.tmp", "wb");

    // error handling the failure of opening either files
    if (fdptr == NULL || ftmp == NULL)
    {
        fprintf(stderr, "Unable to open the file at %s, please ensure the file exists, and you have the required privileges\n", filename);
        return 0;
    }

    // final line record for the player of length REC_SIZE_
    // copying the name and score into a singular string for writing into the file.
    char record[REC_SIZE_];
    memset(record, '\0', REC_SIZE_);
    memcpy(record, cpr.name, FIELD_SIZE_);
    memcpy(record + FIELD_SIZE_, cpr.score, FIELD_SIZE_);
    record[REC_SIZE_-1] = '\n';


    // Read line from source file and write to destination
    // file after replacing given line.
    count = 0;
    while ((read = getline(&line, &len, fdptr)) != -1)
    {
        // line count incremented
        count++;

        /* If current line is line to replace */
        if (count == cpr.line_number) {
            // if the line in the temp file is equal to the line in the original file, write the premade record into the file here.
            fwrite(record, sizeof(char), sizeof(record), ftmp);
            newentry = 0;
        } else {
            // copying the line across keeping the null characters in the correct position
            char line_player[FIELD_SIZE_];
            strncpy(line_player, line, FIELD_SIZE_);

            char line_score[FIELD_SIZE_];
            strncpy(line_score, line+FIELD_SIZE_, FIELD_SIZE_);
            
            char line_full[REC_SIZE_];
            memset(line_full, '\0', REC_SIZE_);
            memcpy(line_full, line_player, FIELD_SIZE_);
            memcpy(line_full+FIELD_SIZE_, line_score, FIELD_SIZE_);
            line_full[REC_SIZE_-1] = '\n'; 
            
            // write the constructed line into the file
            fwrite(line_full, sizeof(char), sizeof(line_full), ftmp);
        }
    }

    // if the line_number is not reached by the while loop above, the record is a new entry.
    // write this entry on the final line + 1 of the temp file
    if (newentry == 1) {
        fwrite(record, sizeof(char), sizeof(record), ftmp);
    }

    // close opened files
    fclose(fdptr);
    fclose(ftmp);

    // delete the original file for replacement
    remove(filename);

    // rename the temporary file to replace the original
    rename("replace.tmp", filename);

    // return successful value
    return 1;
}

// locates the score corresponding to the player_name in the scores file, 
// if there isn't one, the score becomes score_to_add ready to be added to the end of the file.
// sends the player_name, score and line_number of the existing record in a struct to update_score() for writing to the file.
int get_score(const char * filename, const char * player_name, int score_to_add) {

    // reading in binary mode for the null characters
    FILE * fdptr = fopen(filename, "rb");

    // line and line buffer for reading
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int fscore = score_to_add;

    // starting at line 1 not 0.
    int line_number = 1;

    // cpr - 'current player record'
    struct player_score_record cpr;

    // set the player and score strings to contain null characters for their field length
    memset(cpr.name, '\0', FIELD_SIZE_);
    memset(cpr.score, '\0', FIELD_SIZE_);

    // copy the player name into the struct, such that if the string is shorter than FIELD_SIZE_, the remaining characters are still null characters.
    strcpy(cpr.name, player_name);

    // if the pointer doesnt exist, there has been an error opening the file.
    if (fdptr == NULL) {
        fprintf(stderr, "Unable to open the file at %s, ensure the file exists and you have the required permissions\n", filename);
        return 0;
    }

    // reading through the file line by line until there are no more characters
    while ((read = getline(&line, &len, fdptr)) != -1) {

        // if the player name matches the first field of the line
        if (strncmp(line, player_name, strlen(player_name)) == 0) {

            // copy from the start of the score field on the line into the score_record struct
            // adds the score_to_add to the recorded score.
            fscore += atoi(line+FIELD_SIZE_);
            // writes integer value as string into struct score value.
            sprintf(cpr.score, "%d", fscore);

            // we've found the line so we terminate the loop.
            break;
        }
        line_number++;
    }
    // if the player is not in the file
    if (fscore == score_to_add) {
        sprintf(cpr.score, "%d", fscore);
        printf("player_name: %s, score: %d\n", player_name, fscore);
    }

    // assign the line number of the player in the file to the struct
    cpr.line_number = line_number;

    // close the file
    fclose(fdptr);

    // if we made it through without an error, the next function may return an error
    return update_record(filename, cpr);
}

// supplied function with arguments, performs error handling on input, sets the UID to the supplied id,
// calls get score using the supplied player_name and score_to_add
int adjust_score(uid_t uid, const char * player_name, int score_to_add, char **message) {

    // player_name must be at most nine characters long to leave room for a dividing null character.
    if (sizeof(player_name) > FIELD_SIZE_-1) {
        fprintf(stderr, "Supplied 'player_name' is too long, ensure it is a valid string between 1 and 9 characters long\n");
        return 0;
    }
    // score_to_add is an integer and so must be smaller than INT_MAX 
    // prevent integer overflow with input
    // assumption made here that scores may be negative integers, as negative scores exist in the test files.
    if (score_to_add > 2147483647) {
        fprintf(stderr, "Supplied 'score_to_add' is too large, the maximum score is '2147483647'.\n");
        return 0;
    } else if (score_to_add == 0 ) {
        fprintf(stderr, "Supplied 'score_to_add' should not be zero");
        return 0;
    }

    // sets the effective user id as the uid provided.
    if (setuid(uid) == -1) {
        fprintf(stderr, "An error occurred with setUID updating the User ID: %s\n", strerror(errno));
        return 0;
    }
    
    // setting the filename to the provided path in the skeleton code
    char *filename = "/var/lib/curdle/scores";

    // find the start of the player's sc    ore line in the file
    int returnval = get_score(filename, player_name, score_to_add);
    
    // if an error has occured updating the file outside of the adjust_score() function.
    if (returnval != 1) {

        // defining a pointer to a char array to set message to
        char *message_ptr;
        // manual memory allocation with malloc
        message_ptr = (char *) malloc(1000);
        strcpy(message_ptr, "An error occured while updating/adding the player's record in the score file");

        // if the memory allocation fails
        if (message_ptr == NULL) {
            fprintf(stderr, "Memory allocation failed for error message\n");
            // no return or exit here as the code may still have succesfully updated/added the score record to the file
        }

        // copy the message into the allocated buffer
        message = &message_ptr;
        // print the error message to the 'error stream' 
        fprintf(stderr, "%s\n", *message);
    }

    // return User and Group IDs to their real values, with error handling
    // returns the effective user id to the real uid.
    if (setuid(getuid()) == -1) {
        fprintf(stderr, "Error returning UID to Real UID: %s\n", strerror(errno));
    }
    // sets the effective group id to the real gid
    if (setgid(getgid()) == -1) {
        fprintf(stderr, "Error returning GID to Real GID: %s\n", strerror(errno));
    }

    return returnval;
}

// test main function for calling adjust_score() with varying arguments
int main() {

    // defining some test parameters
    int uid = 0;
    const char *player_name = "QKL9";
    int score_to_add = 100;
    char **message = NULL;

    // calling function
    adjust_score(uid, player_name, score_to_add, message);
}

