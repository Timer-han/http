# Что делает код

## LOGIC
`http.c` - основная логика. Некоторые функции:

`SendFile` - отправка изображения формата png, html страницы
`work` - работа внутри одного треда

## StartPage
Стартовая html страница "[`first_page.html`](./first_page/page.html)" содержит просто текст, [`картинку`](./first_page/404.png) и ссылку на сайт "[page.html](./top5meme/page.html)" с мемами

## 404 ERROR
В случае открытия несуществующей страницы, отправляется картинка [`404.png`](./first_page/404.png)

# HOW TO RUN
1. клонируете репозиторий себе на компьютер
2. компилируете и запускаете файл http.c
3. заходите в браузере на сайт "http://127.0.0.1:8077"
4. если что-то не работает, отправляете мне скрин в тг @timer_han