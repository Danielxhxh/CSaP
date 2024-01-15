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

#define  MAX_LOG_SIZE     4
#define  MAX_LOG_FILES    3
 
void defaultServerConfiguration();
void updateServerConfiguration();
int openLog();
int countLogs();
void writeLog(int, char*, char*);
char* getTime();
int removeOldestLog(char*);
void getNewestLog(char*, char[]);
int checkLogSize();

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

	char fileName[27];
	char filePath[strlen(config->directory) + strlen(fileName) + 1]; // +1 for the null terminator
	int numberLogs = countLogs(config->directory);	
	int overThreshold;

	if(numberLogs == -1) {
		return -1;
	}
	// If no log file exists, create new log file
	if (numberLogs == 0) {
	    time_t t = time(NULL);
    	struct tm tm_info = *localtime(&t);
    	strftime(fileName, sizeof(fileName), "%d-%m-%Y-%H-%M-%S.log", &tm_info);

    // If at least one log file exists, get the newest one
	} else {
		getNewestLog(config->directory, fileName);
		strcpy(filePath, config->directory);
	    strcat(filePath, fileName);
			
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
			strftime(fileName, sizeof(fileName), "%d-%m-%Y-%H-%M-%S.log", &tm_info);
		}
		
	}	

    strcpy(filePath, config->directory);
    strcat(filePath, fileName);

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
void writeLog(int fd, char *header, char *buffer){
	char combinedData[strlen(header) + strlen(buffer) + 1];
    strcpy(combinedData, header);
    strcat(combinedData, buffer);

    // TODO: Should  i use fcntl??


    // Write at the end of the file
    lseek(fd, (off_t)0, SEEK_END);

    // Write header + message together
	ssize_t bytesWritten = write(fd, combinedData, strlen(combinedData));


    if (bytesWritten == -1) {
    	printf("Error writing to file.\n");
    }
}

// Updates the 'fileName' variable with the name of the most recent log file
void getNewestLog(char* directory, char fileName[27]){
   
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
		        	if (!fileName[0] || difftime(fileStat.st_mtime, newestStat.st_mtime) > 0) {
		            	newestStat = fileStat;
		            	strncpy(fileName, entry->d_name, 27);
		            	fileName[27] = '\0';

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
	int fd = open(logFilePath, O_RDONLY);
    if (fd == -1) {
        printf("Error opening file: %s\n", logFilePath);
        return -1;
    }

    off_t fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == -1) {
        printf("Error seeking to end of file\n");
        close(fd);
        return -1;
    }

    // Reset file pointer to the beginning
    lseek(fd, 0, SEEK_SET);

    char *buffer = (char *)malloc(fileSize);
    if (!buffer) {
        printf("Error allocating memory\n");
        close(fd);
        return -1;
    }

    ssize_t bytesRead = read(fd, buffer, fileSize);
    if (bytesRead == -1) {
        printf("Error reading file\n");
        free(buffer);
        close(fd);
        return -1;
    }

    close(fd);

    int rowCounter = 0;
    for (ssize_t i = 0; i < bytesRead; i++) {
        if (buffer[i] == '\n') {
            rowCounter++;
        }
    }

    free(buffer);

    return rowCounter >= MAX_LOG_SIZE ? 1 : 0;
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