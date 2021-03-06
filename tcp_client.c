#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>

#define portnumber 8633
#define MAX_BUF_SIZE 100
int get_local_ip(const char *eth_inf, char *ip, int sd);//获取本机ip地址
void *rec_data(void *fd);
void *delay_ban(void);
void rcal(const char *s);
void lcal(const char *s);
void mcal(const char *s);

struct infstr{
	char user_str[100];
	char ip_str[100];
	char passwd_str[100];
} inf;

struct sign_up{
	char user_sign[100];
	char passwd_sign[100];
	char email_sign[100];
} sign;

bool exist;
bool signfail;
bool perm;
bool ban;
bool isout;
const char welcome[] = "Welcome\0";
const char ad1[] = "***********\0";
const char ad2[] = "*********************\0";
const char ad3[] = "**************************************************************\0";
const char log_sign[] = "log in or sign up\0";
const char no_choice[] = "!!!No such a choice, please input again!!!\0";
const char sign_failed[] = "!!!The user has been signed!!!\0";
const char verif_failed[] = "!!!The verification code is wrong!!!\0";
const char log_failed[] = "!!!The user name or the password is not correct!!!\0";
const char log_succ[] = "Login successful, enjoy the chat\0";
const char sign_succ[] = "Sign up successful\0";
const char sign_after[] = "You can log in now or continue to sign up a new user\0";
const char conti[] = "continue\0";
const char no_user[] = "*no such a user*\0";
const char perm_y[] = "have permission\0";
const char perm_n[] = "have not permission\0";
const char stop[] = "******************stop talking in 30 seconds******************\0";
const char start[] = "***********************you can talk now***********************\0";
const char online_answer[] = "online answer\0";
const char ok[] = "ok\0";
const char ch[] = ":\0";
const char l[] = "(\0";
const char r[] = ")\0";
const char out[] = "out\0";
const char you_out[] = "you_out\0";

