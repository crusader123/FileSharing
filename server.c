/* tcpserver.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <netdb.h>
#include <time.h>
#include <openssl/md5.h>
#include <fnmatch.h>

void indexGet(char *str);
void execute(char **cmnd);
void uploadFile(char *str);
void downloadFile(char *str);
void shortList(char [],char []);
void longList();
void longListRegEx();
void getInfo(char []);

unsigned char hashst[1000];
/* Global Varialbles*/
int connected=0;
char *indexGetArray,*infoAbtFile;

int hash(char * filename)
{
	memset(hashst,'\0',sizeof(hashst));
	unsigned char c[MD5_DIGEST_LENGTH];
	//char *filename="hello";
	int i;
	FILE *inFile = fopen (filename, "rb");
	//FILE *outFile = fopen("out.txt","w+");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (inFile == NULL) {
		printf ("%s can't be opened.\n", filename);
		return 0;
	}

	MD5_Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0)
		MD5_Update (&mdContext, data, bytes);
	MD5_Final (c,&mdContext);
	for(i = 0; i < MD5_DIGEST_LENGTH; i++) 
	{
		printf("%02x",c[i]);
		//hashst[i]=c[i];
		//sprintf(hashst,"%02x", c[i]);
	}
	strcpy(hashst,c);
	fclose (inFile);
	return 0;
}
int main()
{
	int sock, bytes_recieved,bytes_received1 , tru = 1;  
	char send_data[1124],recv_data[1124],recv_data1[1124],str[1124],str1[1124],str2[1124],str3[1124],nameFile[102];   
	long int fileSize;
	int size=0;
	char c;
	memset(hashst,'\0',sizeof(hashst));
	fd_set read_flags,write_flags; // the flag sets to be used
	struct timeval waitd = {10, 0};  
	struct sockaddr_in server_addr,client_addr;    
	int sin_size;
	//fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("Socket");
		exit(1);
	}
	server_addr.sin_family = AF_INET;         
	server_addr.sin_port = htons(5000);     
	server_addr.sin_addr.s_addr = INADDR_ANY; 
	bzero(&(server_addr.sin_zero),8); 
	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))== -1)
	{
		perror("Unable to bind");
		exit(1);
	}
	if (listen(sock, 5) == -1) 
	{
		perror("Listen");
		exit(1);
	}
	printf("\nTCPServer Waiting for client on port 5000");
	fflush(stdout);
	char a;
	int flagQuit=0;
	while(1)
	{  

		sin_size = sizeof(struct sockaddr_in);
		int add_len = sizeof( struct sockaddr);
		connected = accept(sock, (struct sockaddr *)&client_addr,&add_len);
		// flags = fcntl(socket,F_GETFL,0);
		//assert(flags != -1);
		//fcntl(socket, F_SETFL, flags | O_NONBLOCK);
		if(connected < 0)continue;
		fcntl(connected, F_SETFL, O_NONBLOCK);
		printf("\n I got a connection from (%s , %d)\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		fflush(stdout);
		while(1)
		{
			FD_ZERO(&read_flags);
			FD_ZERO(&write_flags);
			FD_SET(connected, &read_flags);
			FD_SET(connected, &write_flags);
			FD_SET(STDIN_FILENO, &read_flags);
			FD_SET(STDIN_FILENO, &write_flags);
			//printf("\n SEND (q or Q to quit) : ");
			int sel = select(connected+1, &read_flags, &write_flags, (fd_set*)0, &waitd);
			size=0;
			memset(send_data,'\0',sizeof(send_data));
			if(sel > 0)
			{
				if(FD_ISSET(0, &read_flags))
				{
					read(0,&c,1);
					while(c!='\n')   // better to send data by blocks... in case of large files
					{
						send_data[size]=c;
						size++;
						read(0,&c,1);
					}
					send_data[size]='\0';
					//printf("send : %d\n",size);
				}
				if(FD_ISSET(connected, &write_flags))
				{
					FD_CLR(connected, &write_flags);
					if (strcmp(send_data , "q") == 0 || strcmp(send_data , "Q") == 0)
					{
						send(connected, send_data,size, 0); 
						close(connected);
						flagQuit=1;
						break;
					}
					else
					{
						send(connected, send_data,size, 0);  
						memset(str2,'\0',sizeof(str2));
						memset(str1,'\0',sizeof(str1));
						//str = trunc1(send_data);       // removes xtra spaces and tabs
						strcpy(str1,send_data);
						strcpy(str2,send_data);
						char *token1=NULL;
						token1 = strtok(str1," ");
						while(token1!=NULL)
						{
							if(strcmp(token1,"FileDownload")==0)
							{
								break;
							}
							else if(strcmp(token1,"FileUpload")==0)
							{
								char *token2=NULL;
								token2 = strtok(str2," ");
								int len2=0;
								while(token2!=NULL)
								{
									len2++;
									if(len2==2)
									{
										memset(nameFile,'\0',sizeof(nameFile));
										strcpy(nameFile,token2);
										printf("name of file entered for upload is %s\n",nameFile);
										getInfo(nameFile);
										printf("%s\n",infoAbtFile);
										break;
									}
									token2 = strtok(NULL," ");
								}
								char *sendQuery = (char *)malloc(sizeof(char)*1125);
								memset(sendQuery,'\0',sizeof(sendQuery));
								sprintf(sendQuery,"FileUpload FileName %s",nameFile);
								//printf("1.%s\n",sendQuery);
								sleep(1);
								send(connected,sendQuery,strlen(sendQuery),0);
								FILE * pFile;
								long lSize;
								char * buffer;
								size_t result;

								printf("nameFIle is %s\n",nameFile);
								pFile = fopen (nameFile, "rb" );
								if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

								// obtain file size:
								fseek (pFile , 0 , SEEK_END);
								lSize = ftell (pFile);
								memset(sendQuery,'\0',sizeof(sendQuery));
								sprintf(sendQuery,"FileUpload FileSize %ld",lSize);
								//printf("2.%s\n",sendQuery);
								sleep(1);
								send(connected,sendQuery,strlen(sendQuery),0);
								rewind (pFile);
								// allocate memory to contain the whole file:
								buffer = (char*) malloc (sizeof(char)*lSize);
								if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}
								// copy the file into the buffer:
								result = fread (buffer,1,lSize,pFile);
								if (result != lSize) {fputs ("Reading error",stderr); exit (3);}
								// terminate
								fclose (pFile);


								// Whole file at once
								/*	free(sendQuery);
									sendQuery = (char *)malloc(sizeof(char)*(lSize+100));
									memset(sendQuery,'\0',sizeof(sendQuery));
									sprintf(sendQuery,"FileDownload Data %s",buffer);
									printf("3.%s\n",sendQuery);
									send(connected,sendQuery,strlen(sendQuery),0);
								 */	 

								char *tempBuffer;
								tempBuffer = (char *)malloc(sizeof(char)*1025);
								int numChunks = lSize/(1024-16);
								int i=0;
								for(i=0;i<numChunks;i++)
								{
									memset(sendQuery,'\0',sizeof(sendQuery));
									memset(tempBuffer,'\0',sizeof(tempBuffer));
									strncat(tempBuffer,buffer+i*1008,1008);
									sprintf(sendQuery,"FileUpload Data %s",tempBuffer);
									//printf("3.%s\n",sendQuery);
									sleep(1);
									send(connected,sendQuery,strlen(sendQuery),0);
								}
								if(lSize%1008)
								{
									int rem = lSize%1008;
									memset(sendQuery,'\0',sizeof(sendQuery));
									memset(tempBuffer,'\0',sizeof(tempBuffer));
									strncat(tempBuffer,buffer+numChunks*1008,rem);
									sprintf(sendQuery,"FileUpload Data %s",tempBuffer);
									//printf("3.%s\n",sendQuery);
									sleep(1);
									send(connected,sendQuery,strlen(sendQuery),0);
								}
								free (buffer); 
								break;
							}
							token1 = strtok(NULL," ");
							break;
						}
					}
					fflush(stdout);
				}
				//if((bytes_recieved = recv(connected,recv_data,1024,0))!=-1)
				if(FD_ISSET(connected, &read_flags))
				{
					FD_CLR(connected, &read_flags);
					//printf("byes :%s\n",recv_data);
					bytes_recieved = recv(connected,recv_data,1024,0);
					recv_data[bytes_recieved] = '\0';
					if (strcmp(recv_data , "q") == 0 || strcmp(recv_data , "Q") == 0)
					{
						close(connected);
						flagQuit=1;
						break;
					}
					else 
					{
						//printf("\n RECIEVED DATA = %s " , recv_data);
						//printf("%s\n",recv_data);
						memset(str1,'\0',sizeof(str1));
						memset(str2,'\0',sizeof(str2));
						memset(str3,'\0',sizeof(str3));
						//memset(str3,'\0',sizeof(str3));
						//str = trunc1(send_data);       // removes xtra spaces and tabs
						strcpy(str1,recv_data);
						strcpy(str2,recv_data);
						strcpy(str3,recv_data);
						char *token=NULL;
						token = strtok(str1," ");
						while(token!=NULL)
						{
							if(strcmp(token,"FileHash")==0)
							{   
								char * token2 = NULL;
								token2 = strtok(str2," ");
								char choice[100];
								memset(choice,'\0',sizeof(choice));
								token2 = strtok(NULL," ");
								strcpy(choice,token2);
								//printf(" cjoice is %s\n",choice);
								if(strcmp(choice,"Verify")==0)
								{
									token2 = strtok(NULL," ");
									memset(nameFile,'\0',sizeof(nameFile));
									strcpy(nameFile,token2);
								//	printf(" file is %s\n",nameFile);
									int a = hash(nameFile);
									//printf("%02x\n",hashst);
									int i1;
								//	for(i1 = 0; i1 < MD5_DIGEST_LENGTH; i1++) printf("%02x",hashst[i1]);
								//	printf("%s\n",hashst);
									char sendQuery[1000];
									memset(sendQuery,'\0',sizeof(sendQuery));
									sprintf(sendQuery,"FileHash VerifyRecv %s",hashst);
									send(connected,sendQuery,strlen(sendQuery),0);
								}

								else if(strcmp(choice,"VerifyRecv")==0)
								{   
									token2 = strtok(NULL," ");
									unsigned char hashs[MD5_DIGEST_LENGTH];
									strcpy(hashs,token2);
									int i1; 
									for(i1=0;i1<MD5_DIGEST_LENGTH;i1++)printf("%02x",hashs[i1]);
									printf("\n\n");

								}

							}
							else if(strcmp(token,"IndexGet")==0)
							{
								char *token2=NULL;
								token2 = strtok(str2," ");
								token2 = strtok(NULL," ");
								if(strcmp(token2,"LongList")==0)
								{
									longList();
									send(connected,indexGetArray,strlen(indexGetArray), 0);
								}
								else if(strcmp(token2,"ShortList")==0)
								{
									char * token3=NULL;
									int len5=0;
									char str11[100]="",str22[100]="";
									token2 = strtok(NULL," ");
									while(token2!=NULL)
									{
										len5++;
										if(len5<3)
										{
											strcat(str11,token2);
											if(len5==1)
												strcat(str11," ");
										}
										else
										{
											strcat(str22,token2);
											if(len5==3)
												strcat(str22," ");
										}
										token2=strtok(NULL," ");
									}
									//printf("ShortList inputs are:\n");
									//printf("%s\n",str11);
									//printf("%s\n",str22);
									shortList(str11,str22);
									send(connected,indexGetArray,strlen(indexGetArray), 0);
								}
								else if(strcmp(token2,"RegEx")==0)
								{
									token2 = strtok(NULL," ");
									char regEX[100]="";
									strcpy(regEX,token2);
									printf("regEX is %s\n",regEX);
									longListRegEx(regEX);
									send(connected,indexGetArray,strlen(indexGetArray), 0);
								}
								break;
							}
							else if(strcmp(token,"FileDownload")==0)
							{
								char *token2=NULL;
								token2 = strtok(str2," ");
								token2 = strtok(NULL," ");
								int len4=0;
								while(token2!=NULL)
								{
									len4++;
									if(strcmp(token2,"FileName")==0)
									{
										char *token3 = NULL;
										token3 = strtok(str3," ");
										int len3=0;
										while(token3!=NULL)
										{
											len3++;
											if(len3==3)
											{
												memset(nameFile,'\0',sizeof(nameFile));
												strcpy(nameFile,token3);
												printf("Name of download FIle is %s\n",nameFile);
												break;
											}
											token3 = strtok(NULL," ");
										}
									}
									else if(strcmp(token2,"FileSize")==0)
									{
										char *token3 = NULL;
										token3 = strtok(str3," ");
										int len3=0;
										while(token3!=NULL)
										{
											len3++;
											if(len3==3)
											{
												char temp[100] = "";
												strcpy(temp,token3);
												fileSize = atol(temp);
												printf("Size of download FIle is %ld\n",fileSize);
												break;
											}
											token3 = strtok(NULL," ");
										}
									}
									else if(strcmp(token2,"Data")==0)
									{
										int fd = open(nameFile,O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
										char *uploadBuffer = (char *)malloc(sizeof(char)*1025);
										memset(uploadBuffer,'\0',sizeof(uploadBuffer));
										strcpy(uploadBuffer,recv_data+18);
										printf("Len of write buffer is %d\n",strlen(uploadBuffer));
										write (fd,uploadBuffer,strlen(uploadBuffer));
										printf("Data received from download FIle is %s\n",uploadBuffer);
										close(fd);
									}
									else
									{
										char * token1=NULL;
										token1 = strtok(str3," ");
										int len=0;
										while(token1!=NULL)
										{
											len++;
											if(len==2)
											{
												memset(nameFile,'\0',sizeof(nameFile));
												strcpy(nameFile,token1);

											}
											token1=strtok(NULL," ");
										}
										getInfo(nameFile);
										send(connected,infoAbtFile,strlen(infoAbtFile),0);
										char *sendQuery = (char *)malloc(sizeof(char)*1125);
										memset(sendQuery,'\0',sizeof(sendQuery));
										sprintf(sendQuery,"FileDownload FileName %s",nameFile);
										printf("1.%s\n",sendQuery);
										usleep(100);
										send(connected,sendQuery,strlen(sendQuery),0);
										FILE * pFile;
										long lSize;
										char * buffer;
										size_t result;

										printf("nameFIle is %s\n",nameFile);
										pFile = fopen (nameFile, "rb" );
										if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

										// obtain file size:
										fseek (pFile , 0 , SEEK_END);
										lSize = ftell (pFile);
										memset(sendQuery,'\0',sizeof(sendQuery));
										sprintf(sendQuery,"FileDownload FileSize %ld",lSize);
										printf("2.%s\n",sendQuery);
										usleep(100);
										send(connected,sendQuery,strlen(sendQuery),0);
										rewind (pFile);
										//fclose (pFile);
										//exit(0);
										// allocate memory to contain the whole file:
										buffer = (char*) malloc (sizeof(char)*lSize);
										if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}
										// copy the file into the buffer:
										result = fread (buffer,1,lSize,pFile);
										if (result != lSize) {fputs ("Reading error",stderr); exit (3);}
										// terminate
										fclose (pFile);

										/*
										   Whole file at once
										   free(sendQuery);
										   sendQuery = (char *)malloc(sizeof(char)*(lSize+100));
										   memset(sendQuery,'\0',sizeof(sendQuery));
										   sprintf(sendQuery,"FileUpload Data %s",buffer);
										   printf("3.%s\n",sendQuery);
										   send(connected,sendQuery,strlen(sendQuery),0);
										 */

										char *tempBuffer;
										tempBuffer = (char *)malloc(sizeof(char)*1025);
										int numChunks = lSize/(1024-18);
										int i=0;
										for(i=0;i<numChunks;i++)
										{
											memset(sendQuery,'\0',sizeof(sendQuery));
											memset(tempBuffer,'\0',sizeof(tempBuffer));
											strncat(tempBuffer,buffer+i*1006,1006);
											sprintf(sendQuery,"FileDownload Data %s",tempBuffer);
											printf("3.%s\n",sendQuery);
											usleep(100);
											send(connected,sendQuery,strlen(sendQuery),0);
											printf("ACK\n");
										}
										if(lSize%1006)
										{
											int rem = lSize%1006;
											memset(sendQuery,'\0',sizeof(sendQuery));
											memset(tempBuffer,'\0',sizeof(tempBuffer));
											strncat(tempBuffer,buffer+numChunks*1006,rem);
											sprintf(sendQuery,"FileDownload Data %s",tempBuffer);
											printf("3.%s\n",sendQuery);
											usleep(100);
											send(connected,sendQuery,strlen(sendQuery),0);
											printf("ACK\n");
										}
										free (buffer);
										//printf("Download Complete\n");
										//char endMsg[] = "Download Complete";
										//send(connected,endMsg,strlen(endMsg),0);
									}



									break;
								}
							}
							else if(strcmp(token,"FileUpload")==0)
							{
								char *token2=NULL;
								token2 = strtok(str2," ");
								while(token2!=NULL)
								{
									if(strcmp(token2,"FileName")==0)
									{
										char *token3 = NULL;
										token3 = strtok(str3," ");
										int len3=0;
										while(token3!=NULL)
										{
											len3++;
											if(len3==3)
											{
												memset(nameFile,'\0',sizeof(nameFile));
												strcpy(nameFile,token3);
												printf("Name of UPload FIle is %s\n",nameFile);													
												//getInfo(nameFile);
												//send(connected,infoAbtFile,strlen(infoAbtFile),0);
												break;
											}
											token3 = strtok(NULL," ");
										}
									}
									else if(strcmp(token2,"FileSize")==0)
									{
										char *token3 = NULL;
										token3 = strtok(str3," ");
										int len3=0;
										while(token3!=NULL)
										{
											len3++;
											if(len3==3)
											{
												char temp[100] = "";
												strcpy(temp,token3);
												fileSize = atol(temp);
												printf("Size of UPload FIle is %ld\n",fileSize);
												break;
											}
											token3 = strtok(NULL," ");
										}
									}
									else if(strcmp(token2,"Data")==0)
									{
										int fd1 = open(nameFile,O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
										char *uploadBuffer = (char *)malloc(sizeof(char)*1025);
										memset(uploadBuffer,'\0',sizeof(uploadBuffer));
										strcpy(uploadBuffer,recv_data+16);
										printf("Len of write buffer is %d\n",strlen(uploadBuffer));
										write(fd1,uploadBuffer,strlen(uploadBuffer));
										close(fd1);
										//printf("Data received from Upload FIle is %s\n",uploadBuffer);
									}
									token2 = strtok(NULL," ");
								}
								break;
								//uploadFile(send_data);
							}
						else
						{
							printf("%s\n",recv_data);
						}
							token = strtok(NULL," ");
							break;
						}
						memset(send_data,0,sizeof(send_data));
					}
					fflush(stdout);
				}
			}
			//printf("common\n");
		}
		if(flagQuit==1)
			break;
	}       
	close(sock);
	return 0;
}

