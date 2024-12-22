#include "interface.h"
#include "client.h"
#include <iostream>
#include <stdexcept>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
    string input_file;
    string output_file;
    string config_file;

    // Обработка аргументов командной строки
    try {
        parse_command_line_arguments(argc, argv, input_file, output_file, config_file);
    } catch (const exception &e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    // Проверка, что все необходимые параметры указаны
    if (input_file.empty() || output_file.empty()) {
        cerr << "Ошибка: файл входных данных и файл результатов должны быть указаны." << endl;
        return 1;
    }

    try {
        auto vectors = read_vectors_from_file(input_file);
        int sock = connect_to_server(); // Используются фикс адрес и порт

        string login, password;
        read_config(config_file, login, password); // Чтение конфигурации

        authenticate_client(sock, login, password);
        send_vectors(sock, vectors, output_file); // Передача имени выходного файла

        close(sock);
        cout << "Соединение с сервером закрыто." << endl;
    } catch (const exception &e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}