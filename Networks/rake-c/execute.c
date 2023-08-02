#include "globals.h"

void send_file(FILE *fp, int sock) {

    printf("    sending file to socket %i\n", sock);
    char file_contents[MAX_FILE_SIZE] = {0};

    // error protection against file not being found etc
    HEADER data;
    data.cmd = 4; //CMD_SEND_FILE
    data.length = sizeof(fp);
    data.intreply = 1;
    
    // while there is still data in the file
    while(fgets(file_contents, data.length, fp) != NULL) {
        // send the data to the socket 
        if (write(sock, &data, sizeof(HEADER)) >= 0) {
            write(sock, file_contents, strlen(file_contents));
        } else {
            perror("Failed to send file.");
            exit(3);
        } 
        // wait for a reply here?
        bzero(file_contents, MAX_FILE_SIZE);
    }
}

void send_action(int currAction, int sock) {

    printf("sending action %i to socket %i\n", currAction, sock);
    char* data_to_send = actions[currAction].actions;

    // send dependent files first if there is any
    if (actions[currAction].nfiles > 0) {
        for (int i = 0; i < actions[currAction].nfiles; i++) {
            FILE *fp = fopen(actions[currAction].files[i], "r");
            send_file(fp, sock);
        }
    }

    HEADER data;
    data.cmd = 5; // CMD_EXECUTE
    data.length = strlen(data_to_send);
    data.intreply = 1;

    // send the data to the socket 
    if (write(sock, &data, sizeof(HEADER)) >= 0) {
        write(sock, data_to_send, strlen(data_to_send));
    } else {
        perror("Failed to send file");
        exit(3);
    }
    // wait for a reply here?
    bzero(data_to_send, MAX_FILE_SIZE);
}

// get server's evaluation for how busy it is.
int get_server_quote(int sock) {
    
    int nread;
    int BUFFSIZE = sizeof(HEADER);
    char buff[BUFFSIZE];

    HEADER data;
    data.cmd = 2; // CMD_QUOTE_REQUEST
    data.length = 0;
    data.intreply = 1;

    if (write(sock, &data, sizeof(HEADER)) >= 0) {
        bzero(buff, BUFFSIZE);
        nread = read(sock, buff, BUFFSIZE);
        printf("Header Received: %d bytes\n", nread);
        int n = 0;
        int len = 0, maxlen = 100;
        char buffer[maxlen];
        char* pbuffer = buffer; 
        // will remain open until the server terminates the connection
        while ((n = recv(sock, pbuffer, 1, 0)) > 0) {
            pbuffer += n;
            maxlen -= n;
            len += n;

            buffer[len] = '\0';
            printf("Socket %i Received quote: %s\n", sock, buffer);

            return atoi(buffer);
        }
        perror("Failed to send quote request");
        exit(3);
    }
    return -1;
}

// gets the lowest quote value from the hosts and selects the least busy server for the next action
int get_best_server(int *sockets) {

    int best_server = -1;
    int lowest_quote = 1000;

    for (int a = 0; a < nhosts; a++) {
        
        int gsq = get_server_quote(sockets[a]);
        if (gsq < lowest_quote) {
            lowest_quote = gsq;
            best_server = sockets[a];
        }  
    } 
    return best_server;
}

