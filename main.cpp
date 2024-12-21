#include <arpa/inet.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

#define DEFAULT_PORT 33333
#define SERVER_ADDRESS "127.0.0.1" // Фиксированный адрес сервера
#define SALT_SIZE 16
#define SALT_HEX_LENGTH 16
#define MAX_BUFFER_SIZE 1024

// Функция для хэширования с использованием SHA-224 из Crypto++
string SHA224_hash(const string &hsh) {
  string hash;
  CryptoPP::SHA224 sha224;
  CryptoPP::StringSource(
      hsh, true,
      new CryptoPP::HashFilter(
          sha224, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash))));
  return hash;
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
  cout << "Найдено " << N << " векторов." << endl;

  vector<vector<float>> vectors(N);
  for (uint32_t i = 0; i < N; ++i) {
    uint32_t S;
    file.read(reinterpret_cast<char *>(&S), sizeof(S));
    vectors[i].resize(S);
    file.read(reinterpret_cast<char *>(vectors[i].data()), S * sizeof(float));
    
    // Выводим вектор в десятичной системе
    cout << "Вектор " << i+1 << ": ";
    for (const auto &value : vectors[i]) {
      cout << fixed << setprecision(2) << value << " "; // Выводим с двумя знаками после запятой
    }
    cout << endl;
  }

  return vectors;
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
  server.sin_port = htons(DEFAULT_PORT); // Используем фиксированный порт
  inet_pton(AF_INET, SERVER_ADDRESS,
            &server.sin_addr); // Используем фиксированный адрес

  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
    throw runtime_error("Ошибка подключения к серверу");
  }

  cout << "Успешно подключено к серверу." << endl;
  return sock;
}

// Функция для аутентификации клиента
void authenticate_client(int sock, const string &login,
                         const string &password) {
  cout << "Аутентификация клиента..." << endl;
  send(sock, login.c_str(), login.size(), 0);

  char salt[SALT_SIZE] = {0};
  if (recv(sock, salt, SALT_SIZE, 0) <= 0) {
    throw runtime_error("Ошибка получения SALT от сервера");
  }
  
  // Хэширование пароля с использованием соли
  string hash_input(salt, SALT_SIZE); // Используем бинарную соль
  hash_input += password; // Добавляем пароль
  string hash_output = SHA224_hash(hash_input);

  // Отправка данных аутентификации
  string msg = login + string(salt, SALT_SIZE) + hash_output; // Отправляем бинарную соль

  // Отладочная информация
  cout << "Хэшированный пароль: " << hash_output << endl;

  // Отправка данных аутентификации
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

// Функция для отправки векторов и получения результатов
void send_vectors(int sock, const vector<vector<float>> &vectors) {
  cout << "Отправка векторов на сервер..." << endl;
  uint32_t N = vectors.size();
  send(sock, &N, sizeof(N), 0);

  for (const auto &vector : vectors) {
    uint32_t S = vector.size();
    send(sock, &S, sizeof(S), 0);
    send(sock, vector.data(), S * sizeof(float), 0);

    // Получаем результат
    float result;
    if (recv(sock, &result, sizeof(result), 0) <= 0) {
      throw runtime_error("Ошибка получения результата от сервера");
    }
    cout << "Получен результат для вектора: " << result << endl;
  }
}

// Основная функция
int main(int argc, char *argv[]) {
  string input_file;
  string output_file;
  string config_file = string(getenv("HOME")) + "/.config/vclient.conf";

  int opt;
  while ((opt = getopt(argc, argv, "h:i:o:c:")) != -1) {
    switch (opt) {
    case 'h':
      cout << "Использование: " << argv[0]
           << " -i <файл входных данных> -o <файл результатов> -с <файл конфигурации>" << endl;
      return 0;
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
      cerr << "Неверные параметры." << endl;
      return 1;
    }
  }

  // Проверяем, что все необходимые параметры указаны
  if (input_file.empty() || output_file.empty()) {
    cerr
        << "Ошибка: файл входных данных, файл результатов и файл конфигурации должны быть указаны."
        << endl;
    return 1;
  }

  try {
    auto vectors = read_vectors_from_file(input_file);
    int sock = connect_to_server(); // Используем фиксированный адрес и порт

    string login, password;
    ifstream config(config_file);
    if (config.is_open()) {
      string line;
      if (getline(config, line)) {
        size_t delimiter_pos = line.find(':');
        if (delimiter_pos != string::npos) {
          login = line.substr(0, delimiter_pos);
          password = line.substr(delimiter_pos + 1);
        } else {
          throw runtime_error(
              "Неверный формат файла конфигурации. Ожидается 'user:password'.");
        }
      }
    } else {
      throw runtime_error("Ошибка открытия файла конфигурации: " + config_file);
    }

    cout << "Логин: " << login
         << endl;
    authenticate_client(sock, login, password);
    send_vectors(sock, vectors);

    close(sock);
    cout << "Соединение с сервером закрыто." << endl;
  } catch (const exception &e) {
    cerr << "Ошибка: " << e.what() << endl;
    return 1;
  }

  return 0;
}
