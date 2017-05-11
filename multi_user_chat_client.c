#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_DATA 1024
//#define false 0
//#define true 1

void s_error(char *msg){
	perror(msg);
	exit(-1);
}

void *the_thread_fonsiyonu(void *);

char GLOBAL_input[MAX_DATA];

main(int argc, char *argv[])
{
	struct sockaddr_in client;
	int sckt, psckt, byte_len;
	int len = sizeof(struct sockaddr_in);
	char input[MAX_DATA];
	char output[MAX_DATA];
	bool is_exit = true;
	bool complete_exit = true;
	
	sckt = socket(AF_INET, SOCK_STREAM, 0);
	if(sckt == -1){
		s_error("Socket oluşturulamadı");
	}
	else{
		printf("Socket oluşturuldu.\n");
	}
	
	client.sin_family = AF_INET;
	client.sin_port = htons(atoi(argv[2]));
	client.sin_addr.s_addr = inet_addr(argv[1]);
	bzero(&client.sin_zero, 8);
	bzero(input, MAX_DATA - 1);
	bzero(output, MAX_DATA - 1);
	
	if((connect(sckt, (struct sockaddr *) &client, len)) == -1){
		s_error("Sunucuya bağlanılamadı.");
	}
	printf("Sunucuya bağlanılıyor...\n");
	
	printf("\nBalantı doğrulandı.");
	
	printf("\nBağlantıyı sonlandırmak için '&' giriniz.");

	recv(sckt, output, MAX_DATA, 0);
	printf("\n%s", output);
	
	pthread_t the_thread;
	if(pthread_create(&the_thread, NULL, the_thread_fonsiyonu, (void *) &sckt) < 0){
			s_error("Thread oluşturulamadı.");
		}


	while(is_exit)
		{
			
			bzero(output, MAX_DATA - 1);
			byte_len = recv(sckt, output, MAX_DATA, 0);
			if(byte_len == -1){
				s_error("Mesaj alınamıyor.");
			}
			else if(byte_len == 0){
				printf("Bağlantı kapatıldı.");
				is_exit = false;
				exit(1);
			}
			//output[strcspn(output, "\r\n")] = 0;
			printf("%s", output);
			
			if(*output == '&'){
				is_exit = false;
			}		
			if(*GLOBAL_input == '&'){
				is_exit = false;
			}	
			
		}
	
	printf("Oturum sonlandırıldı güle güle.");
	close(sckt);
	exit(1);
	
	
	
}



void *the_thread_fonsiyonu(void *arg)
{
	int tsckt = *(int *)arg;
	char input[MAX_DATA];
	bool is_exit=true;
	while(is_exit)
	{
		bzero(input, MAX_DATA -1);
		fgets(input, MAX_DATA, stdin);
		
		send(tsckt, input, strlen(input), 0);
		if(*input == '&'){
			is_exit=false;
		}
		strcpy(GLOBAL_input, input);
	}
	
	printf("Oturum sonlandırıldı güle güle.");
	close(tsckt);
	pthread_exit(0);
	
}