void execute() { //WILL NEED TO UPDATE TO DO BY ACTION SET
    //add remote actions to be stored

    //Reading variables
    int nread;
    int BUFFSIZE = sizeof(HEADER);
    char buff[BUFFSIZE];
    
    int server_sockets[nhosts]; //Array of sockets (servers)
    int server; //single sock to be added to array above

    for(int i = 0; i < nhosts; i++){
        server = get_sock(HOSTS[i], PORTS[i]);
        server_sockets[i] = server;
    }

    //int counter = 0; //counter for how many servers have been writen to
    int index = 0; //counter for how many servers have been read

    while(index != nhosts){ //stops when all servers have been read

        //select fd's and other variables which store the servers
        fd_set current_sockets, write_sockets, read_sockets; 
        FD_ZERO(&current_sockets);
        write_sockets = current_sockets;
        read_sockets = current_sockets;
        int max_fd = -1;
        
        //servers which haven't been writen to yet
        //if(counter < nhosts){
            for(int i = 0; i < nhosts; i++){
                if(server_sockets[i] >= 0){
                    FD_SET(server_sockets[i], &write_sockets);
                    if(server_sockets[i] > max_fd){
                        max_fd = server_sockets[i];
                    }
                }
            }
        //}
        
        //servers which haven't been read yet
        for(int i = 0; i < nhosts; i++){
            if(server_sockets[i] >= 0){
                FD_SET(server_sockets[i], &read_sockets);
                if(server_sockets[i] > max_fd){
                    max_fd = server_sockets[i];
                }
            }
        } 
        
        if(max_fd == -1){
            printf("no sockets\n");
            break;
        }
        
        //select statement which chooses the server to write to and read from
        if(select(max_fd+1, &read_sockets, &write_sockets, NULL, NULL) == -1){
            perror("select error\n");
        }
        
        // writing to the server
        for (int j=0; j < nhosts; j++){
            //if (counter < nhosts){
                if(server_sockets[j] >= 0 && FD_ISSET(server_sockets[j], &write_sockets)){ //writing to server
                    int socket = server_sockets[j];
                    for (int k = 0; k < nhosts; k++){
                        
                        int currActionset = 1;
                        // loop through actionsets here
                        for (int l = 0; l < MAX_ACTIONS; l++) {
                            // the server quotes should change as actions/files etc are sent to the server
                            if (actions[l].actionset == currActionset) {
                                // gets the least busy server and send the next action to it
                                socket = get_best_server(server_sockets);
                                send_action(l, socket);
                            } else if (actions[l].actionset == currActionset+1) {
                                // wait for previous actionset to finish
                            }
                        }
                    }                 
                    
                    //counter ++;
                }
            //}
            if(server_sockets[j] >= 0 && FD_ISSET(server_sockets[j], &read_sockets)){ //Reading from server
                bzero(buff, BUFFSIZE);
                nread = read(server_sockets[j], buff, BUFFSIZE);
                printf("Header Received: %d bytes\n", nread);
                int n = 0;
	            int len = 21, maxlen = 100;
	            char buffer[maxlen];
	            char* pbuffer = buffer;
	            n = recv(server_sockets[j], pbuffer, maxlen, 0);
		        pbuffer += n;
		        maxlen -= n;
		        len += n;
		        buffer[len] = '\0';
                index++;
            }

        }

    } 

}

void echo(int sock)
{
    int nread;
    const char* data_to_send = "Client speaking here\n";
    int BUFFSIZE = sizeof(HEADER);
    char buff[BUFFSIZE];

    HEADER data;
    data.cmd = 0;
    data.length = strlen(data_to_send);
    data.intreply = 1; 

    if (write(sock, &data, sizeof(HEADER)) < 0) {
        perror("Cannot write\n");
        exit(3);
    }
    
    write(sock, data_to_send, strlen(data_to_send));

    bzero(buff, BUFFSIZE);
    nread = read(sock, buff, BUFFSIZE);
    printf("Received %d bytes\n", nread);
    HEADER *p = (HEADER*) buff;
    printf("Received CMD=%d, length=%i, intreply=%i\n", p->cmd, p->length, p->intreply);
    
    int n = 0;
	int len = 21, maxlen = 100;
	char buffer[maxlen];
	char* pbuffer = buffer;
	// will remain open until the server terminates the connection
	while ((n = recv(sock, pbuffer, maxlen, 0)) > 0) {
		pbuffer += n;
		maxlen -= n;
		len += n;

		buffer[len] = '\0';
		printf("Received: %s\n", buffer);
	}
}
