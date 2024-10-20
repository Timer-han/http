#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <sys/socket.h> // for socket
#include <netinet/in.h> // sockaddr_in 
#include <unistd.h> // for close
#include <netdb.h> // for gethostbyname

int main() {
    const char hostname[] = "exam.1434.ru";
    const unsigned short port = 63821;
//     char sendbuf[1024] = "USER anonymous\r\n";
//     char sendlength    = 87;
    char recvbuf[102400];
    int sock, sock2, res, buf1, buf2;
    struct sockaddr_in addr;
    struct hostent *host = NULL;
    FILE * fin = fopen("output.txt", "wt");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("[-] Can't create socket\n");
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    host = gethostbyname(hostname);
    if (host == NULL) {
        printf("[-] Can't resolve hostname\n");
        close(sock);
        return -1;
    }
    memcpy(&(addr.sin_addr.s_addr), host->h_addr_list[0], 4);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;  
    res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if ( res < 0 ) {
        printf("[-] Can't connect to server\n");
        close(sock);
        return -1; 
    }

    // -- START SEND/RECV

    printf("[+] Connection successful to remote host\n");

    res = recv(sock, recvbuf, sizeof(recvbuf), 0);
    if(res <= 0 ) {
        printf("[-] Can't recv data from server\n");
        close(sock);
        return -1; 
    }
//     if (res != sendlength) {
//         printf("[-] Recved not the same amount of bytes\n");
//         close(sock);
//         return -1;
//     }
    printf("%d\n%s\n", res, recvbuf);
    
    res = send(sock, "USER anonymous\r\n", 16, 0);
    if(res != 16 ) {
        printf("[-] Can't send data from server\n");
        close(sock);
        return -1; 
    }
    res = recv(sock, recvbuf, 1024, 0);
    if(res <= 0 ) {
        printf("[-] Can't recv data from server\n");
        close(sock);
        return -1; 
    }
    printf("%d\n%s\n", res, recvbuf);
    
    
    res = send(sock, "TYPE I\r\n", 8, 0);
    if(res != 8) {
        printf("[-] Can't send data from server\n");
        close(sock);
        return -1; 
    }
    res = recv(sock, recvbuf, 1024, 0);
    if(res <= 0) {
        printf("[-] Can't recv data from server\n");
        close(sock);
        return -1; 
    }
    printf("%d\n%s\n", res, recvbuf);
    
    
    res = send(sock, "PASV\r\n", 6, 0);
    if(res != 6) {
        printf("[-] Can't send data from server\n");
        close(sock);
        return -1; 
    }
    res = recv(sock, recvbuf, 1024, 0);
    if(res <= 0) {
        printf("[-] Can't recv data from server\n");
        close(sock);
        return -1; 
    }
    printf("%d\n%s\n", res, recvbuf);
    
    
    sscanf(recvbuf + 40, "%d%c%d", &buf1, recvbuf, &buf2);
    printf("%d %d\n", buf1, buf2);

    
    sock2 = socket(AF_INET, SOCK_STREAM, 0);
    if (sock2 < 0) {
        printf("[-] Can't create socket\n");
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    host = gethostbyname("91.77.162.35");
    if (host == NULL) {
        printf("[-] Can't resolve hostname\n");
        close(sock2);
        return -1;
    }
    memcpy(&(addr.sin_addr.s_addr), host->h_addr_list[0], 4);
    addr.sin_port = htons(256 * buf1 + buf2);
    addr.sin_family = AF_INET;  
    res = connect(sock2, (struct sockaddr*)&addr, sizeof(addr));
    if ( res < 0 ) {
        printf("[-] Can't connect to server\n");
        close(sock2);
        return -1; 
    }

    
    res = send(sock, "RETR g59sOa.dat\r\n", 17, 0);
    if(res != 17) {
        printf("[-] Can't send data from server\n");
        close(sock);
        close(sock2);
        return -1; 
    }
    res = 1;
    while (res > 0){
        res = recv(sock2, recvbuf, 102400, 0);
        recvbuf[res] = 0;
        fprintf(fin, "%s", recvbuf);
    }
    


    printf("SUCCESS\n");
    
    fclose(fin);
    close(sock);
    close(sock2);
    return 0;
}

 
 
