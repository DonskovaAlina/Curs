#include "client.h"
#include <arpa/inet.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <cstdint>

using namespace std;

// Функция для вычисления SHA-224 хэша строки
string SHA224_hash(const string &hsh) {
    string hash;
    CryptoPP::SHA224 sha224;
    CryptoPP::StringSource(
        hsh, true,
        new CryptoPP::HashFilter(
            sha224, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash))));
    return hash;
}

// Функция для подключения к серверу
int connect_to_server() {
    cout << "Подключение к серверу по адресу " << SERVER_ADDRESS
         << " на порту " << DEFAULT_PORT << endl;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw runtime_error("Ошибка создания сокета");
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, SERVER_ADDRESS, &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        throw runtime_error("Ошибка подключения к серверу");
    }

    cout << "Успешно подключено к серверу." << endl;
    return sock;
}

// Функция для аутентификации клиента на сервере
void authenticate_client(int sock, const string &login, const string &password) {
    cout << "Аутентификация клиента..." << endl;
    send(sock, login.c_str(), login.size(), 0);

    char salt[SALT_SIZE] = {0};
    if (recv(sock, salt, SALT_SIZE, 0) <= 0) {
        throw runtime_error("Ошибка получения SALT от сервера");
    }


    string hash_input(salt, SALT_SIZE);
    hash_input += password;
    string hash_output = SHA224_hash(hash_input);

    cout << "Хэшированный пароль: " << hash_output << endl;

    if (send(sock, hash_output.c_str(), hash_output.size(), 0) < 0) {
        throw runtime_error("Ошибка отправки данных аутентификации");
    }

    char response[3];
    if (recv(sock, response, sizeof(response) - 1, 0) <= 0 ||
        strncmp(response, "OK", 2) != 0) {
        throw runtime_error("Ошибка аутентификации: неверный логин или пароль");
    }

    cout << "Аутентификация успешна." << endl;
}

void send_vectors(int sock, const vector<vector<float>> &vectors, const string &output_file) {
    cout << "Отправка векторов на сервер..." << endl;
    uint32_t N = vectors.size();
    send(sock, &N, sizeof(N), 0);

    ofstream outfile(output_file); // Открытие файла в текстовом режиме
    if (!outfile) {
        throw runtime_error("Ошибка открытия файла для записи результатов: " + output_file);
    }

    for (const auto &vector : vectors) {
        uint32_t S = vector.size();
        if (send(sock, &S, sizeof(S), 0) < 0) {
            throw runtime_error("Ошибка отправки размера вектора");
        }
        if (send(sock, vector.data(), S * sizeof(float), 0) < 0) {
            throw runtime_error("Ошибка отправки данных вектора");
        }

        float result;
        if (recv(sock, &result, sizeof(result), 0) <= 0) {
            throw runtime_error("Ошибка получения результата от сервера");
        }
        cout << "Получен результат для вектора: " << result << endl;

        // Запись результата в текстовый файл
        outfile << result << endl;
    }

    outfile.close();
}
