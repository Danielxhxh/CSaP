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
#include <dirent.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/wait.h>

#define  MAX_LOG_SIZE     1000
#define  MAX_LOG_FILES    5
 
void signalHandler(int);
void defaultServerConfiguration();
void updateServerConfiguration();
void accessLog();
int openLog();
int countLogs();
int writeLog(int, char*, char*);
char* getTime();
int removeOldestLog(char*);
void getNewestLog(char*);
int checkLogSize(char*);

// The directory where the log file is saved and the listening port
struct ServerConfiguration {
	char directory[100];
	int port;
};
char currentLogFile[27];
int sockfd;
sem_t semaphore;
pid_t fatherPid;

int main(int argc, char *argv[]){
	struct sockaddr_in server_addr, client_addr;
	int newsockfd, clientlen, pid, n;
	struct ServerConfiguration serverConfig;			

	// Make all the changes of the server configuration
	updateServerConfiguration(argc, argv, &serverConfig);


	// Initialize semaphore
    if (sem_init(&semaphore, 1, 1) == -1) {
        printf("Semaphore initialization failed.\n");
        return -1;
    }

	sockfd = socket(AF_INET, SOCK_STREAM, 0);	
	if(sockfd < 0){
		printf("Error in creating the socket.\n");
		return -1;
	}

	// Initializes the memory of server_addr to zero, clearing sensitive data.
	bzero((char *) &server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serverConfig.port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind an address to a socket
	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		printf("Error on binding.\n");
		return -1;
	}

	listen(sockfd, SOMAXCONN); // max allowed value by the system
	printf("Server is listening...\n");

	clientlen = sizeof(client_addr);

	// Setup a function handler to close everything in case of Ctrl+C 
	signal(SIGINT, signalHandler);
    printf("Press CTRL+C to quit the server.\n");


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

    		// Client connected message
			char connectionMessage[128];
			sprintf(connectionMessage, "Client [%s:%d] connected. ", clientIP, clientPort);

			accessLog(connectionMessage, NULL, &serverConfig);
					
			bzero(buffer, 2048);

		    while (1) {		
		    	n = recv(newsockfd, buffer, sizeof(buffer), 0);    	
				if(n > 0){
					char header[77];
					sprintf(header, "[%s:%d] at %s", clientIP, clientPort, getTime());

					accessLog(header, buffer, &serverConfig);

					bzero(buffer, 2048);
				} else if (n == 0){
					char disconnectionMessage[128];
					sprintf(disconnectionMessage, "Client [%s:%d] disconnected. ", clientIP, clientPort);

					accessLog(disconnectionMessage, NULL, &serverConfig);

					bzero(buffer, 2048);
					break;
				} else if (n < 0){
					printf("Error in receving data from the client.\n");
				}

		    }

		    exit(EXIT_SUCCESS);
		} else if(pid > 0){
			fatherPid = getpid();
		} else if(pid < 0){
			printf("Error on fork.\n");
		}

		close(newsockfd);
	}
	close(sockfd);
	return 0;
}

// Handle CTRL+C
void signalHandler(int signum) {
    if (getpid() == fatherPid) {
	    printf("Signal CTRL+C received, waiting for the connections to close...\n");

	    while (waitpid(-1, NULL, 0) > 0);
	    
	    // Close the socket
	    if (close(sockfd) < 0) {
	        printf("Error closing socket.\n");
	    }

	    printf("Exiting...\n");

	    exit(EXIT_SUCCESS);
    }
}

// Access the log
void accessLog(char *header, char *buffer, struct ServerConfiguration *serverConfig) {
    sem_wait(&semaphore);

    // Open the log file or create it if it doesn't exist
    int logFd = openLog(serverConfig);
    if (logFd == -1){
        printf("Error opening/creating the Log file.\n");
        return;
    }

    // Function to write on the log file
    writeLog(logFd, header, buffer);

    close(logFd);

    sem_post(&semaphore);
}

// Reads and loads server configuration from inputs arguments
void updateServerConfiguration(int argc, char *argv[], struct ServerConfiguration *config) {
    if (argc == 1) {
    	defaultServerConfiguration(config);
    } else if (argc == 3) {
    	// Copy argv[1] to config.directory
        strncpy(config->directory, argv[1], sizeof(config->directory) - 1);
        config->directory[sizeof(config->directory) - 1] = '\0'; 

        // Convert argv[2] to an integer and save it in config.port
        config->port = atoi(argv[2]);
    } else {
    	argc < 3 ? printf("Too little arguments. Directory and port are required.\n") : printf("Too many arguments.Directory and port are required.\n"); 
    	defaultServerConfiguration(config);
    }

    printf("\n --- Server configuration --- \n");
	printf("Directory: %s \t Port: %d\n", config->directory, config->port);
	printf(" ---------------------------- \n\n");
}

// Reads and loads default server configuration
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

