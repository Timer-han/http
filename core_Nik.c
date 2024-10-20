#include <netdb.h>      // for gethostbyname
#include <netinet/in.h> // sockaddr_in
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // for socket
#include <sys/time.h>
#include <unistd.h> // for close

int hx_connect_v4(const char *ip, int port);
int SEND(int sock, char *mes, char *recvbuf);

int SEND(int sock, char *mes, char *recvbuf)
{
    int res;
    recvbuf[0] = 0;
    strcpy(recvbuf, "gt3H");
    strcat(recvbuf, mes);
    strcat(recvbuf, "\r\n");
    printf("%s\n\n", recvbuf);

    res = send(sock, recvbuf, strlen(recvbuf), 0);
    if (res != strlen(recvbuf))
    {
        printf("[-] Can't send data from server\n");
        close(sock);
        return -1;
    }
    res = recv(sock, recvbuf, 10240, 0);
    if (res <= 0)
    {
        printf("[-] Can't recv data from server\n");
        close(sock);
        return -1;
    }
    recvbuf[res] = 0;
    printf("%s\n\n", recvbuf);
    return 0;
}

int hx_connect_v4(const char *ip, int port)
{
    int sock, res;
    struct sockaddr_in addr;
    struct hostent *host = NULL;

    // fill structure for input
    memset(&addr, 0, sizeof(addr));
    host = gethostbyname(ip);
    if (host == NULL)
    {
        return -3;
    }
    // Write resolved IP address of a server to the address structure
    memcpy(&(addr.sin_addr.s_addr), host->h_addr_list[0], 4);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    // create socket for TCP connection
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("[+] Can't open sock\n");
        return -1;
    }

    // connect socket with the remote address
    res = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0)
    {
        printf("[+] Can't connect to sock\n");
        close(sock);
        return -2;
    }

    return sock;
}

int main()
{
    const char hostname[] = "exam.1434.ru";
    const unsigned short port = 63822;
    //     char sendbuf[1024] = "USER anonymous\r\n";
    //     char sendlength    = 87;
    char recvbuf[102400], recvbuf2[102400], file[10240] = "gt3H";
    int sock, sock2, res, buf1, buf2, n = -1;
    struct sockaddr_in addr;
    struct hostent *host = NULL;
    FILE *fin = fopen("core.c", "rb");
    sock = hx_connect_v4(hostname, port);

    // -- START SEND/RECV
    if (!fin)
    {
        close(sock);
        return -1;
    }
    while (!feof(fin))
    {
        file[strlen(file)] = fgetc(fin);
        if (file[strlen(file) - 1] == '\n')
        {
            strcat(file + strlen(file), "\r\n");
        }
    }
    file[strlen(file) - 1] = 0;
    printf("%s\n\n", file);
    rewind(fin);

    printf("[+] Connection successful to remote host\n");

    res = recv(sock, recvbuf, sizeof(recvbuf), 0);
    if (res <= 0)
    {
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

    if (SEND(sock, "USER Petrunnikov_TM", recvbuf) < 0)
        return -1;

    if (SEND(sock, "PASS QseeoZ8s2U", recvbuf) < 0)
        return -1;

    if (SEND(sock, "TYPE I", recvbuf) < 0)
        return -1;

    if (SEND(sock, "CWD ret/Vhxj4PzP3LmCDD/access", recvbuf) < 0)
        return -1;
    if (SEND(sock, "PWD", recvbuf) < 0)
        return -1;
    if (SEND(sock, "PASV", recvbuf) < 0)
        return -1;
    sscanf(recvbuf + 40, "%d,%d", &buf1, &buf2);
    printf("%d %d\n", buf1, buf2);
    sock2 = hx_connect_v4(hostname, buf1 * 256 + buf2);
    if (SEND(sock, "STOR core.c", recvbuf) < 0)
        return -1;

    if (send(sock2, file, strlen(file), 0) < 0)
    {
        printf("[-] Can't send data to server\n");
        close(sock2);
        close(sock);
        return -1;
    }
    printf("%s\n\n", recvbuf);
    close(sock2);

    res = recv(sock, recvbuf, 10240, 0);
    if (res <= 0)
    {
        printf("[-] Can't recv data from server\n");
        close(sock);
        return -1;
    }

    if (SEND(sock, "CWD /ret/Vhxj4PzP3LmCDD/unlock", recvbuf) < 0)
        return -1;
    if (SEND(sock, "PWD", recvbuf) < 0)
        return -1;
    if (SEND(sock, "PASV", recvbuf) < 0)
        return -1;
    sscanf(recvbuf + 40, "%d,%d", &buf1, &buf2);
    printf("%d %d\n", buf1, buf2);
    sock2 = hx_connect_v4(hostname, buf1 * 256 + buf2);
    if (SEND(sock, "STOR core.c", recvbuf) < 0)
        return -1;

    if (send(sock2, file, strlen(file), 0) < 0)
    {
        printf("[-] Can't send data to server\n");
        close(sock2);
        close(sock);
        return -1;
    }
    printf("%s\n\n", recvbuf);
    close(sock2);

    res = recv(sock, recvbuf, 10240, 0);
    if (res <= 0)
    {
        printf("[-] Can't recv data from server\n");
        close(sock);
        return -1;
    }

    printf("SUCCESS\n");

    fclose(fin);
    close(sock);
    close(sock2);
    return 0;
}