void uploadFile(char str[])
{
	char filename[100];
	char send_data[1025];
	memset(filename,'\0',sizeof(filename));
	char *token = NULL;
	int len=0;
	token = strtok(str," ");
	while(token!=NULL)
	{
		len++;
		if(len==2)
		{
			strcpy(filename,token);
			break;
		}
		token = strtok(NULL," ");
	}
	int fd = open(filename,O_RDONLY);
	if(fd <=0)printf("cannot open file\n");
	send(connected,filename,100,0);
	int readVar = read(fd,send_data,1024);
	while(readVar > 0)
	{
		send_data[readVar]='\0';
		send(connected,send_data,strlen(send_data), 0);
		readVar = read(fd,send_data,1024);
		if(readVar<=0)
			break;
	}
	close(fd);
}

void indexGet(char str[])
{
	// Sample Query - IndexGet ShortList <starting-time-stamp> <ending-time-stamp>
	// find . -type f -newermt "2013-03-20" ! -newermt "2013-03-23" -ls  for ShortList
	// ls -lh for LongList
	int len=0,flagStyle=0;
	char startTime[20],endTime[20];
	memset(startTime,'\0',sizeof(startTime));
	memset(endTime,'\0',sizeof(endTime));
	char *token;
	token = strtok(str," ");
	while(token != NULL)
	{
		printf("token:%s\n",token);
		len++;
		if(len==2)
		{
			if(strcmp(token,"LongList")==0)
			{
				flagStyle=0;
				break;
			}
			else if(strcmp(token,"ShortList")==0)
				flagStyle=1;
		}
		if(flagStyle==1)
		{
			if(len==3)
				strcpy(startTime,token);
			if(len==4)
				strcpy(endTime,token);
		}
		token = strtok(NULL," ");
	}
	char **cmnd;
	if(flagStyle==0) // LongList - ls -lh
	{
		cmnd = (char **)malloc(sizeof(char *)*2);
		int i=0;
		for(i=0;i<2;i++)
			cmnd[i] = (char *)malloc(sizeof(char)*5);
		strcpy(cmnd[0],"ls");
		strcpy(cmnd[1],"-lh");
		execute(cmnd);
	}
	else if(flagStyle==1)  // find . -type f -newermt "2013-03-20" ! -newermt "2013-03-23" -ls
	{
		cmnd = (char **)malloc(sizeof(char *)*10);
		int i=0;
		for(i=0;i<10;i++)
		{
			cmnd[i] = (char *)malloc(sizeof(char)*20);
			memset(cmnd[i],'\0',sizeof(cmnd[i]));
		}
		strcpy(cmnd[0],"find");
		strcpy(cmnd[1],".");  // have to change this
		strcpy(cmnd[2],"-type");
		strcpy(cmnd[3],"f");
		strcpy(cmnd[4],"-newermt");
		strcpy(cmnd[5],startTime);
		strcpy(cmnd[6],"!");
		strcpy(cmnd[7],"-newermt");
		strcpy(cmnd[8],endTime);
		strcpy(cmnd[9],"-ls");
		//		for(i=0;i<10;i++)			printf("%s ",cmnd[i]);
		//		printf("\n");
		execute(cmnd);
	}
}

