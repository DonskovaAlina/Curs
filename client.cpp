/**
 * @file client.cpp
 * @brief Реализация класса Client.
 * @author Донскова А.Д.
 * @version 1.0
 * @date 22.12.2024 
 */
#include "client.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>

/**
 * @brief Конструктор класса Client.
 * @param server_address Адрес сервера.
 * @param server_port Порт сервера.
 */
Client::Client(const std::string& server_address, int server_port)
    : server_address_(server_address), server_port_(server_port), sock_(-1) {}

/**
 * @brief Деструктор класса Client.
 */
Client::~Client() {
    if (sock_ != -1) {
        close(sock_);
    }
}

/**
 * @brief Подключение к серверу.
 * @throws std::runtime_error В случае ошибки подключения.
 */
void Client::Connect() {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        throw std::runtime_error("Ошибка создания сокета");
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port_);
    inet_pton(AF_INET, server_address_.c_str(), &server.sin_addr);

    if (connect(sock_, (struct sockaddr*)&server, sizeof(server)) < 0) {
        throw std::runtime_error("Ошибка подключения к серверу");
    }

    std::cout << "Успешно подключено к серверу." << std::endl;
}

/**
 * @brief Аутентификация на сервере.
 * @param login Логин пользователя.
 * @param password Пароль пользователя.
 * @throws std::runtime_error В случае ошибки аутентификации.
 */
void Client::Authenticate(const std::string& login, const std::string& password) {
    send(sock_, login.c_str(), login.size(), 0);

    char salt[16] = {0};
    if (recv(sock_, salt, sizeof(salt), 0) <= 0) {
        throw std::runtime_error("Ошибка получения SALT от сервера");
    }

    std::string hash_input(salt, sizeof(salt));
    hash_input += password;
    std::string hash_output = SHA224_hash(hash_input);

    if (send(sock_, hash_output.c_str(), hash_output.size(), 0) < 0) {
        throw std::runtime_error("Ошибка отправки данных аутентификации");
    }

    char response[3];
    if (recv(sock_, response, sizeof(response) - 1, 0) <= 0 || strncmp(response, "OK", 2) != 0) {
        throw std::runtime_error("Ошибка аутентификации: неверный логин или пароль");
    }

    std::cout << "Аутентификация успешна." << std::endl;
}

/**
 * @brief Отправка векторов на сервер.
 * @param vectors Векторы для отправки.
 * @param output_file Файл для записи результатов.
 * @throws std::runtime_error В случае ошибки отправки данных.
 */
void Client::SendVectors(const std::vector<std::vector<float>>& vectors, const std::string& output_file) {
    uint32_t N = vectors.size();
    send(sock_, &N, sizeof(N), 0);

    std::ofstream outfile(output_file);
    if (!outfile) {
        throw std::runtime_error("Ошибка открытия файла для записи результатов: " + output_file);
    }

    for (const auto& vector : vectors) {
        uint32_t S = vector.size();
        if (send(sock_, &S, sizeof(S), 0) < 0) {
            throw std::runtime_error("Ошибка отправки размера вектора");
        }
        if (send(sock_, vector.data(), S * sizeof(float), 0) < 0) {
            throw std::runtime_error("Ошибка отправки данных вектора");
        }

        float result;
        if (recv(sock_, &result, sizeof(result), 0) <= 0) {
            throw std::runtime_error("Ошибка получения результата от сервера");
        }
        std::cout << "Получен результат для вектора: " << result << std::endl;

        outfile << result << std::endl;
    }

    outfile.close();
}

/**
 * @brief Вычисление SHA-224 хеша.
 * @param input Входная строка для хеширования.
 * @return Хеш в виде строки.
 */
std::string Client::SHA224_hash(const std::string& input) {
    std::string hash;
    CryptoPP::SHA224 sha224;
    CryptoPP::StringSource(input, true,
        new CryptoPP::HashFilter(sha224, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash))));
    return hash;
}