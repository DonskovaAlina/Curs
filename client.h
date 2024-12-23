/**
 * @file client.h
 * @brief Заголовочный файл для класса Client, который отвечает за взаимодействие с сервером.
 * @author Донскова А.Д.
 * @version 1.0
 * @date 22.12.2024 
 */
#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include <stdexcept>
#include <arpa/inet.h>
#include <unistd.h>

/**
 * @class Client
 * @brief Класс для взаимодействия с сервером.
 * 
 * Этот класс предоставляет методы для подключения к серверу, аутентификации и отправки данных.
 */
class Client {
public:
    /**
     * @brief Конструктор класса Client.
     * @param server_address Адрес сервера.
     * @param server_port Порт сервера.
     */
    Client(const std::string& server_address, int server_port);

    /**
     * @brief Деструктор класса Client.
     */
    ~Client();

    /**
     * @brief Подключение к серверу.
     * @throws std::runtime_error В случае ошибки подключения.
     */
    void Connect();

    /**
     * @brief Аутентификация на сервере.
     * @param login Логин пользователя.
     * @param password Пароль пользователя.
     * @throws std::runtime_error В случае ошибки аутентификации.
     */
    void Authenticate(const std::string& login, const std::string& password);

    /**
     * @brief Отправка векторов на сервер.
     * @param vectors Векторы для отправки.
     * @param output_file Файл для записи результатов.
     * @throws std::runtime_error В случае ошибки отправки данных.
     */
    void SendVectors(const std::vector<std::vector<float>>& vectors, const std::string& output_file);

    /**
     * @brief Вычисление SHA-224 хеша.
     * @param input Входная строка для хеширования.
     * @return Хеш в виде строки.
     */
    std::string SHA224_hash(const std::string& input);

private:
    std::string server_address_; ///< Адрес сервера.
    int server_port_; ///< Порт сервера.
    int sock_; ///< Дескриптор сокета.
};

#endif // CLIENT_H