void  execute(char **cmnd)
{
	pid_t pid;
	int status;
	if( (pid = fork()) < 0)
	{     	
		printf("forking child process failed\n");
		_exit(1);
	}
	else if(pid == 0)
	{
		if(execvp(cmnd[0], cmnd) < 0)
		{
			printf("exec failed\n");
			_exit(1);
		}
	}
	else 
	{       /* for the parent:  */
		while (wait(&status) != pid);       /* wait for completion  */			
	}
}

void longList()
{
	indexGetArray = (char *)malloc(sizeof(char )*1000);
	memset(indexGetArray,'\0',sizeof(indexGetArray));
	char *dir;
	dir  = (char *)malloc(sizeof(char )*100);
	strcpy(dir,".");
	DIR *d = opendir(dir);
	if(d==NULL)
	{
		printf("Error.Could NOt open\n");
		return;
	}
	else
	{
		chdir(dir);
		while(1)
		{
			struct dirent *directry;
			directry = readdir(d);
			if(directry==NULL)
			{
				//printf(" Empty\n");
				break;
			}
			else
			{
				/* Calculation */
				struct stat info;
				lstat(directry->d_name,&info);
				int size = info.st_size;
				long int tme = info.st_mtime;
				char t[100]="";
				strftime(t, 100, "%d-%m-%Y %H:%M:%S", localtime( &info.st_mtime));

				/*  Printing */
				/*
				   printf("%s ",directry->d_name);
				   printf("%d ",size);
				//printf("%ld ",tme);
				printf("%s ",t);
				if(S_ISDIR(info.st_mode))
				printf("Directory\n");
				else
				printf("File\n");
				printf("\n");
				 */

				strcat(indexGetArray,"Name: ");
				strcat(indexGetArray,directry->d_name);
				strcat(indexGetArray,"\nSize: ");
				char tempS[10] = "";
				sprintf(tempS,"%d",size);
				strcat(indexGetArray,tempS);
				strcat(indexGetArray,"bytes\n Last Modified Time Stamp:");
				strcat(indexGetArray,t);
				strcat(indexGetArray,"\nType: ");
				//printf("%ld ",tme);
				//printf("%s ",t);
				if(S_ISDIR(info.st_mode))
					strcat(indexGetArray,"Directory\n\n");
				else if(S_ISREG(info.st_mode))
					strcat(indexGetArray,"Regular File\n\n");
				else if(S_ISBLK(info.st_mode))
					strcat(indexGetArray,"Block Device\n\n");
				else if(S_ISLNK(info.st_mode))
					strcat(indexGetArray,"Symbolic Link\n\n");
			}
		}
		//chdir("..");
		//closedir(dir);
	}
	//printf("%s",indexGetArray);
}