int main(int argc, char *argv[]){
	const char *eth_name = "wlp6s0";
	char ip[16];
	int sockfd;
	char model[100];
	char buffer[1024];
	char user[100];
	char passwd[100];
	char email[100];
	char verif_code[100];
	char send_msg[100];
	struct sockaddr_in server_addr;
	struct hostent *host;
	int nbytes;
	char hi[30] = "Hi, server\n";
	pthread_t thread;
	int i;
	
	if(argc != 2){
		fprintf(stderr, "Usage:%s hostname \a\n", argv[0]);
		exit(1);
	}
	
	if((host = gethostbyname(argv[1])) == NULL){
		fprintf(stderr, "Gethostname error\n");
		exit(1);
	}
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		fprintf(stderr, "Socket Error:%s\a\n", strerror(errno));
		exit(1);
	}
	
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnumber);
	
	server_addr.sin_addr = *((struct in_addr*)host->h_addr);
	
	if(connect(sockfd, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr)) == -1){
		fprintf(stderr, "Connnect Error:%s\a\n", strerror(errno));
		exit(1);
	}
	get_local_ip(eth_name, ip, sockfd);
	
	mcal(welcome);
	printf("%s\n", welcome);
	mcal(ad1);
	printf("%s\n", ad1);
	mcal(ad2);
	printf("%s\n", ad2);
	
	while(1){
		signfail = false;
		mcal(log_sign);
		printf("%s\n", log_sign);
		fgets(model, MAX_BUF_SIZE, stdin);
		model[strlen(model) - 1] = '\0';
		if(write(sockfd, model, sizeof(model)) == -1){
			fprintf(stderr, "a Write Error:%s\n", strerror(errno));
			exit(1);
		}
		usleep(100000);//100ms
		if(strcmp(model,"log in") == 0){
			printf("user:");
			fgets(user, MAX_BUF_SIZE, stdin);
			user[strlen(user) - 1] = '\0';
			
			printf("passwd:");
			fgets(passwd, MAX_BUF_SIZE, stdin);
			passwd[strlen(passwd) - 1] = '\0';
			
			
			strcpy(inf.user_str, user);
			strcpy(inf.ip_str, ip);
			strcpy(inf.passwd_str, passwd);
			//printf("writing\n");
			if(write(sockfd, &inf, sizeof(struct infstr)) == -1){
				fprintf(stderr, "b Write Error:%s\n", strerror(errno));
				exit(1);
			}
			//printf("write success\n");
			if((nbytes = read(sockfd, buffer, 1024)) == -1){
				fprintf(stderr, "Read Error:%s\n", strerror(errno));
				exit(1);
			}
			buffer[nbytes] = '\0';
			if(strcmp(buffer, log_succ) == 0){
				mcal(log_succ);
				printf("%s\n", log_succ);
				mcal(ad3);
				printf("%s\n", ad3);
				break;
			}
			else if(strcmp(buffer, log_failed) == 0){
				mcal(log_failed);
				printf("%s\n", log_failed);
				mcal(ad3);
				printf("%s\n", ad3);
			}
		}
		else if(strcmp(model,"sign up") == 0){
			printf("user:");
			fgets(user, MAX_BUF_SIZE, stdin);
			user[strlen(user) - 1] = '\0';
			strcpy(sign.user_sign, user);
			printf("passwd:");
			fgets(passwd, MAX_BUF_SIZE, stdin);
			passwd[strlen(passwd) - 1] = '\0';
			strcpy(sign.passwd_sign, passwd);
			printf("email:");
			fgets(email, MAX_BUF_SIZE, stdin);
			email[strlen(email) - 1] = '\0';
			strcpy(sign.email_sign, email);
			if(write(sockfd, &sign, sizeof(sign)) == -1){
				fprintf(stderr, "(sign)Write Error:%s\n", strerror(errno));
				exit(1);
			}
			
			if((nbytes = read(sockfd, buffer, sizeof(buffer))) == -1){
				fprintf(stderr, "Read Error:%s\n", strerror(errno));
				exit(1);
			}
			buffer[nbytes] = '\0';
			if(strcmp(buffer, sign_failed) == 0){
				mcal(sign_failed);
				printf("%s\n", sign_failed);
				mcal(ad3);
				printf("%s\n", ad3);
				continue;
			}
			else{
				printf("verif_code:");
				fgets(verif_code, MAX_BUF_SIZE, stdin);
				verif_code[strlen(verif_code) - 1] = '\0';
				if(write(sockfd, verif_code, sizeof(verif_code)) == -1){
					fprintf(stderr, "(sign)Write Error:%s\n", strerror(errno));
					exit(1);
				}
				if((nbytes = read(sockfd, buffer, sizeof(buffer))) == -1){
					fprintf(stderr, "Read Error:%s\n", strerror(errno));
					exit(1);
				}
				buffer[nbytes] = '\0';

				if(strcmp(buffer, sign_succ) == 0){
					mcal(sign_succ);
					printf("%s\n", sign_succ);
					mcal(sign_after);
					printf("%s\n", sign_after);
					mcal(ad3);
					printf("%s\n", ad3);
					continue;
				}
				else if(strcmp(buffer, verif_failed) == 0){
					mcal(verif_failed);
					printf("%s\n", verif_failed);
					mcal(ad3);
					printf("%s\n", ad3);
					continue;
				}
				
			}
		}
		
		else{
			mcal(no_choice);
			printf("%s\n", no_choice);
			mcal(ad3);
			printf("%s\n", ad3);
			memset(model, 0, sizeof(model));
		}
	}
	
	
	if(pthread_create(&thread, NULL, (void *)rec_data, &sockfd) != 0){
		fprintf(stderr, "Create thread Error:%s\n", strerror(errno));
		exit(1);
	}
	ban = false;
	while(1){
		//memset(send_msg, 0, sizeof(send_msg));
		exist = true;
		perm = false;
		isout = false;
		fflush(stdin);
		fgets(send_msg, MAX_BUF_SIZE, stdin);
		fflush(stdin);
		send_msg[strlen(send_msg) - 1] = '\0';
		if(isout){
			break;
		}
		if(!ban || strcmp(send_msg, "(end)")==0){
			if(write(sockfd, send_msg, sizeof(send_msg)) == -1){
				fprintf(stderr, "c Write Error:%s\n", strerror(errno));
				exit(1);
			}
		}
		
		//usleep(100000);//100ms
		if(strcmp(send_msg, "(end)")==0){
			break;
		}
		else if(!ban && strcmp(send_msg, "(sendTo)")==0){
			char self[100];
			char to[100];
			memset(self, 0, sizeof(self));
			memset(to, 0, sizeof(to));
			strcat(self, l);
			strcat(self, "me");
			strcat(self, r);
			strcat(self, ch);
			
			while(1){
				printf("user:");
				fgets(send_msg, MAX_BUF_SIZE, stdin);
				send_msg[strlen(send_msg) - 1] = '\0';
				strcpy(to, send_msg);
				if(write(sockfd, send_msg, sizeof(send_msg)) == -1){
					fprintf(stderr, "d Write Error:%s\n", strerror(errno));
					exit(1);
				}
				usleep(100000);
				if(exist){
					printf("message:");
					fgets(send_msg, MAX_BUF_SIZE, stdin);
					send_msg[strlen(send_msg) - 1] = '\0';
					if(write(sockfd, send_msg, sizeof(send_msg)) == -1){
						fprintf(stderr, "e Write Error:%s\n", strerror(errno));
						exit(1);
					}
					//printf("(private message)\n");
					
					strcat(self, send_msg);
					strcat(self, l);
					strcat(self, "to:");
					strcat(self, to);
					strcat(self, r);
					rcal(self);
					printf("%s\n",self);
					
					break;
				}
				else{
					printf("%s\n", no_user);
					break;
				}
			
			}
			
		}
		else if(!ban && strcmp(send_msg, "(banned)")==0){
			usleep(100000);
			if(perm){
				printf("user:");
				fgets(send_msg, MAX_BUF_SIZE, stdin);
				send_msg[strlen(send_msg) - 1] = '\0';
				if(write(sockfd, send_msg, sizeof(send_msg)) == -1){
					fprintf(stderr, "f Write Error:%s\n", strerror(errno));
					exit(1);
				}
				usleep(100000);
				if(exist){
					printf("(banned successful)\n");
				}
				else{
					printf("%s\n", no_user);
				}
			}
			else{
				printf("*You don't have the permission*\n");
			}
		
		}
		else if(!ban && strcmp(send_msg, "(out)")==0){
			usleep(100000);
			if(perm){
				printf("user:");
				fgets(send_msg, MAX_BUF_SIZE, stdin);
				send_msg[strlen(send_msg) - 1] = '\0';
				if(write(sockfd, send_msg, sizeof(send_msg)) == -1){
					fprintf(stderr, "f Write Error:%s\n", strerror(errno));
					exit(1);
				}
				usleep(100000);
				if(exist){
					printf("(out successful)\n");
				}
				else{
					printf("%s\n", no_user);
				}
			}
			else{
				printf("*You don't have the permission*\n");
			}
		}
		else if(!ban && strcmp(send_msg, "(online_list)") != 0){
			char self[100];
			memset(self, 0, sizeof(self));
			strcat(self, l);
			strcat(self, "me");
			strcat(self, r);
			strcat(self, ch);
			strcat(self, send_msg);
			rcal(self);
			printf("%s\n", self);
		}
		
	}

	pthread_detach(thread);//destory thread
	close(sockfd);
	exit(0);
}

