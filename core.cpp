#include <netdb.h> // for gethostbyname
#include <netinet/in.h> // sockaddr_in
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // for socket
#include <sys/time.h>
#include <unistd.h> // for close
pthread_t threads[10];
pthread_t cleanup_thread_id;
int thread_count = 0;

void *cleanup_thread(void *arg);
int send_picture(int sock, const char *file);
void *work(void *socket);
int send_html(int sock, const char *file);

int send_html(int sock, const char *file)
{
    int n = -1;
    char buf[512];
    FILE *fin = fopen(file, "rb");

    if (!fin) {
        printf("[-] Can't open the image\n");
        return -1;
    }
    printf("[+] File opened\n");

    while (!feof(fin)) {
        fgetc(fin);
        n++;
    }
    printf("[+] size of \"%s\" is %d\n", file, n);
    rewind(fin);

    if (n < 0) {
        fclose(fin);
        return -1;
    }

    printf("[+] Trying to send the request\n");

    strcpy(buf, "HTTP/1.0 200 OK\r\n"
                "Content-Language: ru\r\n"
                "Content-Type: text/html; charset=utf-8");
    strcat(buf, "\r\n");
    strcat(buf, "Content length: ");
    sprintf(buf + strlen(buf), "%d", n);
    strcat(buf, "\r\n"
                "Connection: close\r\n"
                "\r\n");
    if (send(sock, buf, strlen(buf), 0) < 0) {
        printf("[-] Can't send data\n");
        fclose(fin);
        return -1;
    }

    printf("[+] Sending the file\n");

    for (int i = 0; i < n / 500; i++) {
        for (int j = 0; j < 500; j++) {
            buf[j] = fgetc(fin);
        }
        if (send(sock, buf, 500, 0) < 0) {
            printf("[-] Can't send data\n");
            fclose(fin);
            return -1;
        }
    }
    for (int j = 0; j < (n % 500); j++) {
        buf[j] = fgetc(fin);
    }
    if (send(sock, buf, n % 500, 0) < 0) {
        printf("[-] Can't send data\n");
        fclose(fin);
        return -1;
    }

    printf("[+] The file \"%s\" has been sent to server\n", file);
    fclose(fin);
    printf("[+] File closed\n");
    return 0;
}
int send_picture(int sock, const char *file)
{
    int k, n = -1;
    char typei[100];
    char buf[512];
    FILE *fin = fopen(file, "rb");
    typei[0] = 'i';
    typei[1] = 'm';
    typei[2] = 'a';
    typei[3] = 'g';
    typei[4] = 'e';
    typei[5] = 0;
    buf[0] = 0;
    if (strstr(file, ".png") != NULL) {
        strcat(typei, "/png");
    }
    if (strstr(file, ".jpg") != NULL) {
        strcat(typei, "/jpg");
    }
    if (strstr(file, ".jpeg") != NULL) {
        strcat(typei, "/jpg");
    }
    if (strstr(file, ".ico") != NULL) {
        strcat(typei, "/ico");
    }
    printf("type of picture:%s\n", typei);

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
    strcat(buf, typei);
    strcat(buf, "\r\n");
    strcat(buf, "Content length: ");
    sprintf(buf + strlen(buf), "%d", n);
    strcat(buf, "\r\n"
                "Connection: close\r\n"
                "\r\n");
    if (send(sock, buf, strlen(buf), 0) < 0) {
        printf("[-] Can't send data\n");
        fclose(fin);
        return -1;
    }

    printf("[+] Sending the file\n");

    for (int i = 0; i < n / 500; i++) {
        for (int j = 0; j < 500; j++) {
            buf[j] = fgetc(fin);
        }
        if (send(sock, buf, 500, 0) < 0) {
            printf("[-] Can't send data\n");
            fclose(fin);
            return -1;
        }
    }
    for (int j = 0; j < (n % 500); j++) {
        buf[j] = fgetc(fin);
    }
    if (send(sock, buf, n % 500, 0) < 0) {
        printf("[-] Can't send data\n");
        fclose(fin);
        return -1;
    }

    printf("[+] The file \"%s\" has been sent to client\n", file);
    fclose(fin);
    printf("[+] File closed\n");
    return 0;
}

int main(int, char **)
{
    const unsigned short port = 8087;
    int sock, *sock_peer, ip_h;
    pthread_t threads[10];
    int thread_count = 0;
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
    if (setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger_opt,
                   sizeof(linger_opt)) != 0) {
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
    // Создаем поток для очистки завершившихся потоков
    pthread_create(&cleanup_thread_id, NULL, cleanup_thread, NULL);

    while (1) {
        sock_peer = (int *)malloc(sizeof(int));
        sock_peer[0] =
            accept(sock, (struct sockaddr *)&peer_addr, &peer_addr_size);
        if (sock_peer[0] < 0) {
            printf("[-] Can't accept new connection\n");
            close(sock);
            return -1;
        }
        ip_h = ntohl(peer_addr.sin_addr.s_addr);

        printf("[+] New connection accepted from %d.%d.%d.%d on port %d\n",
               (ip_h >> 24) & 0xff, (ip_h >> 16) & 0xff, (ip_h >> 8) & 0xff,
               ip_h & 0xff, htons(peer_addr.sin_port));
        if (thread_count < 10) {
            pthread_create(&threads[thread_count], NULL, work,
                           (void *)sock_peer);
            thread_count++;
        } else {
            printf("[-] Too many active threads, rejecting new connection\n");
            close(sock_peer[0]);
            free(sock_peer);
        }
    }

    // Ожидание завершения всех потоков
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    close(sock);
    return 0;
}

void *work(void *socket)
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
    int r, sock = *((int *)socket);
    free(socket);
    printf("[+] Start to  work with sock %d\n", sock);

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


    printf("[+] Trying to send the file: %s\n", buf);
    if (!strcmp(buf, "/")) {
        send_html(sock, "./index.html");
    } else if (!strcmp(buf, "/1.jpg")) {
        send_picture(sock, "./1.jpg");
    } else if (!strcmp(buf, "/index.html")) {
        send_html(sock, "index.html");
    } else if (!strcmp(buf, "/2.jpeg")) {
        send_picture(sock, "./2.jpg");
    } else if (!strcmp(buf, "/favicon.ico")) {
    } else {
        send_picture(sock, "./404.png");
    }

    return NULL;
}

void *cleanup_thread(void *arg)
{
    int result;
    while (1) {
        for (int i = 0; i < 10; i++) {
            if (pthread_join(threads[i], (void **)&result) == 0) {
                for (int j = i; j < 10 - 1; j++) {
                    threads[j] = threads[j + 1];
                }
                thread_count--;
                break;
            }
        }
    }

    return NULL;
}
