#include "client.h"
#include "interface.h"
#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporterStdout.h>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <vector>

using namespace std;

// SUITE для тестирования функций из client.cpp
SUITE(ClientTests) {
  // Тестирование функции SHA224_hash
  TEST(TestSHA224Hash) {
    string input = "test_password";
    string expected_hash = "3598976BFD77124E19B8AF1E37C9E424A3CEFDABDCB68B7E0385D01F"; // Правильный хэш для "test_password"

    Client client("127.0.0.1", 33333); // Создаем объект класса Client
    string actual_hash = client.SHA224_hash(input);

    CHECK_EQUAL(expected_hash, actual_hash);
  }

  // Тестирование функции Authenticate с некорректными данными
  TEST(TestAuthClient) {
    string login = "user";
    string password = "wrong_password";

    Client client("127.0.0.1", 33333); // Создаем объект класса Client
    client.Connect(); // Подключаемся к серверу
    CHECK_THROW(client.Authenticate(login, password), runtime_error);
  }

  // Тестирование функции Authenticate с некорректным логином
  TEST(TestAuthClientInvalidLogin) {
    string login = "Uuser";
    string password = "P@ssW0rd";

    Client client("127.0.0.1", 33333); // Создаем объект класса Client
    client.Connect(); // Подключаемся к серверу
    CHECK_THROW(client.Authenticate(login, password), runtime_error);
  }
}