// 获取本机ip
int get_local_ip(const char *eth_inf, char *ip, int sd)
{
	struct sockaddr_in sin;
	struct ifreq ifr;
 
	strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
	
	// if error: No such device
	if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
	{
		printf("ioctl error: %s\n", strerror(errno));
		close(sd);
		return -1;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	snprintf(ip, 16, "%s", inet_ntoa(sin.sin_addr));
	return 0;
}

void *rec_data(void *fd){
	int nbytes;
	int sock_fd;
	char rec_msg[100];
	sock_fd = *((int *)fd);
	//ban varible
	
	while(1){
		//memset(rec_msg, 0, sizeof(rec_msg));
		if((nbytes = read(sock_fd, rec_msg, sizeof(rec_msg))) == -1){
			fprintf(stderr, "Read Error:%s\n", strerror(errno));
			exit(1);
		}
		rec_msg[nbytes] = '\0';
		if(strcmp(rec_msg, no_user) == 0){
			exist = false;
			//printf("%s\n", rec_msg);
		}
		else if(strcmp(rec_msg, perm_y) == 0){
			perm = true;
		}
		else if(strcmp(rec_msg, perm_n) == 0){
			perm = false;
		}
		else if(strcmp(rec_msg, stop) == 0){
			mcal(stop);
			printf("%s\n", stop);
			ban = true;
			pthread_t thread_new;
			if(pthread_create(&thread_new, NULL, (void *)delay_ban, NULL) != 0){
				fprintf(stderr, "Create thread Error:%s\n", strerror(errno));
				exit(1);
			}
		}
		else if(strcmp(rec_msg, out) == 0){
			if(write(sock_fd, you_out, sizeof(you_out)) == -1){
				fprintf(stderr, "f Write Error:%s\n", strerror(errno));
				exit(1);
			}
			isout = true;
			pthread_exit(NULL);
		}
		else{
			if(rec_msg[strlen(rec_msg) - 1] == '@'){
				rec_msg[strlen(rec_msg) - 1] = '\0';
				printf("%s\n", rec_msg);
			}
			else{
				lcal(rec_msg);
				printf("%s\n", rec_msg);
			}
		}
		
	}
	//free(fd);
	//close(sock_fd);
	pthread_exit(NULL);
	
}

void *delay_ban(){
	sleep(30);
	ban = false;
	mcal(start);
	printf("%s\n", start);
	pthread_exit(NULL);
}

void rcal(const char *s){
	struct winsize w;
	int len, mid;
	int i;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	
	len = strlen(s);
	mid = w.ws_col / 2;
	for(i = 0; i < mid + 3; i++){
		printf(" ");
	}
}

void lcal(const char *s){
	struct winsize w;
	int len, mid;
	int i;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	
	len = strlen(s);
	mid = w.ws_col / 2;
	for(i = 0; i < mid - 20; i++){
		printf(" ");
	}
}

void mcal(const char *s){
	struct winsize w;
	int len, alen;
	int i;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	
	len = strlen(s);
	alen = w.ws_col;
	for(i = 0; i < (alen - len) / 2; i++){
		printf(" ");
	}
}

