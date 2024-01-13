#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

void defaultServerConfiguration();
void updateServerConfiguration();
int openLog();
void writeLog(int, char*, char*);
char* getTime();

// The directory where the log file is saved and the listening port
struct ServerConfiguration {
	char directory[100];
	int port;
};

int main()
{
	struct sockaddr_in server_addr, client_addr;
	int sockfd, newsockfd, clientlen, pid, n;
	struct ServerConfiguration serverConfig;

	// Load default server configuration
	defaultServerConfiguration(&serverConfig);

	// Update server configuration
	updateServerConfiguration(&serverConfig);

	// Open the log file or create it if doesn't exist
	int logFd = openLog(&serverConfig);
	if (logFd == -1){
		printf("Error opening/creating the Log file.\n");
		return 1;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);	
	if(sockfd < 0){
		printf("Error in creating the socket.\n");
		return 1;
	}

	// Initializes the memory of server_addr to zero, clearing sensitive data.
	bzero((char *) &server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serverConfig.port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind an address to a socket
	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		printf("Error on binding.\n");
		return 1;
	}

	listen(sockfd, SOMAXCONN); // max allowed value by the system
	printf("Server is listening...\n");

	clientlen = sizeof(client_addr);

	// While loop to listen forever and accept multiple connections
	while(1){
		newsockfd = accept(sockfd, (struct sockaddr*)&client_addr, &clientlen);
		if(newsockfd == -1)
			printf("Error on acception.\n");		

		// It continues here only after a client connects
		pid = fork();

		if(pid == 0){
			close(sockfd);

			char buffer[2048];

		    char clientIP[INET_ADDRSTRLEN];
    		int clientPort;
    		inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, INET_ADDRSTRLEN);
    		clientPort = ntohs(client_addr.sin_port);

			bzero(buffer, 2048);
			

		    while ((n = read(newsockfd, buffer, sizeof(buffer))) > 0) {		    	
				// printf("%s:%d at %s", clientIP, clientPort, getTime());
				//printf("Message from client: %s\n", buffer); 

				char header[75];  // Adjust the size as needed
				sprintf(header, "%s:%d at %s", clientIP, clientPort, getTime());

				// Function to write on Log file
				writeLog(logFd, header, buffer);

				bzero(buffer, 2048);
		    }

			exit(0);
		}else if(pid < 0){
			printf("Error on fork.\n");
		}

		close(newsockfd);
	}
	close(sockfd);
	close(logFd);
	return 0;
}

void defaultServerConfiguration(struct ServerConfiguration *config) {
    FILE *file = fopen("configurations/serverConfiguration.txt", "r");
    if (file == NULL) {
        printf("Error loading default server configuration.\n");
        return;
    }

    char line[256];
    

    while (fgets(line, sizeof(line), file) != NULL) {
        char *token = strtok(line, ":");
        if (token != NULL) {
            if (strcmp(token, "directory") == 0) {
                token = strtok(NULL, ":");
                if (token != NULL) {
                    // Trim leading and trailing whitespace from directory
                    sscanf(token, " %255[^\t\n]", config->directory);
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
		printf("Do you want to modify the default server configuration? (yes/no): ");	
		scanf("%3s", response);

		getchar();
	
		if (strcmp(response, "yes") == 0) {
			// Change the directory of the Log file
		    printf("Enter the directory where the Log file will be saved: ");
		    scanf("%s", config->directory);
		    
		    printf("Enter the listening port of the server: ");
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
	    printf("Directory: %s \t Port: %d\n", config->directory, config->port);
	    printf(" ---------------------------- \n\n");
}

int openLog(struct ServerConfiguration *config){
	
	mode_t mode = 0644;

	char fileName[27];  // Adjust the size as needed
    time_t t = time(NULL);
    struct tm tm_info = *localtime(&t);
    strftime(fileName, sizeof(fileName), "%d-%m-%Y-%H-%M-%S.log", &tm_info);


	char filePath[strlen(config->directory) + strlen(fileName) + 2]; // +2 for the path separator and null terminator
    strcpy(filePath, config->directory);
    strcat(filePath, fileName);

	int fd = open(filePath, O_WRONLY | O_CREAT, mode);
    
	return fd;
}

void writeLog(int fd, char *header, char *buffer){
	char combinedData[strlen(header) + strlen(buffer) + 1];
    strcpy(combinedData, header);
    strcat(combinedData, buffer);

    // TODO: Should  i use fcntl??


    // Write header + message together
    ssize_t bytesWritten = write(fd, combinedData, strlen(combinedData));

    if (bytesWritten == -1) {
    	printf("Error writing to file.\n");
    }
}

// Used to get the time of connection of each client
char* getTime(){
	time_t rawtime;
	struct tm * timeinfo;
	
	time(&rawtime);
  	timeinfo = localtime(&rawtime);
  	
  	// From struct to string
	return asctime(timeinfo);
}