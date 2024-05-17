#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <sys/socket.h> // for socket
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // for close
#include <netdb.h>      // for gethostbyname

#include <dirent.h>
#include <math.h>
#include <sys/stat.h>


void work(int sock);
int readfile(const char *fname, char *buf, size_t n, const char *mode);
// int find(char *name, unsigned char *** images, int * n, int * k);

// //Функция для обхота всего дерева каталогов ________________________________________________
// int find(char *name, unsigned char *** images, int * n, int * k) {
//     char fname[256];
//     char c[256];
//     DIR *d = opendir(name);
//     struct dirent *item;
//     int i, j;
//     if (!d) {
//         return -1;
//     }

//     while ((item = readdir(d))) {
//         // MORE MEMORY \____________________________________________________________________
//         if (*n >= *k - 1) {
//             *k *= 2;
//             images[0] = (unsigned char **) realloc(images[0], *k * sizeof(unsigned char *));
//             if (!images[0]) {
//                 closedir(d);
//                 return -1;//Функция для обхота всего дерева каталогов ________________________________________________
// int find(char *name, unsigned char *** images, int * n, int * k) {
//     char fname[256];
//     char c[256];
//     DIR *d = opendir(name);
//     struct dirent *item;
//     int i, j;
//     if (!d) {
//         return -1;
//     }

//     while ((item = readdir(d))) {
//         // MORE MEMORY \____________________________________________________________________
//         if (*n >= *k - 1) {
//             *k *= 2;
//             images[0] = (unsigned char **) realloc(images[0], *k * sizeof(unsigned char *));
//             if (!images[0]) {
//                 closedir(d);
//                 return -1;
//             }
//             for (i = *k / 2; i < *k; i++) {
//                 images[0][i] = (unsigned char *) malloc(67108864 * sizeof(unsigned char));
//                 if (!images[0][i]) {
//                     for (j = *k / 2; j < i; j++) {
//                         free(images[0][j]);
//                     }
//                     *k /= 2;
//                     closedir(d);
//                     return -1;
//                 }
//             }

//         }
//         //__________________________________________________________________________
//         if (item->d_name[0] == '.') {
//             continue;
//         }
//         if (item->d_type == DT_DIR) {
//             sprintf(c, "%s/%s", name, item->d_name);
//             printf("%s\n", c);
//             find(c, images, n, k);
//         }
//         if (item->d_type == DT_REG) {
//             sprintf(fname, "%s/%s", name, item->d_name);
//             if (jpeeg(fname, images[0][*n]) == 1) {
//                 n[0]++;
//             }
//         }

//     }
//     closedir(d);

//     return 0;
// }

//             }
//             for (i = *k / 2; i < *k; i++) {
//                 images[0][i] = (unsigned char *) malloc(67108864 * sizeof(unsigned char));
//                 if (!images[0][i]) {
//                     for (j = *k / 2; j < i; j++) {
//                         free(images[0][j]);
//                     }
//                     *k /= 2;
//                     closedir(d);
//                     return -1;
//                 }
//             }

//         }
//         //__________________________________________________________________________
//         if (item->d_name[0] == '.') {
//             continue;
//         }
//         if (item->d_type == DT_DIR) {
//             sprintf(c, "%s/%s", name, item->d_name);
//             printf("%s\n", c);
//             find(c, images, n, k);
//         }
//         if (item->d_type == DT_REG) {
//             sprintf(fname, "%s/%s", name, item->d_name);
//             if (jpeeg(fname, images[0][*n]) == 1) {
//                 n[0]++;
//             }
//         }

//     }
//     closedir(d);

//     return 0;
// }


int findStr(char *buf, const char *src, size_t n, size_t m)
{
    for (size_t i = 0; i < n - m; i++)
    {
        if (!strncmp(buf + i, src, m))
        {
            return i;
        }
    }
    return -1;
}

int readfile(const char *fname, char *buf, size_t n, const char *mode)
{
    FILE *fin = fopen(fname, mode);
    int i = 0;
    if (!fin)
    {
        fprintf(stderr, "[-] Can't open the image\n");
        return -1;
    }
    fprintf(stderr, "[+] File opened\n");

    while (!feof(fin))
    {
        buf[i++] = fgetc(fin);
        if (i > n + 4){
            fprintf(stderr, "[-] Not enoufh mem\n");
            return -2;
        }
    }
    fprintf(stderr, "[+] File copied into buffer\n");

    fclose(fin);
    fprintf(stderr, "[+] File closed\n");
    return i - 1;
}

