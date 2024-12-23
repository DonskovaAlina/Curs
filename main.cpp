/**
 * @file main.cpp
 * @brief Основной файл программы.
 * @author Донскова А.Д.
 * @version 1.0
 * @date 22.12.2024 
 */
#include "interface.h"
#include "client.h"
#include <iostream>

/**
 * @brief Точка входа в программу.
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
 * @return Код возврата.
 */
int main(int argc, char* argv[]) {
    try {
        Interface interface(argc, argv);
        interface.ParseCommandLine();

        std::string login, password;
        interface.ReadConfigFile(interface.GetConfigFile(), login, password);

        Client client(interface.GetServerAddress(), interface.GetServerPort());
        client.Connect();
        client.Authenticate(login, password);

        auto vectors = interface.ReadVectorsFromFile(interface.GetInputFile());
        client.SendVectors(vectors, interface.GetOutputFile());
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}