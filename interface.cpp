#include "interface.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <getopt.h>

using namespace std;

// Функция для обработки аргументов командной строки
void parse_command_line_arguments(int argc, char *argv[], string &input_file, string &output_file, string &config_file) {
    config_file = string(getenv("HOME")) + "/.config/vclient.conf"; // Значение по умолчанию

    int opt;
    while ((opt = getopt(argc, argv, ":i:o:c:h")) != -1) {
        switch (opt) {
            case 'h':
                cout << "Использование: " << argv[0]
                     << " -i <файл входных данных> -o <файл результатов> -c <файл конфигурации>" << endl;
                exit(0); // Выход с выводом справки
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'c':
                config_file = optarg;
                break;
            default:
                throw runtime_error("Неверные параметры.");
        }
    }

    // Проверка на обязательные параметры
    if (input_file.empty() || output_file.empty()) {
        throw runtime_error("Файл входных данных и файл результатов должны быть указаны.");
        cout << " -i <файл входных данных> -o <файл результатов> -c <файл конфигурации>" << endl;
    }
}

// Функция для чтения векторов из бинарного файла
vector<vector<float>> read_vectors_from_file(const string &filename) {
    cout << "Чтение векторов из файла: " << filename << endl;
    ifstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Ошибка открытия файла: " + filename);
    }

    uint32_t N;
    file.read(reinterpret_cast<char *>(&N), sizeof(N));
    if (file.gcount() != sizeof(N)) {
        throw runtime_error("Ошибка чтения количества векторов из файла: " + filename);
    }
    
    cout << "Найдено " << N << " векторов." << endl;

    vector<vector<float>> vectors(N);
    for (uint32_t i = 0; i < N; ++i) {
        uint32_t S;
        file.read(reinterpret_cast<char *>(&S), sizeof(S));
        if (file.gcount() != sizeof(S)) {
            throw runtime_error("Ошибка чтения размера вектора " + to_string(i) + " из файла: " + filename);
        }
        
        vectors[i].resize(S);
        file.read(reinterpret_cast<char *>(vectors[i].data()), S * sizeof(float));
        if (file.gcount() != static_cast<std::streamsize>(S * sizeof(float))) {
            throw runtime_error("Ошибка чтения данных вектора " + to_string(i) + " из файла: " + filename);
        }

        // Вывод вектора для проверки
        cout << "Вектор " << i + 1 << ": ";
        for (const auto &value : vectors[i]) {
            cout << fixed << setprecision(2) << value << " ";
        }
        cout << endl;
    }

    return vectors;
}

// Функция для чтения конфигурации из файла
void read_config(const string &config_file, string &login, string &password) {
    ifstream config(config_file);
    if (config.is_open()) {
        string line;
        if (getline(config, line)) {
            size_t delimiter_pos = line.find(':');
            if (delimiter_pos != string::npos) {
                login = line.substr(0, delimiter_pos);
                password = line.substr(delimiter_pos + 1);
            } else {
                throw runtime_error("Неверный формат файла конфигурации. Ожидается 'user:password'.");
            }
        }
    } else {
        throw runtime_error("Ошибка открытия файла конфигурации: " + config_file);
    }
}