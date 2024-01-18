#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void defaultServerConfiguration();
void updateServerConfiguration();
void sendMessage(int);

// The hostname and port added from the client are saved here
struct ServerConfiguration {
	char hostname[100];
	int port;
};

int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr;
	struct hostent *server;
	struct ServerConfiguration serverConfig;
	int sockfd;

	// Update server configuration from client input or load default one
	updateServerConfiguration(argc, argv, &serverConfig);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);	
	if(sockfd < 0) {
		printf("Error in opening the socket.\n");
		return 1;
	}

	server = gethostbyname(serverConfig.hostname);

	bzero((char *) &server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serverConfig.port);
	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);

	// Connect to the server
	if((connect(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr))) < 0){
		printf("Error in connecting to the server.\n");
		return 1;
	} 

	// If connected successfully, ready to send message
	sendMessage(sockfd);

	shutdown(sockfd,SHUT_WR); 
	close(sockfd);	

	return 0;
}

// Reads and loads server configuration from inputs arguments
void updateServerConfiguration(int argc, char *argv[], struct ServerConfiguration *config) {
    if (argc == 1) {
    	defaultServerConfiguration(config);
    } else if (argc == 3) {
    	// Copy argv[1] to config.hostname
        strncpy(config->hostname, argv[1], sizeof(config->hostname) - 1);
        config->hostname[sizeof(config->hostname) - 1] = '\0'; 

        // Convert argv[2] to an integer and save it in config.port
        config->port = atoi(argv[2]);
    } else {
    	argc < 3 ? printf("Too little arguments. Hostname and port are required.\n") : printf("Too many arguments. Hostname and port are required.\n"); 
    	defaultServerConfiguration(config);
    }

    printf("\n --- Server configuration --- \n");
	printf("Hostname: %s \t Port: %d\n", config->hostname, config->port);
	printf(" ---------------------------- \n\n");

}

// Reads and loads default server configuration 
void defaultServerConfiguration(struct ServerConfiguration *config) {
    FILE *file = fopen("configurations/clientConfiguration.txt", "r");
    if (file == NULL) {
        printf("Error loading default server configuration.\n");
        return;
    }

    char line[256];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        char *token = strtok(line, ":");
        if (token != NULL) {
            if (strcmp(token, "hostname") == 0) {
                token = strtok(NULL, ":");
                if (token != NULL) {
                    // Trim leading and trailing whitespace from hostname
                    sscanf(token, " %255[^\t\n]", config->hostname);
                }
            } else if (strcmp(token, "port") == 0) {
                token = strtok(NULL, ":");
                if (token != NULL) {
                    config->port = atoi(token);
                }
            }
        }
    }

    fclose(file);
    printf("Default server configuration loaded.\n");
}

// Sends multiple messages 
void sendMessage(int sockfd){
	char buffer[2048];

	printf("-Press CTRL+D to quit.-\n");
	
	while(!feof(stdin)){
		printf("Enter message: ");	
	
		fgets(buffer, sizeof(buffer), stdin);

		if (send(sockfd, buffer,strlen(buffer),0) == -1){
            printf("Error sending data!\n");
            break;
        }
        bzero(buffer, sizeof(buffer));
	}
	return;
}
