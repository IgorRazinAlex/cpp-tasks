# C++, ФПМИ МФТИ, осень 2022 - весна 2023
# Оглавление
- [Начало работы](#начало-работы)
- [Формат сдачи задач](#как-сдавать-задачи)
- [Полезные ссылки](#полезные-ссылки)
- [Таблица с результатами](https://docs.google.com/spreadsheets/d/1stlHqG1o7f2N1gPj7SgvM253V6KVPlMteuiUVfbnfjk/edit#gid=1167867899)

# Полезные ссылки
- [cppreference](https://en.cppreference.com/w/)
- [C++23 Working Draft N4928](https://isocpp.org/files/papers/N4928.pdf)

# Начало работы
Необходимо создать репозиторий, в который вы будете коммитить свои решения. Чтобы его создать, можно выполнить следующие шаги:
1) Его можно создать использовав текущий template repository. \
Для этого нажимаем **Use this template ->  Create a new repository**

<img width="391" alt="how-to-use-template" src="https://user-images.githubusercontent.com/36928556/228303272-1992e767-0da6-4cbd-88b0-81fa67095799.png">

2) **Обязательно делаем репозиторий приватным**. \
Все решения, выложенные в открытый доступ, будут забанены (┛ಠ_ಠ)┛彡┻━┻

<img width="765" alt="how-to-create-repo" src="https://user-images.githubusercontent.com/36928556/228299763-b57694f3-9955-49a1-8b7d-7ec80799db78.png">

3) Вы создали свой собственный репозиторий, вы прекрасны. Теперь необходимо склонировать его себе на устройство и начать решать задачи. На этом моменте я сошлюсь на замечательный курс "*Технологии программирования*". Про формат сдачи заданий можно почитать [далее](#как-сдавать-задачи)

# Как сдавать задачи
1) Чтобы сдать очередную задачу, создаем новую ветку с названием задачи
```
git checkout main
git checkout -b <task_name>
```

2) Кладем решение задачи в директорию `tasks/task_name`. Локально проверяем, что решение проходит `clang-tidy` и `clang-format`:
```
clang-tidy deque.h
diff deque.h <(clang-format deque.h)
```

3) Коммитим изменения и пушим ветку на github
```
git commit -m "Hopefully adequate commit message"
git push -u origin <task_name>
```

4) Создаем *Pull Request* из ветки `task_name` в ветку `main`. *Pull Request* должен содержать в описании ссылку на последнюю успешную посылку по задаче (решение в контесте должно совпадать с решением в репозитории)

5) Убеждаемся, что CI отработало успешно, добавляем в *Reviewers* ассистента и ждем фидбек. Не забываем добавить ассистента в коллабораторы репозитория, если его там нет

6) После поправок коммитим изменения в ту же ветку и запрашиваем повторное *review*