// SUITE для тестирования функций из interface.cpp
SUITE(InterfaceTests) {
  // Тестирование функции ReadVectorsFromFile
  TEST(TestReadVectors) {
    string filename = "test_input_vectors.bin";
    ofstream file(filename, ios::binary);
    uint32_t N = 2;
    file.write(reinterpret_cast<char *>(&N), sizeof(N));

    uint32_t S1 = 3;
    file.write(reinterpret_cast<char *>(&S1), sizeof(S1));
    float vec1[] = {1.0f, 2.0f, 3.0f};
    file.write(reinterpret_cast<char *>(vec1), S1 * sizeof(float));

    uint32_t S2 = 2;
    file.write(reinterpret_cast<char *>(&S2), sizeof(S2));
    float vec2[] = {4.0f, 5.0f};
    file.write(reinterpret_cast<char *>(vec2), S2 * sizeof(float));

    file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    vector<vector<float>> vectors = interface.ReadVectorsFromFile(filename);
    CHECK_EQUAL(2, vectors.size());
    CHECK_ARRAY_EQUAL(vec1, vectors[0].data(), 3);
    CHECK_ARRAY_EQUAL(vec2, vectors[1].data(), 2);
  }

  // Тестирование функции ReadConfigFile
  TEST(TestReadConfig) {
    string config_file = "test_config.conf";
    string login, password;
    ofstream config(config_file);
    config << "user:P@ssW0rd";
    config.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    interface.ReadConfigFile(config_file, login, password);
    CHECK_EQUAL("user", login);
    CHECK_EQUAL("P@ssW0rd", password);
  }

  // Тест на неправильный формат конфигурационного файла
  TEST(TestReadConfigInvalid) {
    string config_file = "invalid_config.conf";
    string login, password;
    ofstream config(config_file);
    config << "user=P@ssW0rd";
    config.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    CHECK_THROW(interface.ReadConfigFile(config_file, login, password), runtime_error);
  }

  // Тест на отсутствие конфигурационного файла
  TEST(TestReadConfigNotFound) {
    string config_file = "nonexistent_config.conf";
    string login, password;

    Interface interface(0, nullptr); // Создаем объект класса Interface
    CHECK_THROW(interface.ReadConfigFile(config_file, login, password), runtime_error);
  }

  // Тест на некорректный параметр
  TEST(TestCmdLineInvalidParam) {
    int argc = 2;
    const char *argv[] = {"program_name", "-x"};

    Interface interface(argc, const_cast<char **>(argv)); // Создаем объект класса Interface
    CHECK_THROW(interface.ParseCommandLine(), runtime_error);
  }

  // Тест на отсутствие параметра -i
  TEST(TestCmdLineMissingInput) {
    int argc = 4;
    const char *argv[] = {"program_name", "-o", "output.txt", "-c", "config.conf"};

    Interface interface(argc, const_cast<char **>(argv)); // Создаем объект класса Interface
    CHECK_THROW(interface.ParseCommandLine(), runtime_error);
  }

  // Тест на отсутствие параметра -o
  TEST(TestCmdLineMissingOutput) {
    int argc = 4;
    const char *argv[] = {"program_name", "-i", "input.bin", "-c", "config.conf"};

    Interface interface(argc, const_cast<char **>(argv)); // Создаем объект класса Interface
    CHECK_THROW(interface.ParseCommandLine(), runtime_error);
  }

  // Тест на отсутствие параметра -c
  TEST(TestCmdLineMissingConfig) {
    int argc = 4;
    const char *argv[] = {"program_name", "-i", "input.bin", "-o", "output.txt"};

    Interface interface(argc, const_cast<char **>(argv)); // Создаем объект класса Interface
    CHECK_THROW(interface.ParseCommandLine(), runtime_error);
  }

  // Тест на отсутствие параметра -a
  TEST(TestCmdLineMissingServerAddress) {
    int argc = 4;
    const char *argv[] = {"program_name", "-i", "input.bin", "-o", "output.txt", "-c", "config.conf"};

    Interface interface(argc, const_cast<char **>(argv)); // Создаем объект класса Interface
    CHECK_THROW(interface.ParseCommandLine(), runtime_error);
  }

  // Тест на отсутствие параметра -p
  TEST(TestCmdLineMissingServerPort) {
    int argc = 4;
    const char *argv[] = {"program_name", "-i", "input.bin", "-o", "output.txt", "-c", "config.conf", "-a", "127.0.0.1"};

    Interface interface(argc, const_cast<char **>(argv)); // Создаем объект класса Interface
    CHECK_THROW(interface.ParseCommandLine(), runtime_error);
  }

  // Тест на недопустимое значение порта (отрицательное)
  TEST(TestCmdLineInvalidPortNegative) {
    int argc = 6;
    const char *argv[] = {"program_name", "-i", "input.bin", "-o", "output.txt", "-c", "config.conf", "-a", "127.0.0.1", "-p", "-1"};

    Interface interface(argc, const_cast<char **>(argv)); // Создаем объект класса Interface
    CHECK_THROW(interface.ParseCommandLine(), runtime_error);
  }

  // Тест на недопустимое значение порта (больше 65535)
  TEST(TestCmdLineInvalidPortTooLarge) {
    int argc = 6;
    const char *argv[] = {"program_name", "-i", "input.bin", "-o", "output.txt", "-c", "config.conf", "-a", "127.0.0.1", "-p", "65536"};

    Interface interface(argc, const_cast<char **>(argv)); // Создаем объект класса Interface
    CHECK_THROW(interface.ParseCommandLine(), runtime_error);
  }

  // Тестирование функции ReadVectorsFromFile с корректным файлом
  TEST(TestReadVectorsValidFile) {
    ofstream test_file("test_vectors.bin", ios::binary);
    uint32_t N = 2;
    test_file.write(reinterpret_cast<char *>(&N), sizeof(N));

    uint32_t S1 = 3;
    float v1[] = {1.0f, 2.0f, 3.0f};
    test_file.write(reinterpret_cast<char *>(&S1), sizeof(S1));
    test_file.write(reinterpret_cast<char *>(v1), sizeof(v1));

    uint32_t S2 = 2;
    float v2[] = {4.0f, 5.0f};
    test_file.write(reinterpret_cast<char *>(&S2), sizeof(S2));
    test_file.write(reinterpret_cast<char *>(v2), sizeof(v2));

    test_file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    auto vectors = interface.ReadVectorsFromFile("test_vectors.bin");
    CHECK_EQUAL(vectors.size(), 2);
    CHECK_EQUAL(vectors[0].size(), 3);
    CHECK_EQUAL(vectors[1].size(), 2);
    remove("test_vectors.bin");
  }

  // Тестирование функции ReadVectorsFromFile с некорректным файлом
  TEST(TestReadVectorsInvalidFile) {
    Interface interface(0, nullptr); // Создаем объект класса Interface
    CHECK_THROW(interface.ReadVectorsFromFile("invalid_file.bin"), runtime_error);
  }

  // Тест на чтение векторов из файла с отрицательными числами
  TEST(TestReadVectorsFromFileWithNegativeNums) {
    string filename = "negative_vectors.bin";
    ofstream file(filename, ios::binary);
    uint32_t N = 2;
    file.write(reinterpret_cast<char *>(&N), sizeof(N));

    uint32_t S1 = 3;
    file.write(reinterpret_cast<char *>(&S1), sizeof(S1));
    float vec1[] = {-1.0f, -2.0f, -3.0f};
    file.write(reinterpret_cast<char *>(vec1), S1 * sizeof(float));

    uint32_t S2 = 2;
    file.write(reinterpret_cast<char *>(&S2), sizeof(S2));
    float vec2[] = {-4.0f, -5.0f};
    file.write(reinterpret_cast<char *>(vec2), S2 * sizeof(float));
    file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    vector<vector<float>> vectors = interface.ReadVectorsFromFile(filename);

    CHECK_EQUAL(2, vectors.size());
    CHECK_EQUAL(3, vectors[0].size());
    CHECK_EQUAL(2, vectors[1].size());

    for (size_t i = 0; i < vectors[0].size(); ++i) {
      CHECK_EQUAL(vec1[i], vectors[0][i]);
    }
    for (size_t i = 0; i < vectors[1].size(); ++i) {
      CHECK_EQUAL(vec2[i], vectors[1][i]);
    }
  }

  // Тест на чтение векторов из файла с нулевыми значениями
  TEST(TestReadVectFromFileWithZeroValues) {
    string filename = "zero_vectors.bin";

    ofstream file(filename, ios::binary);
    uint32_t N = 2;
    file.write(reinterpret_cast<char *>(&N), sizeof(N));

    uint32_t S1 = 3;
    file.write(reinterpret_cast<char *>(&S1), sizeof(S1));
    float vec1[] = {0.0f, 0.0f, 0.0f};
    file.write(reinterpret_cast<char *>(vec1), S1 * sizeof(float));

    uint32_t S2 = 2;
    file.write(reinterpret_cast<char *>(&S2), sizeof(S2));
    float vec2[] = {0.0f, 0.0f};
    file.write(reinterpret_cast<char *>(vec2), S2 * sizeof(float));

    file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    vector<vector<float>> vectors = interface.ReadVectorsFromFile(filename);

    CHECK_EQUAL(2, vectors.size());
    CHECK_EQUAL(3, vectors[0].size());
    CHECK_EQUAL(2, vectors[1].size());

    for (size_t i = 0; i < vectors[0].size(); ++i) {
      CHECK_EQUAL(vec1[i], vectors[0][i]);
    }
    for (size_t i = 0; i < vectors[1].size(); ++i) {
      CHECK_EQUAL(vec2[i], vectors[1][i]);
    }
  }

  // Тест на чтение векторов из файла с большими числами
  TEST(TestReadVectFromFileWithLargeNum) {
    string filename = "large_vectors.bin";

    ofstream file(filename, ios::binary);
    uint32_t N = 2;
    file.write(reinterpret_cast<char *>(&N), sizeof(N));

    uint32_t S1 = 3;
    file.write(reinterpret_cast<char *>(&S1), sizeof(S1));
    float vec1[] = {1e6f, 2e6f, 3e6f};
    file.write(reinterpret_cast<char *>(vec1), S1 * sizeof(float));

    uint32_t S2 = 2;
    file.write(reinterpret_cast<char *>(&S2), sizeof(S2));
    float vec2[] = {4e6f, 5e6f};
    file.write(reinterpret_cast<char *>(vec2), S2 * sizeof(float));

    file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    vector<vector<float>> vectors = interface.ReadVectorsFromFile(filename);
    CHECK_EQUAL(2, vectors.size());
    CHECK_EQUAL(3, vectors[0].size());
    CHECK_EQUAL(2, vectors[1].size());

    for (size_t i = 0; i < vectors[0].size(); ++i) {
      CHECK_EQUAL(vec1[i], vectors[0][i]);
    }
    for (size_t i = 0; i < vectors[1].size(); ++i) {
      CHECK_EQUAL(vec2[i], vectors[1][i]);
    }
  }

  // Тест на чтение векторов из файла с одним элементом
  TEST(TestReadVectorsFromFileWithSingleElem) {
    string filename = "single_element_vectors.bin";
    ofstream file(filename, ios::binary);
    uint32_t N = 2;
    file.write(reinterpret_cast<char *>(&N), sizeof(N));

    uint32_t S1 = 1;
    file.write(reinterpret_cast<char *>(&S1), sizeof(S1));
    float vec1[] = {1.0f};
    file.write(reinterpret_cast<char *>(vec1), S1 * sizeof(float));

    uint32_t S2 = 1;
    file.write(reinterpret_cast<char *>(&S2), sizeof(S2));
    float vec2[] = {2.0f};
    file.write(reinterpret_cast<char *>(vec2), S2 * sizeof(float));

    file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    vector<vector<float>> vectors = interface.ReadVectorsFromFile(filename);

    CHECK_EQUAL(2, vectors.size());
    CHECK_EQUAL(1, vectors[0].size());
    CHECK_EQUAL(1, vectors[1].size());

    CHECK_EQUAL(vec1[0], vectors[0][0]);
    CHECK_EQUAL(vec2[0], vectors[1][0]);
  }

  // Тест на чтение векторов из файла с очень большим количеством элементов
  TEST(TestReadVectorsFromFileWithVeryLargeNumOfElem) {
    string filename = "large_elements_vectors.bin";

    ofstream file(filename, ios::binary);
    uint32_t N = 2;
    file.write(reinterpret_cast<char *>(&N), sizeof(N));

    uint32_t S1 = 1000;
    file.write(reinterpret_cast<char *>(&S1), sizeof(S1));
    vector<float> vec1(S1, 1.0f);
    file.write(reinterpret_cast<char *>(vec1.data()), S1 * sizeof(float));

    uint32_t S2 = 1000;
    file.write(reinterpret_cast<char *>(&S2), sizeof(S2));
    vector<float> vec2(S2, 2.0f);
    file.write(reinterpret_cast<char *>(vec2.data()), S2 * sizeof(float));

    file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    vector<vector<float>> vectors = interface.ReadVectorsFromFile(filename);

    CHECK_EQUAL(2, vectors.size());
    CHECK_EQUAL(1000, vectors[0].size());
    CHECK_EQUAL(1000, vectors[1].size());

    for (size_t i = 0; i < vectors[0].size(); ++i) {
      CHECK_EQUAL(1.0f, vectors[0][i]);
    }
    for (size_t i = 0; i < vectors[1].size(); ++i) {
      CHECK_EQUAL(2.0f, vectors[1][i]);
    }
  }

  // Тест на чтение векторов из файла с очень маленьким количеством элементов
  TEST(TestReadVectorsFromFileWithVerySmallNumOfElem) {
    string filename = "small_elements_vectors.bin";

    ofstream file(filename, ios::binary);
    uint32_t N = 2;
    file.write(reinterpret_cast<char *>(&N), sizeof(N));

    uint32_t S1 = 1;
    file.write(reinterpret_cast<char *>(&S1), sizeof(S1));
    float vec1[] = {1.0f};
    file.write(reinterpret_cast<char *>(vec1), S1 * sizeof(float));

    uint32_t S2 = 1;
    file.write(reinterpret_cast<char *>(&S2), sizeof(S2));
    float vec2[] = {2.0f};
    file.write(reinterpret_cast<char *>(vec2), S2 * sizeof(float));

    file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    vector<vector<float>> vectors = interface.ReadVectorsFromFile(filename);
    CHECK_EQUAL(2, vectors.size());
    CHECK_EQUAL(1, vectors[0].size());
    CHECK_EQUAL(1, vectors[1].size());

    CHECK_EQUAL(vec1[0], vectors[0][0]);
    CHECK_EQUAL(vec2[0], vectors[1][0]);
  }

  // Тест на чтение векторов из файла с очень большим количеством векторов
  TEST(TestReadVectorsFromFileWithVeryLargeNumOfVect) {
    string filename = "large_vectors_vectors.bin";

    ofstream file(filename, ios::binary);
    uint32_t N = 1000;
    file.write(reinterpret_cast<char *>(&N), sizeof(N));

    for (uint32_t i = 0; i < N; ++i) {
      uint32_t S = 3;
      file.write(reinterpret_cast<char *>(&S), sizeof(S));
      float vec[] = {1.0f, 2.0f, 3.0f};
      file.write(reinterpret_cast<char *>(vec), S * sizeof(float));
    }

    file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    vector<vector<float>> vectors = interface.ReadVectorsFromFile(filename);

    CHECK_EQUAL(1000, vectors.size());
    CHECK_EQUAL(3, vectors[0].size());

    for (size_t i = 0; i < vectors[0].size(); ++i) {
      CHECK_EQUAL(1.0f + i, vectors[0][i]);
    }
  }

  // Тест на чтение векторов из файла с очень маленьким количеством векторов
  TEST(TestReadVectorsFromFileWithVerySmallNumOfVect) {
    string filename = "small_vectors_vectors.bin";

    ofstream file(filename, ios::binary);
    uint32_t N = 1;
    file.write(reinterpret_cast<char *>(&N), sizeof(N));

    uint32_t S = 3;
    file.write(reinterpret_cast<char *>(&S), sizeof(S));
    float vec[] = {1.0f, 2.0f, 3.0f};
    file.write(reinterpret_cast<char *>(vec), S * sizeof(float));

    file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    vector<vector<float>> vectors = interface.ReadVectorsFromFile(filename);

    CHECK_EQUAL(1, vectors.size());
    CHECK_EQUAL(3, vectors[0].size());

    for (size_t i = 0; i < vectors[0].size(); ++i) {
      CHECK_EQUAL(1.0f + i, vectors[0][i]);
    }
  }

  // Тест на чтение векторов из файла с очень маленьким количеством векторов и
  // элементов
  TEST(TestReadVectorsFromFileWithVerySmallNumOfVecAndElem) {
    string filename = "small_vectors_elements_vectors.bin";

    ofstream file(filename, ios::binary);
    uint32_t N = 1;
    file.write(reinterpret_cast<char *>(&N), sizeof(N));

    uint32_t S = 1;
    file.write(reinterpret_cast<char *>(&S), sizeof(S));
    float vec[] = {1.0f};
    file.write(reinterpret_cast<char *>(vec), S * sizeof(float));

    file.close();

    Interface interface(0, nullptr); // Создаем объект класса Interface
    vector<vector<float>> vectors = interface.ReadVectorsFromFile(filename);

    CHECK_EQUAL(1, vectors.size());
    CHECK_EQUAL(1, vectors[0].size());

    CHECK_EQUAL(1.0f, vectors[0][0]);
  } 
}

int main() {     // Создаем объект для вывода статистики тестов в стандартный поток вывода
    UnitTest::TestReporterStdout reporter;
    UnitTest::TestRunner runner(reporter);

    // Запускаем все тесты
    int result = runner.RunTestsIf(UnitTest::Test::GetTestList(), NULL, UnitTest::True(), 0);

    return result; }