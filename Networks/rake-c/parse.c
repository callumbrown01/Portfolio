#include "globals.h"

int PORTS[MAX_PORTS];
char HOSTS[MAX_HOSTS][MAX_LENGTH] = {0};
// a list of actions containing their attributes
struct action actions[MAX_ACTIONS];   

// varianles for to execute function
int nhosts = 0;
int nfiles = 0;
int nfinalactionsets = 0;
int firstport = 0;

// simple function to check if a line starts with a tab for example
int starts_with(const char *line, const char *template) {
    
    if(strstr(line, template) != NULL){
        return 1;
    }
    return 0;
}

// trims whitespace around a string
char *trim_string(char *str) {
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

void get_PORT(char *str, int hostnum) {
    int port;
    // finds the = in the line and records the value afterwards
    if (firstport < 1){
        char *arg = strchr(str, '=');
        // moves to the word after =
        arg++;
        arg = trim_string(arg);
        //strncpy(PORT, arg, 10);
        port = atoi(arg);
        PORTS[0] = port;
        firstport = port;
    }
    else{
        port = atoi(str);
        PORTS[hostnum] = port;
    }
}

void get_HOSTS(char *str) {
    // similar to above PORT function
    char *arg = strchr(str, '=');
    arg++;
    arg = trim_string(arg);

    // replaces " " with '\0' in the arg string to split the hostnames into tokens.
    char *token = strtok(arg, " ");
    int n = 0;

    // continues until there are no non " " values in the arg string
    while(token != NULL && n < MAX_HOSTS) {

        if(strchr(token, ':') != NULL){
            char *getnum;
            getnum = strchr(token, ':');
            getnum++;
            //printf("portnum: %s\n", getnum);
            get_PORT(getnum, n);
        }
        else{
            PORTS[n] = firstport;
        }
        // assigns the value to the static HOSTS variable 
        strncpy(HOSTS[n], token, 20);
        // finds the next 
        printf("Host: %s\n", token);
        token = strtok(NULL, " ");
        
        n++;
    }
    nhosts = n;
}

// strips an action line and adds the resulting string to the current action struct
void get_action(int currAction, char *str) {

    char *action = trim_string(str);
    if (strstr(str, "remote-")) {
        actions[currAction].isRemote = 1;
        actions[currAction].cmdnum = 2;
    } else {
        actions[currAction].isRemote = 0;
    }
    if (strstr(str, "echo")){
        actions[currAction].cmdnum = 0;
    }
    strncpy(actions[currAction].actions, action, strlen(action));
}

// strips a file dependency line and splits it adding each file to the current action struct
void get_files(int currAction, char *str) {

    char *arg = trim_string(str);
    char *token = strtok(arg, " ");
    int n = 0;

    while (token != NULL && n < MAX_FILES_PER_ACTION) {
        if (starts_with(token, "requires")) {
            token = strtok(NULL, " ");
            continue;
        }
        strncpy(actions[currAction].files[n], token, MAX_LENGTH);
        token = strtok(NULL, " ");
        n++;
        nfiles++;
    }
    actions[currAction].nfiles = nfiles;
}

int parse_file(char *rakefile) {

    // open the rakefile in read mode
    FILE *file =  fopen(rakefile, "r");
    // the current line in the rakefile
    char line[MAX_LINE_LENGTH] = {0};
    // the current action in the rakefile as it reads
    // through chronologically (assumes correct syntax)
    int currAction = -1;
    // the current actionset the action belongs to
    int actionset = 0;

    if (!file) {
        printf("file '%s' unable to be located.\n", rakefile);
        return EXIT_FAILURE;
    }

    // read in all lines 
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        
        // if line is empty move to next ine
        if (*line == '\0') {
            continue;
        }

        if (starts_with(line, "PORT")) {     
            get_PORT(line, 0);
        }

        if (starts_with(line, "HOSTS")) {
            get_HOSTS(line);
        }

        // gets and sets the current actionset number for assignment to actions
        // assumes rakefile has correct syntax
        if (strstr(line, ":")) {
            actionset++;
        }

        if (starts_with(line, "    ")) {
            // if a line contains two tabs its a file dependency line
            if (starts_with(line, "        ")) {
                get_files(currAction, line);
            } else {
                // if a line contains one tab its an action line
                // so update current action and set the actionset
                currAction++;
                actions[currAction].actionset = actionset;
                get_action(currAction, line);
            }
        }
    }
    nfinalactionsets = actionset;
    return 0; 
}