int SendPage(int sock, const char *file)
{
    int r, n;
    char buf[110000], buf2[100000], buf3[100];
    buf[0] = 0;
    buf2[0] = 0;
    r = readfile(file, buf2, sizeof(buf2) - 4, "rt");

    if (r < 0)
    {
        fprintf(stderr, "[-] Can't read the file!\n");

        return -1;
    }

    strcat(buf, "HTTP/1.0 200 OK\r\n"
                "Content-Language: ru\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Content-Length: ");

    sprintf(buf3, "%d", r);

    strcat(buf, buf3);
    strcat(buf, "\r\n"
                "Connection: close\r\n"
                "\r\n");

    fprintf(stderr, "[+] Buffer is ready to sending\n");

    n = send(sock, buf, strlen(buf), 0);
    if (n < 0)
    {
        return -1;
    }
    n = send(sock, buf2, r, 0);
    if (n < 0)
    {
        return -1;
    }

    fprintf(stderr, "[+] The file \"%s\" has been sent to server\n", file);

    return 0;
}

int SendImage(int sock, const char *file)
{
    int r, n;
    char buf[1000], buf2[1000000], buf3[200];
    buf[0] = 0;
    buf2[0] = 0;
    r = readfile(file, buf2, sizeof(buf2) - 4, "rb+");
    if (r < 0)
    {
        return -1;
    }

    strcat(buf, "HTTP/1.0 200 OK\r\n"
                "Content-Language: ru\r\n"
                "Content-Type: image/png\r\n"
                "Content-Length: ");

    sprintf(buf3, "%d\r\n", r);

    strcat(buf, buf3);
    strcat(buf, "Connection: close\r\n"
                "\r\n");
    printf("%s\n\n", buf);

    fprintf(stderr, "[+] Buffer is ready to sending\n");

    n = send(sock, buf, strlen(buf), 0);
    if (n < 0)
    {
        return -1;
    }
    n = send(sock, buf2, r, 0);
    if (n < 0)
    {
        return -1;
    }

    fprintf(stderr, "[+] The image \"%s\" with size \"%d\"has been sent to server\n", file, r);

    return 0;
}

int main(int, char **)
{
    const unsigned short port = 8077;
    int sock, sock_peer, ip_h;
    struct sockaddr_in addr, peer_addr;
    struct linger linger_opt = {1, 0};
    socklen_t peer_addr_size = sizeof(peer_addr);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("[-] Can't create socket\n");
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    if (bind(sock, (const struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        printf("[-] Can't bind socket to port\n");
        close(sock);
        return -1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt)) != 0)
    {
        printf("[-] Can't linger socket\n");
        close(sock);
        return -1;
    }
    if (listen(sock, 2) != 0)
    {
        printf("[-] Can't set listen queue for socket\n");
        close(sock);
        return -1;
    }
    printf("[+] Server listening on port %d\n", port);
    while (1)
    {
        sock_peer = accept(sock, (struct sockaddr *)&peer_addr, &peer_addr_size);
        if (sock_peer < 0)
        {
            printf("[-] Can't accept new connection\n");
            close(sock);
            return -1;
        }
        ip_h = ntohl(peer_addr.sin_addr.s_addr);

        printf("[+] New connection accepted from %d.%d.%d.%d on port %d\n", (ip_h >> 24) & 0xff, (ip_h >> 16) & 0xff, (ip_h >> 8) & 0xff, ip_h & 0xff, htons(peer_addr.sin_port));

        work(sock_peer);
    }
    close(sock);
    return 0;
}

void work(int sock)
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
    char buf[10000], buf2[10000];
    int r;
    r = recv(sock, buf, sizeof(buf) - 1, 0);
    buf[r] = '\0';
    printf("[+] Buf:\n%s\n", buf);

    buf2[0] = 0;
    strcat(buf2, buf + 4);
    for (size_t i = 0; i < strlen(buf2); i++)
    {
        if (buf2[i] == ' ' || buf2[i] == '\n')
        {
            buf2[i] = 0;
            break;
        }
    }

    printf("[+] Trying to send the file: %s\n", buf2);
    if (!strcmp(buf2, "/"))
    {
        SendPage(sock, "first_page.html");
    }
    if (!strcmp(buf2, "/404.png"))
    {
        SendImage(sock, buf2 + 1);
    }
}