// Bir MULTI-USER ECHO TCP CHAT SERVER. 
// Sanki bir oda oluşturulmuş konuşan, odadaki herkese konuşuyormuşcasına.

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_DATA 1024
#define MAX_CLNT 10
//#define false 0
//#define true 1

//Prototipler
void *thread_bas_alici(void *);
void *thread_verici(void *);
void s_error(char *msg){
	perror(msg);
	exit(-1);
}

//thread argümanı için struct
struct the_thread_struct{
	int tsckt, tsckt_id;
	char nick[12];
};

//Public olarak kullanılması gereken değişkenler
int s = 0;
int sockf_a[MAX_CLNT];
char fd_a[MAX_CLNT][12];
//------------------------------------------------------------------------------------------------------------------//
main(int argc, char *argv[])
{
	
	// ****************  Standart socket TCP socket server **************** //
	int sckt, psckt;
	struct sockaddr_in server, client;
	int len = sizeof(struct sockaddr_in);
	char input[MAX_DATA];
	char input2[MAX_DATA];
	int i = 0;
	bool is_exit = true;
	bool complete_exit = true;
	
	sckt = socket(AF_INET, SOCK_STREAM, 0);
	if(sckt == -1)
	{
		s_error("Socket oluşturulamadı");
	}
	else
	{
		printf("Socket oluşturuldu.\n");
	}
	
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&server.sin_zero, 8);
	
	if((bind(sckt, (struct sockaddr *) &server, len)) == -1)
	{
		s_error("Bind hatası.");
	}
	
	listen(sckt, MAX_CLNT);	
	// ****************  Standart socket TCP socket server **************** //
	//------------------------------------------------------------------------------------------------------------------//
	printf("Bağlantı bekleniyor...\n");
	pthread_t thread_id[MAX_CLNT];
	struct the_thread_struct thread_struct[MAX_CLNT];

	while(s<MAX_CLNT)
	{

		psckt = accept(sckt, (struct sockaddr *) &client, &len); // Burada bekler yeni bağlantı için
		
		if( psckt == -1)
		{
			is_exit = false;
			s_error("Bağlantı kabul hatası.");
		}
		
		// ****************  Severa bağlanan yeni kullanıcı nick belirleme işlemi **************** //
		printf("\n%s ip adresli yeni client %d nolu porttan bağlantı kurdu.\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		bzero(input, MAX_DATA - 1);
		strcpy(input, "Lütfen en fazla 12 karakter den oluşan bir nick yazınız: ");
		send(psckt, input, strlen(input), 0);
		bzero(input, MAX_DATA - 1);
		int nick_len;
		nick_len=recv(psckt, input, MAX_DATA, 0);
		printf("Yeni kullanıcı: %s\n", input);

		char nick2[12];
		strncpy(nick2, input, 12);
		//printf("Nick2 kullanıcı: %s\n", nick2);
		//printf("byte len: %d\n", nick_len);
		//nick2[nick_len-1] = '\0';
		//nick2[nick_len-2] = '\0';
		nick2[strcspn(nick2, "\r\n")] = 0;
		printf("Yeni kullanıcı: %s \n", nick2);
		strcpy(input, nick2);
		// ****************  Severa bağlanan yeni kullanıcı nick belirleme işlemi **************** //
		//------------------------------------------------------------------------------------------------------------------//
		// ****************  Thread'e atılacak argümanı doldurma global değişken düzenleme **************** //
		for(int j = 0; j<MAX_CLNT;j++)
		{
			if(sockf_a[j] == NULL || sockf_a[j] <= 0){
				sockf_a[j] = psckt;	
				thread_struct[j].tsckt = psckt;
				thread_struct[j].tsckt_id = j;
				strncpy(thread_struct[j].nick, input, 12);
				strncpy(fd_a[j], input, 12);
				i = j;
				break;
			}
			else if(j == MAX_CLNT-1){
				printf("Daha fazla yer yok kış kış.");
				i = -1;
				close(psckt);
				break;
			}
			i = -1;
		}
		// ****************  Thread'e atılacak argümanı doldurma global değişken düzenleme **************** //
		//------------------------------------------------------------------------------------------------------------------//
		// ****************  Bağlı olanlara yeni kullanıcı giriş yaptı bilgisi **************** //
		if(i >= -1)
		{
			bzero(input2, MAX_DATA - 1);
			sprintf(input2, "Yeni kullanıcı bağlandı: %s\n", input);
			
			for(int j = 0; j< MAX_CLNT ; j++)
			{
				if(sockf_a[j] != psckt && sockf_a[j] != NULL || sockf_a[j] <= 0){
					send(sockf_a[j], input2, strlen(input2), 0);
				}			
			}			
		}
		// ****************  Bağlı olanlara yeni kullanıcı giriş yaptı bilgisi **************** //
		
		if( pthread_create( &thread_id[i] , NULL ,  thread_bas_alici , &thread_struct[i]) < 0)
        {
            s_error("Thread oluşturulamadı.");
        }
			
	
	}
	
	printf("SERVER KAPATILIYOR! GÜLE GÜLE.");
	for(int j = 0; j< MAX_CLNT ; j++)
	{
		if(sockf_a[j] != NULL && sockf_a[j] > 0)
		{
			close(sockf_a[j]);
		}
	}
	for(int j=0;j<MAX_CLNT;j++){
		pthread_exit(&thread_id[j]);
	}
	
	close(sckt);
	
}

