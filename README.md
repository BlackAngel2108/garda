# Проект "Тестовое задание C++"

Этот проект представляет собой пошаговое выполнение тестового задания для C++ разработчика.

## Уровни

*   **Level 1:** Создание "Hello, World" на C++ с использованием CMake.
*   **Level 2:** Контейнеризация с помощью Docker.
*   **Level 3:** Настройка удаленной разработки в VS Code.
*   **Level 4:** Инициализация Git-репозитория.
*   **Level 5:** Разработка HTTP Echo-сервера на C++ с использованием `cpp-httplib`.
*   **Level 6:** Разработка C++ калькулятора.
*   **Level 7:** Разработка HTTP Calculator Server.
*   **Level 8:** Разработка C++ CLI клиента для Calculator Server.
*   ... и далее (согласно `testovoe.txt`)

## Требования

*   **Git**
*   **Docker Desktop** (должен быть запущен)
*   **Visual Studio Code** с установленным расширением [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers).
*   **C++ Compiler и CMake** (будут установлены автоматически внутри Dev Container).
*   **Библиотеки:** `cpp-httplib`, `nlohmann/json`, `googletest` (управляются через Git-подмодули или CMake FetchContent).

## Инструкция по запуску

Вы можете запустить этот проект двумя способами. Рекомендуемый способ - с помощью интеграции VS Code, так как он полностью автоматизирован.

### Важный шаг: Инициализация подмодулей

После клонирования репозитория (или если вы уже клонировали его), необходимо инициализировать Git-подмодули, чтобы получить все внешние библиотеки:

```bash
git submodule update --init --recursive
```

### Способ 1: С помощью VS Code Dev Containers (Рекомендуемый)

Этот способ автоматически создает Docker-образ, запускает контейнер и подключает к нему VS Code.

1.  **Клонируйте репозиторий:**
    ```bash
    git clone <URL вашего репозитория>
    cd <папка проекта>
    ```

2.  **Инициализируйте подмодули (если вы не сделали это ранее):**
    ```bash
    git submodule update --init --recursive
    ```

3.  **Откройте проект в VS Code:**
    ```bash
    code .
    ```

4.  **Переоткройте проект в контейнере:**
    *   После открытия папки VS Code автоматически обнаружит конфигурацию `.devcontainer` и предложит **"Reopen in Container"**. Нажмите на эту кнопку.
    *   **Важно:** Если вы меняли `Dockerfile` или `devcontainer.json`, вам может понадобиться команда **"Dev Containers: Rebuild Container"** через палитру команд (`Ctrl+Shift+P`), чтобы изменения вступили в силу.
    *   Дождитесь, пока VS Code соберет Docker-образ и настроит среду разработки. В первый раз это может занять несколько минут.

5.  **Соберите проект:**
    *   После того, как проект откроется в контейнере, откройте терминал VS Code (Terminal -> New Terminal).
    *   Выполните следующие команды для сборки:
        ```bash
        mkdir -p build && cd build
        cmake ..
        cmake --build .
        ```
    *   Исполняемые файлы будут расположены в `build/bin/` (например, `build/bin/http_server`, `build/bin/calc_client`).

6.  **Запустите HTTP Calculator Server (`http_server`):**
    *   Из каталога `build` внутри контейнера выполните:
        ```bash
        ./bin/http_server
        ```
    *   Сервер запустится и будет ожидать подключений на порту `8080`.

