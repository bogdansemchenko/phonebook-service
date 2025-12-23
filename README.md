# Phonebook Service

## Стек технологий

* **Язык:** C++17
* **Web Framework:** Oat++ (1.3.0)
* **Пакетный менеджер:** Conan 2.0
* **Сборка:** CMake + Ninja
* **Тестирование:** Google Test (GTest)
* **Инфраструктура:** Docker & Docker Compose
* **Документация:** Swagger UI

## Требования

Для запуска вам потребуется только установленный **Docker** и **Docker Compose**.

## Установка и запуск

### 1. Клонирование репозитория

```bash
git clone [https://github.com/bogdansemchenko/phonebook-service.git](https://github.com/bogdansemchenko/phonebook-service.git)
cd phonebook-service
```
### 2. Запуск приложения

```bash
docker compose up --build
```
### 3. Запуск модульных тестов

```bash
docker compose run --rm phonebook ./run_tests
```
### 4. Запуск нагрузочных тестов

```bash
docker compose run --rm phonebook ./run_benchmarks
```
