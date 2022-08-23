#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535
#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF)) //为16位数据交换大小端

char buf[MAX_SIZE+1];

void recv_mail()
{
    const char* host_name = "pop.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = "3202653618@qq.com"; // TODO: Specify the user
    const char* pass = "dgswnezyiffqdceh"; // TODO: Specify the password
    char dest_ip[16];
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

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
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

    // TODO: Send user and password and print server response
    sprintf(buf ,"USER %s\r\n",user);
    send(s_fd, (void *)buf, strlen(buf), 0);
    printf("%s",buf);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);

    sprintf(buf ,"PASS %s\r\n",pass);
    send(s_fd, (void *)buf, strlen(buf), 0);
    printf("%s",buf);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);

    // TODO: Send STAT command and print server response
    sprintf(buf ,"STAT\r\n");
    send(s_fd, (void *)buf, strlen(buf), 0);
    printf("%s",buf);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
    int num_mail = -1, total_size_mail = -1;
    sscanf(buf, "+OK %d %d", &num_mail, &total_size_mail);

    // TODO: Send LIST command and print server response
    sprintf(buf ,"LIST\r\n");
    send(s_fd, (void *)buf, strlen(buf), 0);
    printf("%s",buf);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);

    // TODO: Retrieve the first mail and print its content
    sprintf(buf ,"RETR 1\r\n");
    send(s_fd, (void *)buf, strlen(buf), 0);
    printf("%s",buf);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
    //(+OK 四个字节后为大小)
    int total_size = atoi(buf + 4);
    total_size -= r_size;
    while(total_size > 0){
        if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        buf[r_size] = '\0';
        printf("%s", buf);
        total_size -= r_size;
    }

    // TODO: Send QUIT command and print server response
    sprintf(buf ,"QUIT\r\n");
    send(s_fd, (void *)buf, strlen(buf), 0);
    printf("%s",buf);
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
    recv_mail();
    exit(0);
}