//------------------------------------------------------------------------------------------------------------------//
// ****************  Client'dan alır odadaki herkese echo olarak iletir **************** //
void *thread_bas_alici(void *sockt)
{
	s++;
	
	struct the_thread_struct *arg_thread_struct = (struct the_thread_struct*) sockt;
	int asckt = arg_thread_struct->tsckt;
	printf("Fd = %d\n", asckt);
	int asckt_id = arg_thread_struct->tsckt_id;
	char a_nick[12];

	strncpy(a_nick, arg_thread_struct->nick, 12);

	char output[MAX_DATA];
	char copy_output[MAX_DATA];
	char output2[MAX_DATA];
	bool is_exit=true;
	int byte_len;
	
	
	bzero(output, MAX_DATA - 1);
	bzero(copy_output, MAX_DATA - 1);

	sprintf(output, "\n%s sunucuya bağlandın. Artık odadaki herkesle konuşabilirsin.\n", arg_thread_struct->nick);
	send(arg_thread_struct->tsckt, output, strlen(output), 0);
	
	while(is_exit || s < 1000)
	{
		bzero(output, MAX_DATA - 1);
		byte_len = recv(arg_thread_struct->tsckt, output, MAX_DATA, 0);
			
		if(byte_len == -1){
			s_error("Mesaj alınamıyor.");
		}
		else if(byte_len == 0){
			printf("\nKullanıcı çıkış yaptı.");
			is_exit = false;
			break;
		}
		strcpy(copy_output, output);
		output[strcspn(output, "\r\n")] = '\0';	
		printf("\n%s - %d: %s", arg_thread_struct->nick, arg_thread_struct->tsckt_id, copy_output);
		bzero(output2, MAX_DATA - 1);
		sprintf(output2, "\n%s: %s", arg_thread_struct->nick, copy_output);	
		for(int j = 0; j< MAX_CLNT ; j++)
		{
			if(sockf_a[j] != arg_thread_struct->tsckt && sockf_a[j] != NULL && sockf_a[j] > 0){
			send(sockf_a[j], output2, strlen(output2), 0);
			}
		}
						
	}
	
	//------------------------------------------------------------------------------------------------------------------//
	// ****************  Çıkış yapan kullanıcı bilgilendirimesi ve çıkış işlemleri **************** //
	bzero(output, MAX_DATA - 1);
	bzero(output2, MAX_DATA - 1);
	sprintf(output2, "%s kullanıcısı çıkış yaptı.\n", arg_thread_struct->nick);
	for(int j = 0; j< MAX_CLNT ; j++)
	{
		if(sockf_a[j] != arg_thread_struct->tsckt && sockf_a[j] != NULL && sockf_a[j] > 0){
			send(sockf_a[j], output2, strlen(output2), 0);
		}			
	}
	
	printf("%s Oturum sonlandırıldı güle güle.\n", output2);
	s--;
	close(arg_thread_struct->tsckt);
	sockf_a[arg_thread_struct->tsckt_id] = 0;
	bzero(fd_a[arg_thread_struct->tsckt_id], 12);
	bzero(arg_thread_struct, sizeof(arg_thread_struct));
	pthread_exit(0);
	// ****************  Çıkış yapan kullanıcı bilgilendirimesi ve çıkış işlemleri **************** //
	
}

//------------------------------------------------------------------------------------------------------------------//