#include <stdio.h>
#include <string.h>

int IntToStr(int n, char * a){
    int i = 0;
    if (n < 0){
        n *= -1;
        a[i++] = '-';
    }
    while (n > 0){
        
    }
}

int main(void)
{
    char text[10000];
    if (readfile("first_page.html", text, sizeof(text)) < 0){
        return -1;
    }
}