7.  **Используйте CLI Client (`calc_client`):**
    *   Откройте **новый** терминал **на вашей хост-машине** (не в контейнере).
    *   Используйте `calc_client` для отправки запросов на сервер, работающий в контейнере. Убедитесь, что порт 8080 проброшен (как это обычно настроено в `.devcontainer` или при ручном запуске Docker).
    *   **Примеры использования `calc_client`:**
        *   **Команда "echo":**
            ```bash
            ./garda/build/bin/calc_client -s http://localhost:8080 -c echo
            ```
            Ожидаемый вывод: `echo`
        *   **Вычисление выражения "2 + 2":**
            ```bash
            ./garda/build/bin/calc_client -s http://localhost:8080 -e "2 + 2"
            ```
            Ожидаемый вывод: `4`
        *   **Неверное выражение:**
            ```bash
            ./garda/build/bin/calc_client -s http://localhost:8080 -e "2 ++ 2"
            ```
            Ожидаемый вывод: `Error from server: Invalid expression` (или аналогичное сообщение)
        
        *   **Использование переменных (одна строка):**
            ```bash
            ./garda/build/bin/calc_client -s http://localhost:8080 -e "var = 2 + 3"
            ```
            Ожидаемый вывод: `5`

        *   **Использование переменных (многострочный ввод):**
            ```bash
            ./garda/build/bin/calc_client -s http://localhost:8080 -e "foo = 10; bar = foo * 2; bar + 5"
            ```
            Ожидаемый вывод: `25`

        *   **Неизвестная переменная:**
            ```bash
            ./garda/build/bin/calc_client -s http://localhost:8080 -e "unknown_var * 2"
            ```
            Ожидаемый вывод: `Error from server: Unknown variable: unknown_var`

        *   **Команда "clean":**
            ```bash
            ./garda/build/bin/calc_client -s http://localhost:8080 -c clean
            ```
            Ожидаемый вывод: `Operation successful (no explicit result).`
        
        *   **Stateful: Хранение промежуточных результатов (Задание 10):**
            ```bash
            # Установка переменной
            ./garda/build/bin/calc_client -s http://localhost:8080 -e "pi=3.14"
            # Ожидаемый вывод: Operation successful (no explicit result).

            # Использование переменной в следующем запросе
            ./garda/build/bin/calc_client -s http://localhost:8080 -e "2 * pi * 3"
            # Ожидаемый вывод: 18.84
            
            # Сброс состояния
            ./garda/build/bin/calc_client -s http://localhost:8080 -c clean
            # Ожидаемый вывод: Operation successful (no explicit result).

            # Переменная pi больше недоступна
            ./garda/build/bin/calc_client -s http://localhost:8080 -e "2 * pi * 3"
            # Ожидаемый вывод: Error from server: Unknown variable: pi
            ```

        ### **Задание 11: Sessions**
        Ключевой особенностью Задания 11 является поддержка сессий, которая позволяет изолировать переменные для разных пользователей или контекстов. Это достигается путем передачи параметра `sid` (Session ID) в JSON-запросе к серверу.

        *   **Установка переменной в сессии 'A':**
            ```bash
            # Пример raw JSON запроса (если клиент поддерживает)
            # { "sid": "A", "exp": "x=10" }
            # Ожидаемый Response: {} (или res: 10.0)
            ```

        *   **Использование переменной в той же сессии 'A':**
            ```bash
            # Пример raw JSON запроса
            # { "sid": "A", "exp": "x * 2" }
            # Ожидаемый Response: { "res": 20.0 }
            ```

        *   **Изоляция сессий: Переменная недоступна без указания Session ID:**
            ```bash
            # Пример raw JSON запроса (без указания sid)
            # { "exp": "x + 1" }
            # Ожидаемый Response: { "err": "Unknown variable: x" }
            ```

        *   **Изоляция сессий: Переменная недоступна в другой сессии 'B':**
            ```bash
            # Пример raw JSON запроса (в сессии 'B')
            # { "sid": "B", "exp": "x + 1" }
            # Ожидаемый Response: { "err": "Unknown variable: x" }
            ```

        *   **Очистка состояния для конкретной сессии 'A':**
            ```bash
            # Пример raw JSON запроса
            # { "sid": "A", "cmd": "clean" }
            # Ожидаемый Response: {}

            # Проверка, что переменная 'x' удалена из сессии 'A'
            # { "sid": "A", "exp": "x + 1" }
            # Ожидаемый Response: { "err": "Unknown variable: x" }
            ```
        *   **Проверка, что другая сессия 'B' не затронута очисткой сессии 'A':**
            ```bash
            # Предположим, 'x=9' была установлена в сессии 'B' ранее
            # { "sid": "B", "exp": "x + 1" }
            # Ожидаемый Response: { "res": 10.0 }
            ```


        
        *Примечание: Если вы запускаете `calc_client` из другого места или вне смонтированной папки, скорректируйте путь к исполняемому файлу.*

8.  **Запустите тесты:**
    Тесты запускаются из каталога `build` внутри контейнера.
    *   **Модульные тесты калькулятора (`calculator_tests`):**
        ```bash
        ./bin/calculator_tests
        ```
    *   **Интеграционные тесты сервера (`server_tests`):**
        ```bash
        ./bin/server_tests
        ```
    *   **Интеграционные тесты клиента (`client_tests`):**
        Для клиентских тестов требуется запущенный mock-сервер.
        *   Запустите mock-сервер в фоновом режиме:
            ```bash
            ./bin/mock_server &
            ```
            *Mock-сервер будет прослушивать порт 8081.*
        *   Запустите клиентские тесты:
            ```bash
            ./bin/client_tests
            ```
        *   После завершения тестов **обязательно завершите процесс mock-сервера**:
            ```bash
            killall mock_server # или найдите PID с помощью `ps aux | grep mock_server` и используйте `kill <PID>`
            ```
        
    *Примечание: Если `mock_server` не завершить, он может занять порт 8081 и помешать будущим запускам тестов или других процессов.*

