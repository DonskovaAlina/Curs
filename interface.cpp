/**
 * @file interface.cpp
 * @brief Реализация класса Interface.
 * @author Донскова А.Д.
 * @version 1.0
 * @date 22.12.2024 
 */
#include "interface.h"
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <cstring>

/**
 * @brief Конструктор класса Interface.
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
 */
Interface::Interface(int argc, char* argv[])
    : argc_(argc), argv_(argv), server_port_(33333) {}

/**
 * @brief Парсинг командной строки.
 * @throws std::runtime_error В случае ошибки парсинга.
 */
void Interface::ParseCommandLine() {
    const char* short_options = "i:o:c:a:p:h";
    struct option long_options[] = {
        {"input", required_argument, nullptr, 'i'},
        {"output", required_argument, nullptr, 'o'},
        {"config", required_argument, nullptr, 'c'},
        {"address", required_argument, nullptr, 'a'},
        {"port", required_argument, nullptr, 'p'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc_, argv_, short_options, long_options, nullptr)) != -1) {
        switch (opt) {
            case 'i':
                input_file_ = optarg;
                break;
            case 'o':
                output_file_ = optarg;
                break;
            case 'c':
                config_file_ = optarg;
                break;
            case 'a':
                server_address_ = optarg;
                break;
            case 'p':
                server_port_ = std::stoi(optarg);
                break;
            case 'h':
                std::cout << "Использование: " << argv_[0]
                          << " -i <файл входных данных> -o <файл результатов> -c <файл конфигурации> -a <адрес сервера> [-p <порт сервера>]" << std::endl;
                exit(0);
            default:
                throw std::runtime_error("Неверные параметры.");
        }
    }

    if (input_file_.empty() || output_file_.empty() || server_address_.empty()) {
        throw std::runtime_error("Файл входных данных, файл результатов и адрес сервера должны быть указаны.");
    }

    if (server_port_ < 0 || server_port_ > 65535) {
        throw std::runtime_error("Недопустимое значение порта. Порт должен быть в диапазоне от 0 до 65535.");
    }
}

/**
 * @brief Чтение векторов из файла.
 * @param filename Имя файла.
 * @return Вектор векторов типа float.
 * @throws std::runtime_error В случае ошибки чтения файла.
 */
std::vector<std::vector<float>> Interface::ReadVectorsFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Ошибка открытия файла: " + filename);
    }

    uint32_t N;
    file.read(reinterpret_cast<char*>(&N), sizeof(N));
    std::vector<std::vector<float>> vectors(N);

    for (uint32_t i = 0; i < N; ++i) {
        uint32_t S;
        file.read(reinterpret_cast<char*>(&S), sizeof(S));
        vectors[i].resize(S);
        file.read(reinterpret_cast<char*>(vectors[i].data()), S * sizeof(float));
    }

    return vectors;
}

/**
 * @brief Чтение конфигурационного файла.
 * @param config_file Имя конфигурационного файла.
 * @param login Ссылка на строку для сохранения логина.
 * @param password Ссылка на строку для сохранения пароля.
 * @throws std::runtime_error В случае ошибки чтения файла.
 */
void Interface::ReadConfigFile(const std::string& config_file, std::string& login, std::string& password) {
    std::ifstream config(config_file);
    if (!config) {
        throw std::runtime_error("Ошибка открытия файла конфигурации: " + config_file);
    }

    std::string line;
    if (std::getline(config, line)) {
        size_t delimiter_pos = line.find(':');
        if (delimiter_pos != std::string::npos) {
            login = line.substr(0, delimiter_pos);
            password = line.substr(delimiter_pos + 1);
        } else {
            throw std::runtime_error("Неверный формат файла конфигурации. Ожидается 'user:password'.");
        }
    }
}