int checkShortList(char str1[] , char str2[] , char chk[])
{
	struct tm time1,time2,time3;

	strptime(str1, "%d-%m-%Y %H:%M:%S", &time1);
	strptime(str2, "%d-%m-%Y %H:%M:%S", &time2);
	strptime(chk, "%d-%m-%Y %H:%M:%S", &time3);
	time1.tm_isdst = -1;	time2.tm_isdst = -1;	time3.tm_isdst = -1;
	time_t timestamp1 = mktime(&time1);	time_t timestamp2 = mktime(&time2);	time_t timestamp3 = mktime(&time3);
	if(timestamp3<timestamp2)
	{
		if(timestamp3>timestamp1) 
			return 1;
	}
	return 0;
}

void shortList(char str1[],char str2[])
{
	//printf("%s\n",str1);
	//printf("%s\n",str2);
	indexGetArray = (char *)malloc(sizeof(char )*1000);
	memset(indexGetArray,'\0',sizeof(indexGetArray));
	char *dir;
	dir  = (char *)malloc(sizeof(char )*100);
	strcpy(dir,".");
	DIR *d = opendir(dir);
	if(d==NULL)
	{
		printf("error\n");
		return;
	}
	else
	{
		chdir(dir);
		while(1)
		{
			struct dirent *directry;
			directry = readdir(d);
			if(directry==NULL)
			{
				//printf(" Empty\n");
				break;
			}
			else
			{
				/* Calculation */
				struct stat info;
				lstat(directry->d_name,&info);  
				int size = info.st_size;// Size
				long int tme = info.st_mtime;  // TimeStamp
				char t[100]="";
				strftime(t, 100, "%d-%m-%Y %H:%M:%S", localtime( &info.st_mtime));

				/* Printing */
				if(checkShortList(str1,str2,t))
				{
					/*
					   printf("%s ",directry->d_name);
					   printf("%d ",size);
					//printf("%ld ",tme);
					printf("%s ",t);
					if(S_ISDIR(info.st_mode))
					printf("Directory\n");
					else
					printf("File\n");
					printf("\n");
					 */
					//printf("dfdsfdsf");
					strcat(indexGetArray,"Name: ");
					strcat(indexGetArray,directry->d_name);
					strcat(indexGetArray,"\nSize: ");
					char tempS[10] = "";
					sprintf(tempS,"%d",size);
					strcat(indexGetArray,tempS);
					strcat(indexGetArray," bytes\n Last Modified Time Stamp:");
					strcat(indexGetArray,t);
					strcat(indexGetArray,"\nType: ");
					//printf("%ld ",tme);
					//printf("%s ",t);
					if(S_ISDIR(info.st_mode))
						strcat(indexGetArray,"Directory\n\n");
					else if(S_ISREG(info.st_mode))
						strcat(indexGetArray,"Regular File\n\n");
					else if(S_ISBLK(info.st_mode))
						strcat(indexGetArray,"Block Device\n\n");
					else if(S_ISLNK(info.st_mode))
						strcat(indexGetArray,"Symbolic Link\n\n");
				}
			}
		}
		//chdir("..");
	}
	//printf("%s",indexGetArray);
}