### Способ 2: Вручную с использованием Docker

Этот способ показывает, как вручную управлять Docker для сборки и запуска проекта. Это полезно для понимания того, что происходит "под капотом" у Dev Containers.

1.  **Клонируйте репозиторий:**
    ```bash
    git clone <URL вашего репозитория>
    cd <папка проекта>
    ```

2.  **Инициализируйте подмодули:**
    ```bash
    git submodule update --init --recursive
    ```

3.  **Соберите Docker-образ:**
    Откройте терминал в папке проекта и выполните:
    ```bash
    docker build -t garda-cpp-env .
    ```

4.  **Запустите контейнер в фоновом режиме, пробросив порты:**
    Эта команда запускает контейнер и монтирует текущую папку проекта в директорию `/app` внутри контейнера.
    Для корректной работы сервера (8080) и mock-сервера (8081) необходимо пробросить оба порта.
    ```bash
    # Для PowerShell
    docker run -d -it --name garda_dev_container -p 8080:8080 -p 8081:8081 -v "${PWD}:/app" garda-cpp-env

    # Для Git Bash / CMD
    docker run -d -it --name garda_dev_container -p 8080:8080 -p 8081:8081 -v "%cd%:/app" garda-cpp-env
    ```

5.  **Подключитесь к командной строке контейнера:**
    ```bash
    docker exec -it garda_dev_container bash
    ```

6.  **Скомпилируйте проект (внутри контейнера):**
    После выполнения предыдущей команды вы окажетесь в терминале внутри контейнера. Выполните следующие команды:
    ```bash
    # Создаем папку для сборки и переходим в нее
    mkdir -p build && cd build

    # Конфигурируем проект с помощью CMake
    cmake ..

    # Собираем проект
    cmake --build .
    ```

7.  **Запустите HTTP Calculator Server (`http_server`) (внутри контейнера):**
    *   Внутри контейнера, из каталога `build`, выполните:
        ```bash
        ./bin/http_server
        ```
    *   Сервер запустится и будет ожидать подключений на порту `8080`.

8.  **Используйте CLI Client (`calc_client`) (на вашей хост-машине):**
    *   Откройте **новый** терминал **на вашей хост-машине** (не в контейнере).
    *   Используйте `calc_client` для отправки запросов на сервер.
    *   **Примеры использования `calc_client`:**
        *   **Команда "echo":**
            ```bash
            ./garda/build/bin/calc_client -s http://localhost:8080 -c echo
            ```
            Ожидаемый вывод: `echo`
        *   **Вычисление выражения "2 + 2":**
            ```bash
            ./garda/build/bin/calc_client -s http://localhost:8080 -e "2 + 2"
            ```
            Ожидаемый вывод: `4`
        *   **Неверное выражение:**
            ```bash
            ./garda/build/bin/calc_client -s http://localhost:8080 -e "2 ++ 2"
            ```
            Ожидаемый вывод: `Error from server: Invalid expression` (или аналогичное сообщение)

9.  **Запустите тесты (внутри контейнера):**
    Из каталога `build` внутри контейнера.
    *   **Модульные тесты калькулятора (`calculator_tests`):**
        ```bash
        ./bin/calculator_tests
        ```
    *   **Интеграционные тесты сервера (`server_tests`):**
        ```bash
        ./bin/server_tests
        ```
    *   **Интеграционные тесты клиента (`client_tests`):**
        *   Запустите mock-сервер в фоновом режиме:
            ```bash
            ./bin/mock_server &
            ```
            *Mock-сервер будет прослушивать порт 8081.*
        *   Запустите клиентские тесты:
            ```bash
            ./bin/client_tests
            ```
        *   После завершения тестов **обязательно завершите процесс mock-сервера**:
            ```bash
            killall mock_server # или найдите PID с помощью `ps aux | grep mock_server` и используйте `kill <PID>`
            ```

10. **Остановка и удаление контейнера (когда закончите):**
    ```bash
    docker stop garda_dev_container
    docker rm garda_dev_container
    ```