// Handles all the logic about log files opening
int openLog(struct ServerConfiguration *config){
	
	mode_t mode = 0644;

	char filePath[strlen(config->directory) + strlen(currentLogFile) + 1]; // +1 for the null terminator
	int numberLogs = countLogs(config->directory);	
	int overThreshold;

	if(numberLogs == -1) {
		return -1;
	}
	// If no log file exists, create new log file
	if (numberLogs == 0) {
	    time_t t = time(NULL);
    	struct tm tm_info = *localtime(&t);
    	strftime(currentLogFile, sizeof(currentLogFile), "%d-%m-%Y-%H-%M-%S.log", &tm_info);

    // If at least one log file exists, get the newest one
	} else {
		getNewestLog(config->directory);
		strcpy(filePath, config->directory);
	    strcat(filePath, currentLogFile);
			
		overThreshold = checkLogSize(filePath);

		if(overThreshold == -1){
			printf("Error in checking the threshold.\n");
			return -1;
		} 

		// If maximum log file size is exceded
		if (overThreshold == 1) {	
		
			// Clear (reset) filePath
	    	memset(filePath, 0, sizeof(filePath));	

			if (numberLogs == MAX_LOG_FILES) {				
				// Remove oldest log file
				if(removeOldestLog(config->directory) == -1){
					return -1;
				}
			}

			time_t t = time(NULL);
			struct tm tm_info = *localtime(&t);
			strftime(currentLogFile, sizeof(currentLogFile), "%d-%m-%Y-%H-%M-%S.log", &tm_info);
		}
		
	}	

    strcpy(filePath, config->directory);
    strcat(filePath, currentLogFile);

	int fd = open(filePath, O_WRONLY | O_CREAT, mode);

	return fd;
}

// Returns number of log files 
int countLogs(char *directory){
	int counter = 0;
    struct dirent *dirent;
    DIR * dir = opendir(directory);

    if (dir == NULL){
        printf("Error to open logs directory %s\n",directory);
        return -1;
    }	

    while ((dirent = readdir(dir)) != NULL){
    	 if (dirent->d_type == DT_REG ){ 
            counter++;
        }
    }

    closedir(dir);
    return(counter);
}

// Removes oldest log file. Return 1 if successful, -1 if error
int removeOldestLog(char* directory){
    char oldestFileName[27];
    time_t oldestFileTime = 0;

    DIR *dir;
    struct dirent *entry;

    dir = opendir(directory);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { 
            char filePath[512];
            strcpy(filePath, directory);
            strcat(filePath, entry->d_name);


            struct stat fileStat;
            if (stat(filePath, &fileStat) == 0) {
                // Check if the current file is older than the current oldest file
                if (oldestFileTime == 0 || difftime(fileStat.st_mtime, oldestFileTime) < 0) {
                    oldestFileTime = fileStat.st_mtime;
                    strcpy(oldestFileName, entry->d_name);
                }
            } else {
                printf("Error getting file status.\n");
            }
        }
    }

    closedir(dir);

    // Remove the oldest file
    char filePath[512];

	strcpy(filePath, directory);
    strcat(filePath, oldestFileName);

    if (remove(filePath) == 0) {
        printf("Oldest file %s removed successfully.\n", oldestFileName);
        return 1;
    } else {
		printf("Error removing oldest file.\n");
        return -1;
    }   
}

// Writes to log file
int writeLog(int fd, char *header, char *buffer){
	// Initial file position
    off_t initialPosition = lseek(fd, (off_t)0, SEEK_CUR);


    // Write at the end of the file
    lseek(fd, (off_t)0, SEEK_END);

    // Used when client connects/disconnects
    if (buffer == NULL) {
        ssize_t headerBytesWritten = write(fd, header, strlen(header));
        write(fd, "\n", 1);

        if (headerBytesWritten == -1) {
            printf("Error writing to file.\n");
            lseek(fd, initialPosition, SEEK_SET);
            return -1;
        }

        return (int)(initialPosition / strlen(header));
    } else {
		char combinedData[strlen(header) + strlen("Message: ") + strlen(buffer) + 1];
		sprintf(combinedData, "%sMessage: %s", header, buffer);


	    // Write header + message together
		ssize_t bytesWritten = write(fd, combinedData, strlen(combinedData));


	    if (bytesWritten == -1) {
	    	printf("Error writing to file.\n");
	    	lseek(fd, initialPosition, SEEK_SET);
	    	return -1;
	    }

	    return (int)(initialPosition / (strlen(header) + strlen(buffer) + 1));
    }
}

// Updates the 'currentLogFile' variable with the name of the most recent log file
void getNewestLog(char* directory){
   
    DIR *dir;
    struct dirent *entry;
    struct stat newestStat;

    if ((dir = opendir(directory)) != NULL) {
	    while ((entry = readdir(dir)) != NULL) {
	    	if (entry->d_type == DT_REG) {
		        char filePath[256];
		        strcpy(filePath, directory);
		    	strcat(filePath, entry->d_name);

		        struct stat fileStat;
		        if (stat(filePath, &fileStat) == 0) {
		        	if (!currentLogFile[0] || difftime(fileStat.st_mtime, newestStat.st_mtime) > 0) {
		            	newestStat = fileStat;
		            	strncpy(currentLogFile, entry->d_name, 27);
		            	currentLogFile[27] = '\0';

		        	}   
		        } else{
		        	printf("Error getting file status.\n");
		        }
	    	}
	    }
	}else  {
        printf("Error opening directory.\n");
    
    }

    closedir(dir);
}

// If log file size (rows) is over a threshold returns 1, else 0. In case of error -1 
int checkLogSize(char* logFilePath){
	FILE *fp = fopen(logFilePath, "a");
	
	fseek(fp, 0, SEEK_END);
	int nrCharacters = ftell(fp);
	fseek(fp, 0, SEEK_SET);

    return nrCharacters >= MAX_LOG_SIZE ? 1 : 0;
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