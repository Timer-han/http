#include <netdb.h>      // for gethostbyname
#include <netinet/in.h> // sockaddr_in
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // for socket
#include <sys/time.h>
#include <unistd.h> // for close

#include <dirent.h>
#include <math.h>
#include <sys/stat.h>

#include <pthread.h>

void *work(void *sok);
int SendFile(int sock, const char *file, const char *type);

int SendFile(int sock, const char *file, const char *type)
{
    int r, n = -1;
    char buf[512];
    FILE *fin = fopen(file, "rb");

    if (!fin) {
        fprintf(stderr, "[-] Can't open the image\n");
        return -1;
    }
    fprintf(stderr, "[+] File opened\n");

    while (!feof(fin)) {
        fgetc(fin);
        n++;
    }
    
    fprintf(stderr, "[+] size of \"%s\" is %d\n", file, n);
    rewind(fin);

    if (n < 0) {
        fclose(fin);
        return -1;
    }

    fprintf(stderr, "[+] Trying to send the request\n");

    strcpy(buf, "HTTP/1.0 200 OK\r\n"
                "Content-Language: ru\r\n"
                "Content-Type: ");
    strcat(buf, type);
    strcat(buf, "\r\n"
                "Content-Length: ");
    sprintf(buf + strlen(buf), "%d", n);

    strcat(buf, "\r\n"
                "Connection: close\r\n"
                "\r\n");
    if (send(sock, buf, strlen(buf), 0) < 0) {
        fprintf(stderr, "[-] Can't send data\n");
        fclose(fin);
        return -1;
    }

    fprintf(stderr, "[+] Sending the file\n");

    for (int i = 0; i < n / 500; i++) {
        for (int j = 0; j < 500; j++) {
            buf[j] = fgetc(fin);
        }
        if (send(sock, buf, 500, 0) < 0) {
            fprintf(stderr, "[-] Can't send data\n");
            fclose(fin);
            return -1;
        }
    }
    for (int j = 0; j < n % 500; j++) {
        buf[j] = fgetc(fin);
    }
    if (send(sock, buf, 500, 0) < 0) {
        fprintf(stderr, "[-] Can't send data\n");
        fclose(fin);
        return -1;
    }

    fprintf(stderr, "[+] The file \"%s\" has been sent to server\n", file);
    fclose(fin);
    fprintf(stderr, "[+] File closed\n");
    return 0;
}

int main(int, char **)
{
    const unsigned short port = 8083;
    int sock, *sock_peer, ip_h;
    struct sockaddr_in addr, peer_addr;
    struct linger linger_opt = {1, 0};
    socklen_t peer_addr_size = sizeof(peer_addr);
    pthread_t thread1;
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
    if (setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt)) != 0) {
        printf("[-] Can't linger socket\n");
        close(sock);
        return -1;
    }
    if (listen(sock, 2) != 0) {
        printf("[-] Can't set listen queue for socket\n");
        close(sock);
        return -1;
    }
    printf("[+] Server listening on port %d\n", port);
    while (1) {
        sock_peer = (int *) malloc(sizeof(int));
        sock_peer[0] = accept(sock, (struct sockaddr *)&peer_addr, &peer_addr_size);
        if (sock_peer[0] < 0) {
            printf("[-] Can't accept new connection\n");
            close(sock);
            return -1;
        }
        ip_h = ntohl(peer_addr.sin_addr.s_addr);

        printf("[+] New connection accepted from %d.%d.%d.%d on port %d\n", (ip_h >> 24) & 0xff, (ip_h >> 16) & 0xff,
               (ip_h >> 8) & 0xff, ip_h & 0xff, htons(peer_addr.sin_port));
        if ((int)((ip_h >> 24) & 0xff) == 14) {
            continue;
        }
        pthread_create(&thread1, NULL, work, (void *)(sock_peer));
    }
    close(sock);
    return 0;
}

void *work(void *sok)
{
    // ZDES CHTO DELAEM:
    // 1. recv
    // 2. разбираем буффер, который пришёл ввиде:
    //   GET /FILE.HTML HTTP/1.0
    //   ...
    //   ...
    // 3. Открываем указанный файл
    // 4. Формируем ответ
    // 5. Делаем send()
    char buf[1000];
    int r, sock = *((int *)sok);
    free(sok);
    fprintf(stderr,
            "--------------------------------------------------------[+] "
            "Starting work with sock %d\n",
            sock);

    r = recv(sock, buf, sizeof(buf) - 1, 0);
    if (r <= 3) {
        return NULL;
    }
    buf[r] = '\0';

    if (strncmp(buf, "GET", 3)) {
        return NULL;
    }

    printf("[+] Buf:\n%s\n", buf);
    strcpy(buf, buf + 4);
    for (size_t i = 0; i < strlen(buf); i++) {
        if (buf[i] == ' ' || buf[i] == '\n') {
            buf[i] = 0;
            break;
        }
    }

    printf("[+] Trying to answer on request: %s\n", buf);
    if (!strcmp(buf, "/")) {
        SendFile(sock, "./first_page/page.html", "text/html; charset=utf-8");
    }
    else if (!strcmp(buf, "/first_page/404.png")) {
        SendFile(sock, "./first_page/404.png", "image/png");
    }
    else if (!strcmp(buf, "/top5meme/page.html")) {
        SendFile(sock, "./top5meme/page.html", "text/html; charset=utf-8");
    }
    else if (!strcmp(buf, "/top5meme/1.png")) {
        SendFile(sock, "./top5meme/1.png", "image/png");
    }
    else if (!strcmp(buf, "/top5meme/2.jpg")) {
        SendFile(sock, "./top5meme/2.jpg", "image/jpg");
    }
    else if (!strcmp(buf, "/top5meme/3.jpg")) {
        SendFile(sock, "./top5meme/3.jpg", "image/jpg");
    }
    else if (!strcmp(buf, "/top5meme/4.png")) {
        SendFile(sock, "./top5meme/4.png", "image/png");
    }
    else if (!strcmp(buf, "/top5meme/5.jpg")) {
        SendFile(sock, "./top5meme/5.jpg", "image/jpg");
    }
    else if (!strcmp(buf, "/first_page/404.png")) {
        SendFile(sock, "./first_page/404.png", "image/png");
    }
    else if (!strcmp(buf, "/favicon.ico")) {
        SendFile(sock, "./favicon.ico", "image/ico");
    }
    else {
        SendFile(sock, "./first_page/404.png", "image/png");
    }
    fprintf(stderr,
            "--------------------------------------------------------[+] Ending "
            "work with sock %d\n",
            sock);
    return NULL;
}