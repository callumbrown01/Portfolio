#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

int caesar_encrypt(signed char key, char * buf, size_t buf_len) {

    // temp array to store buf values
    char tempbuf[buf_len];
    
    // key must be a number between -25 to 25, fail if outside this range
    if (key < -25 || key > 25) {

        fprintf(stderr, "key is not within the bounds (-25 < key < 25)\n");
        return -1;

    }

    // encrypt array of chars using caesar cypher
    // loop through array char by char
    for (int i = 0; i < buf_len; i++) {

        // if the char in buf is alphabetical A-Z or a-z add the key to it
        if (isalpha(buf[i]) != 0) {
            tempbuf[i] = buf[i] + key;
        } else {
            // copy the non alphabetical char across 
            tempbuf[i] = buf[i];
        }

    }

    // on success return 0 
    return 0;
}

