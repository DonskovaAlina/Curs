/**
 * @file interface.h
 * @brief Заголовочный файл для класса Interface, который отвечает за обработку командной строки и чтение файлов.
 * @author Донскова А.Д.
 * @version 1.0
 * @date 22.12.2024 
 */
#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <stdexcept>

/**
 * @class Interface
 * @brief Класс для обработки командной строки и чтения файлов.
 * 
 * Этот класс предоставляет методы для парсинга командной строки, чтения векторов из файла и чтения конфигурационного файла.
 */
class Interface {
public:
    /**
     * @brief Конструктор класса Interface.
     * @param argc Количество аргументов командной строки.
     * @param argv Массив аргументов командной строки.
     */
    Interface(int argc, char* argv[]);

    /**
     * @brief Парсинг командной строки.
     * @throws std::runtime_error В случае ошибки парсинга.
     */
    void ParseCommandLine();

    /**
     * @brief Чтение векторов из файла.
     * @param filename Имя файла.
     * @return Вектор векторов типа float.
     * @throws std::runtime_error В случае ошибки чтения файла.
     */
    std::vector<std::vector<float>> ReadVectorsFromFile(const std::string& filename);

    /**
     * @brief Чтение конфигурационного файла.
     * @param config_file Имя конфигурационного файла.
     * @param login Ссылка на строку для сохранения логина.
     * @param password Ссылка на строку для сохранения пароля.
     * @throws std::runtime_error В случае ошибки чтения файла.
     */
    void ReadConfigFile(const std::string& config_file, std::string& login, std::string& password);

    /**
     * @brief Получение имени входного файла.
     * @return Имя входного файла.
     */
    std::string GetInputFile() const { return input_file_; }

    /**
     * @brief Получение имени выходного файла.
     * @return Имя выходного файла.
     */
    std::string GetOutputFile() const { return output_file_; }

    /**
     * @brief Получение имени конфигурационного файла.
     * @return Имя конфигурационного файла.
     */
    std::string GetConfigFile() const { return config_file_; }

    /**
     * @brief Получение адреса сервера.
     * @return Адрес сервера.
     */
    std::string GetServerAddress() const { return server_address_; }

    /**
     * @brief Получение порта сервера.
     * @return Порт сервера.
     */
    int GetServerPort() const { return server_port_; }

private:
    int argc_; ///< Количество аргументов командной строки.
    char** argv_; ///< Массив аргументов командной строки.
    std::string input_file_; ///< Имя входного файла.
    std::string output_file_; ///< Имя выходного файла.
    std::string config_file_; ///< Имя конфигурационного файла.
    std::string server_address_; ///< Адрес сервера.
    int server_port_; ///< Порт сервера.
};

#endif // INTERFACE_H