void getInfo(char str11[])
{
	infoAbtFile = (char *)malloc(sizeof(char)*1000);
	memset(infoAbtFile,'\0',sizeof(infoAbtFile));
	struct stat sb;
	//printf("Hiiiiiiiiiiiiiiiiiiiiiiii\n");
	if (stat(str11, &sb) == -1)
	{
		perror("stat");
		exit(EXIT_FAILURE);
	}
	/*
	   printf("File name:                %s\n",str11);
	   printf("File size:                %lld bytes\n",(long long) sb.st_size);
	   printf("Last file modification:   %s", ctime(&sb.st_mtime));
	 */
	char st1[100]="" , st2[100]="" , st3[100]="";
	sprintf(st1,"File name:                %s\n",str11);
	sprintf(st2,"File size:                %lld bytes\n",(long long) sb.st_size);
	sprintf(st3,"Last file modification:   %s\n", ctime(&sb.st_mtime));
	strcat(infoAbtFile,st1);
	strcat(infoAbtFile,st2);
	strcat(infoAbtFile,st3);
	//printf("%s\n",infoAbtFile);
	return;
}

void longListRegEx(char regEx[])
{
	indexGetArray = (char *)malloc(sizeof(char )*10000);
	memset(indexGetArray,'\0',sizeof(indexGetArray));
	char *dir;
	dir  = (char *)malloc(sizeof(char )*100);
	memset(indexGetArray,'\0',sizeof(indexGetArray));
	strcpy(dir,".");
	DIR *d = opendir(dir);
	if(d==NULL)
	{
		printf("error\n");
		return;
	}
	else
	{
		chdir(dir);
		while(1)
		{
			struct dirent *directry;
			directry = readdir(d);
			if(directry==NULL)
			{
				//printf(" Empty\n");
				break;
			}
			else
			{
				/* Calculation */
				struct stat info;
				lstat(directry->d_name,&info);
				int size = info.st_size;
				long int tme = info.st_mtime;
				char t[100]="";
				strftime(t, 100, "%d-%m-%Y %H:%M:%S", localtime( &info.st_mtime));

				/*  Printing */
				/*
				   printf("%s ",directry->d_name);
				   printf("%d ",size);
				//printf("%ld ",tme);
				printf("%s ",t);
				if(S_ISDIR(info.st_mode))
				printf("Directory\n");
				else
				printf("File\n");
				printf("\n");
				 */

				//printf("regEx is %s\n",regEx);
				int flagReg = fnmatch(regEx,directry->d_name,FNM_NOESCAPE);
				if(flagReg==0)
				{
					strcat(indexGetArray,"Name: ");
					strcat(indexGetArray,directry->d_name);
					strcat(indexGetArray,"\nSize: ");
					char tempS[10] = "";
					sprintf(tempS,"%d",size);
					strcat(indexGetArray,tempS);
					strcat(indexGetArray,"bytes\n Last Modified Time Stamp:");
					strcat(indexGetArray,t);
					strcat(indexGetArray,"\nType: ");
					//printf("%ld ",tme);
					//printf("%s ",t);
					if(S_ISDIR(info.st_mode))
						strcat(indexGetArray,"Directory\n\n");
					else if(S_ISREG(info.st_mode))
						strcat(indexGetArray,"Regular File\n\n");
					else if(S_ISBLK(info.st_mode))
						strcat(indexGetArray,"Block Device\n\n");
					else if(S_ISLNK(info.st_mode))
						strcat(indexGetArray,"Symbolic Link\n\n");
				}
			}
		}
		//chdir("..");
	}
	//printf("%s\n",indexGetArray);
}
