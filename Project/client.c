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
void sendMessageOLD(int);
void sendMessage(int);

// The hostname and port added from the client are saved here
struct ServerConfiguration {
	char hostname[100];
	int port;
};

int main()
{
	struct sockaddr_in server_addr;
	struct hostent *server;
	struct ServerConfiguration serverConfig;
	int sockfd;

	// Load default server configuration
	defaultServerConfiguration(&serverConfig);
	
	// Update server configuration from client input
	updateServerConfiguration(&serverConfig);

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

void updateServerConfiguration(struct ServerConfiguration *config){
	char response[4];

	while(1){
		printf("Do you want to connect to another server? (yes/no): ");	
		scanf("%3s", response);

		getchar();
	
		if (strcmp(response, "yes") == 0) {
			// For custom connection
		    printf("Enter the hostname of the server: ");
		    scanf("%s", config->hostname);
		    
		    printf("Enter the port of the server: ");
		    scanf("%d", &config->port);
	
		    // Used to consume the new line character from the above scanf
		    getchar();

		    break;
		} else if(strcmp(response, "no") == 0){
			// Don't do anything
			break;
		} else {
			printf("Please enter valid response. \n");
		}
	}
	    printf("\n --- Server configuration --- \n");
	    printf("Hostname: %s \t Port: %d\n", config->hostname, config->port);
	    printf(" ---------------------------- \n\n");
}

void sendMessageOLD(int sockfd){
	int n;
	char buffer[256];

	printf("Enter message: ");
	bzero(buffer, 256);
	fgets(buffer, 255, stdin);
	n = write(sockfd, buffer, strlen(buffer));
	if (n < 0)
		printf("Error in writing to socket.\n");
		
	bzero(buffer, 256);
	n = read(sockfd, buffer, sizeof(buffer));
	if (n < 0)
		printf("Error in reading from socket.\n");

	printf("Message from server: %s\n", buffer); 	
}

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
