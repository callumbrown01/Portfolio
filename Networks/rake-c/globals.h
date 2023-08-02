#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero
#include <sys/select.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>

#define MAX_HOSTS 8
#define MAX_LENGTH 20
#define MAX_LINE_LENGTH 80
#define MAX_ACTIONS 50
#define MAX_FILES_PER_ACTION 5
#define MAX_PORTS 10
#define MAX_FILE_SIZE 10000

// PORT and HOST variables for connections
extern int PORTS[MAX_PORTS];
extern char HOSTS[MAX_HOSTS][MAX_LENGTH];
extern int nhosts;
extern int nfinalactionsets;

// actions stored with their attributes easily accessible
extern struct action {
    int actionset;
    bool isRemote;
    char actions[MAX_LINE_LENGTH]; // string of action itself;
    char files[MAX_FILES_PER_ACTION][MAX_LENGTH]; // list of dependent file names
    int nfiles; // number of dependent files
    // not sure if this is needed or not
    int cmdnum;
} action;                                  

extern struct action actions[MAX_ACTIONS];   

// functions 
int starts_with(const char *line, const char *template);
char *trim_string(char *str);
void get_PORT(char *str, int hostnum);
void get_HOSTS(char *str);
void get_action(int nactions, char *str);
extern int parse_file(char *rakefile);

extern int get_sock(char *host, int port);
extern void execute();
extern void sendMsg(int sock, void* msg, uint32_t msgsize);

void send_file(FILE *fp, int sock);
void send_action(int currAction, int sock);
extern void execute();
extern void close_connection(int sock);
extern void echo(int client);
extern  int write_execute(int *connections, int count);

typedef enum {
    CMD_ECHO = 0,
    CMD_ECHOREPLY = 1,

    CMD_QUOTE_REQUEST = 2,
    CMD_QUOTE_REPLY = 3,

    CMD_SEND_FILE = 4,
    CMD_EXECUTE = 5,
    CMD_RETURN_STATUS = 6,

    RETURN_STDOUT = 7,
    RETURN_STDERR = 8,
    RETURN_FILE = 9,
}CMD;

typedef struct {
    CMD     cmd;
    int     length;
    int     intreply;
} HEADER;
