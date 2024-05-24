#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

void work(int sock){
    char buf[10000];
    int r=recv(sock,buf,sizeof(buf)-1,0);
    if(r>0){
        buf[r]='\0';
        printf("recived: %s\n", buf);
    }
    send(sock,"HTTP/1.1 200 OK\r\n"
        "Content-Language: ru\r\n"
        "Content-Type: text/tml; charset=utf-8\r\n"
        "Content-Length: 15\r\n"
        "Connection: close\r\n"
        "\r\n"
        "HELLO FROM HELL\r\n",strlen("HTTP/1.1 200 OK\r\n"
        "Content-Language: ru\r\n"
        "Content-Type: text/tml; charset=utf-8\r\n"
        "Content-Length: 15\r\n"
        "Connection: close\r\n"
        "\r\n"
        "HELLO FROM HELL\r\n"),0);

}
int main (int, char**){
    const unsigned short port = 8085;
    int sock, sock_peer, ip_h;
    struct sockaddr_in addr, peer_addr;
    struct linger linger_opt = {1, 0};
    socklen_t peer_addr_size = sizeof(peer_addr);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("[-] Can't create socket\n");
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    if (bind(sock, (const struct sockaddr *)&addr, sizeof(addr)) != 0) {
        printf("[-] Can't bind socket to port\n");
        close(sock);
        return -1;
    }
    if(setsockopt(sock,SOL_SOCKET, SO_LINGER,&linger_opt, sizeof(linger_opt))!=0){
        printf("[-] Can't linger socket\n");
        close(sock);
        return -1;
    }
    if(listen (sock,2)!=0){
        printf("[-] Can't set listen queue for socket\n");
        close(sock);
        return -1;
    }
    printf("[+] Server listening on port %d\n",port);
    while(1){
        sock_peer = accept(sock,(struct sockaddr*) &peer_addr, &peer_addr_size);
        if(sock_peer <0 ){
            printf("[-] Can't accept new connection\n");
            close(sock);
            return -1;
        }
        ip_h=ntohl(peer_addr.sin_addr.s_addr);
        printf("[+] New connection accepted from %d. %d. %d. %d on port %d\n", (ip_h>>24) & 0xff, (ip_h>>16)&0xff,(ip_h>>8)&0xff, ip_h & 0xff, htons(peer_addr.sin_port));
        work(sock_peer);
    }
    close(sock);
    return 0;

}