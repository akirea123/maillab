#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095
#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF)) //为16位数据交换大小端

char buf[MAX_SIZE+1];

// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "smtp.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "3202653618"; // TODO: Specify the user
    const char* pass = "dgswnezyiffqdceh"; // TODO: Specify the password
    const char* from = "3202653618@qq.com"; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
    //创建socket
    s_fd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = swap16(port);
    sockaddr.sin_addr.s_addr = inet_addr(dest_ip);
    bzero(&sockaddr.sin_zero,sizeof(sockaddr.sin_zero));
    connect(s_fd, (struct sockaddr *)&sockaddr,sizeof(sockaddr));

    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // Send EHLO command and print server response
    const char* EHLO = "EHLO qq.com\r\n";
    send(s_fd, (void *)EHLO, strlen(EHLO), 0);
    printf("%s",EHLO);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
    
    // TODO: Print server response to EHLO command
    const char* AUTH = "AUTH login\r\n";
    send(s_fd, (void *)AUTH, strlen(AUTH), 0);
    printf("%s",AUTH);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);

    // TODO: Authentication. Server response should be printed out.
    char *userbase64 = encode_str(user); 
    strcat(userbase64, "\r\n");
    send(s_fd, (void *)userbase64, strlen(userbase64), 0);
    printf("%s",userbase64);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
    free(userbase64);

    char *passbase64 = encode_str(pass); 
    strcat(passbase64, "\r\n");
    send(s_fd, (void *)passbase64, strlen(passbase64), 0);
    printf("%s",passbase64);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
    free(passbase64);

    // TODO: Send MAIL FROM command and print server response
    char MAIL_FROM[MAX_SIZE+1];
    sprintf(MAIL_FROM,"MAIL FROM:<%s>\r\n",from);
    send(s_fd, (void *)MAIL_FROM, strlen(MAIL_FROM), 0);
    printf("%s",MAIL_FROM);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);

    // TODO: Send RCPT TO command and print server response
    char RCPT_TO[MAX_SIZE+1];
    sprintf(RCPT_TO,"RCPT TO:<%s>\r\n",receiver);
    send(s_fd, (void *)RCPT_TO, strlen(RCPT_TO), 0);
    printf("%s",RCPT_TO);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);

    // TODO: Send DATA command and print server response
    const char *data = "data\r\n";
    send(s_fd, (void *)data, strlen(data), 0);
    printf("%s",data);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);

    sprintf(buf, "From: %s\r\nTo: %s\r\nContent-Type: multipart/mixed; boundary=qwertyuiopasdfghjklzxcvbnm\r\n", from, receiver);
    if(subject != NULL) strcat(buf, "Subject: "), strcat(buf, subject), strcat(buf, "\r\n\r\n");
    send(s_fd, (void *)buf, strlen(buf), 0);
    printf("%s",buf);    

    if(msg !=NULL)
    {
        //如果输入文件路径
        if(access(msg,0)==0)
        {
            FILE *fp = NULL;
            fp = fopen(msg,"r");
            fseek(fp,0,SEEK_END);
            int cnt1 =ftell(fp);
            fseek(fp,0,SEEK_SET);
            char *messcontain = (char *)malloc(cnt1);
            fread(messcontain,sizeof(char),cnt1,fp);
            fclose(fp);
            sprintf(buf, "--qwertyuiopasdfghjklzxcvbnm\r\nContent-Type:text/plain\r\n\r\n");
            send(s_fd, (void *)buf, strlen(buf), 0);
            send(s_fd, (void *)messcontain, strlen(messcontain), 0);
            free(messcontain);
            printf("%s",buf);
            printf("%s",messcontain);  
        }
        //直接输入文本
        else
        {
            sprintf(buf,"--qwertyuiopasdfghjklzxcvbnm\r\nContent-Type:text/plain\r\n\r\n");
            send(s_fd, (void *)buf, strlen(buf), 0);
            send(s_fd, (void *)msg, strlen(msg), 0);
            printf("%s",buf);
            printf("%s",msg);
        }
        send(s_fd, "\r\n", 2, 0);
        printf("\r\n");
    }
    
    if(att_path !=NULL)
    {
        sprintf(buf, "--qwertyuiopasdfghjklzxcvbnm\r\nContent-Type:application/octet-stream\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment; name=%s\r\n\r\n",att_path);
        send(s_fd, (void *)buf, strlen(buf), 0);
        printf("%s",buf);
        FILE *fp1 = fopen(att_path,"r");
        if(fp1 == NULL)
        {
            perror("file not exist");
            exit(EXIT_FAILURE);
        }    
        FILE *fp2 = fopen("attbase64.temp","w+");
        encode_file(fp1,fp2);
        fseek(fp2,0,SEEK_END);
        int cnt2 =ftell(fp2);
        fseek(fp2,0,SEEK_SET);
        char *attbase64 = (char *)malloc(cnt2);
        fread(attbase64,sizeof(char),cnt2,fp2);
        fclose(fp1);
        fclose(fp2);
        send(s_fd, (void *)attbase64, strlen(attbase64), 0);
        free(attbase64);
    }   
    sprintf(buf, "--qwertyuiopasdfghjklzxcvbnm\r\n");
    send(s_fd, (void *)buf, strlen(buf), 0);
    printf("%s",buf); 
    send(s_fd, (void *)end_msg, strlen(end_msg), 0);
    printf("%s",end_msg);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
    
    // TODO: Send QUIT command and print server response
    const char *quit = "quit\r\n";
    send(s_fd, (void *)quit, strlen(quit), 0);
    printf("%s",quit);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}
