# Проект "Тестовое задание C++"

Этот проект представляет собой пошаговое выполнение тестового задания для C++ разработчика.

## Уровни

*   **Level 1:** Создание "Hello, World" на C++ с использованием CMake.
*   **Level 2:** Контейнеризация с помощью Docker.
*   **Level 3:** Настройка удаленной разработки в VS Code.
*   **Level 4:** Инициализация Git-репозитория.
*   **Level 5:** Разработка HTTP Echo-сервера на C++ с использованием `cpp-httplib`.
*   ... и далее

## Требования

*   **Git**
*   **Docker Desktop** (должен быть запущен)
*   **Visual Studio Code** с установленным расширением [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers).
*   **C++ Compiler и CMake** (будут установлены автоматически внутри Dev Container).
*   **Библиотека `cpp-httplib`** (добавлена как Git-подмодуль в папку `extern/cpp-httplib`).

## Инструкция по запуску

Вы можете запустить этот проект двумя способами. Рекомендуемый способ - с помощью интеграции VS Code, так как он полностью автоматизирован.

### Важный шаг: Инициализация подмодулей

После клонирования репозитория (или если вы уже клонировали его), необходимо инициализировать Git-подмодули, чтобы получить библиотеку `cpp-httplib`:

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

5.  **Соберите и запустите Echo-сервер:**
    *   После того, как проект откроется в контейнере, перейдите на вкладку "Run and Debug" (Выполнение и отладка) в VS Code.
    *   Нажмите зеленую стрелку "▶" рядом с "Debug echo_server" или просто нажмите `F5`.
    *   Проект будет автоматически скомпилирован, и вывод сервера (`Server is starting on http://0.0.0.0:8080` и логи запросов) появится в консоли отладки.

6.  **Протестируйте сервер:**
    *   Откройте браузер и перейдите по адресу `http://localhost:8080`. Вы увидите приветственное сообщение.
    *   Для тестирования Echo-функционала откройте терминал **на вашей хост-машине** (не в контейнере) и выполните:
        *   **Для Git Bash, Linux, macOS:**
            ```bash
            curl -X POST -H "Content-Type: text/plain" -d "Hello from curl!" http://localhost:8080/echo
            ```
        *   **Для PowerShell:**
            ```powershell
            Invoke-WebRequest -Uri http://localhost:8080/echo -Method Post -Headers @{"Content-Type" = "text/plain"} -Body "Hello from PowerShell!"
            ```
        В ответ вы должны получить отправленные данные, а в консоли отладки VS Code увидеть сообщение о полученном запросе.

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

4.  **Запустите контейнер в фоновом режиме, пробросив порт 8080:**
    Эта команда запускает контейнер и монтирует текущую папку проекта в директорию `/app` внутри контейнера.
    ```bash
    # Для PowerShell
    docker run -d -it --name dev_container -p 8080:8080 -v "${PWD}:/app" garda-cpp-env

    # Для Git Bash / CMD
    docker run -d -it --name dev_container -p 8080:8080 -v "%cd%:/app" garda-cpp-env
    ```

5.  **Подключитесь к командной строке контейнера:**
    ```bash
    docker exec -it dev_container bash
    ```

6.  **Скомпилируйте и запустите проект (внутри контейнера):**
    После выполнения предыдущей команды вы окажетесь в терминале внутри контейнера. Выполните следующие команды:
    ```bash
    # Создаем папку для сборки и переходим в нее
    mkdir -p build && cd build

    # Конфигурируем проект с помощью CMake
    cmake ..

    # Собираем проект (Echo-сервер)
    cmake --build .

    # Запускаем приложение (Echo-сервер)
    ./echo_server
    ```

7.  **Протестируйте сервер:**
    *   Откройте браузер на хост-машине и перейдите по адресу `http://localhost:8080`.
    *   Или используйте `curl` на хост-машине:
        *   **Для Git Bash, Linux, macOS:**
            ```bash
            curl -X POST -H "Content-Type: text/plain" -d "Hello from curl!" http://localhost:8080/echo
            ```
        *   **Для PowerShell:**
            ```powershell
            Invoke-WebRequest -Uri http://localhost:8080/echo -Method Post -Headers @{"Content-Type" = "text/plain"} -Body "Hello from PowerShell!"
            ```

8.  **Остановка и удаление контейнера (когда закончите):**
    ```bash
    docker stop dev_container
    docker rm dev_container
    ```
