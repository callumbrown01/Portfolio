#include "globals.h"

int main(int argc, char** argv) {

    char *path = "Rakefile.txt";
    // defaults path to rakefile if no specific path is provided
    if (argc > 1) {
        path = argv[1];
    }

    parse_file(path);

    execute();


    return 0;
}