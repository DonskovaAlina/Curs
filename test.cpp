#include "client.h"
#include "interface.h"
#include <UnitTest++/UnitTest++.h>
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
    string expected_hash = "3598976BFD77124E19B8AF1E37C9E424A3CEFDABDCB68B7E038"
                           "5D01F"; // Правильный хэш для "test_password"
    string actual_hash = SHA224_hash(input);

    CHECK_EQUAL(expected_hash, actual_hash);
  }

  // Тестирование функции authenticate_client кор данные
  TEST(TestAuthClient) {
    int imit_sock = 1;
    string login = "user";
    string password = "P@ssW0rd";
    CHECK_THROW(authenticate_client(imit_sock, login, password), runtime_error);
  }

  // Тестирование функции send_vectors
  TEST(TestSendVectors) {
    int imit_sock = 1;
    vector<vector<float>> vectors = {{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f}};
    string output_file = "result.txt";
    CHECK_THROW(send_vectors(imit_sock, vectors, output_file), runtime_error);
  }

  // Тест на отправку вектора с отрицательными числами
  TEST(TestSendVectorsNeg) {
    int imit_sock = 1;
    vector<vector<float>> vectors = {{-1.0f, -2.0f, -3.0f}, {-4.0f, -5.0f}};
    string output_file = "result.txt";
    CHECK_THROW(send_vectors(imit_sock, vectors, output_file), runtime_error);
  }

  // Тест на отправку вектора с нулевыми значениями
  TEST(TestSendVectorsZero) {
    int imit_sock = 1;
    vector<vector<float>> vectors = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}};
    string output_file = "result.txt";
    CHECK_THROW(send_vectors(imit_sock, vectors, output_file), runtime_error);
  }

  // Тест на отправку вектора с большими числами
  TEST(TestSendVectorsLarge) {
    int imit_sock = 1;
    vector<vector<float>> vectors = {{1e6f, 2e6f, 3e6f}, {4e6f, 5e6f}};
    string output_file = "result.txt";
    CHECK_THROW(send_vectors(imit_sock, vectors, output_file), runtime_error);
  }

  // Тест на отправку вектора с одним элементом
  TEST(TestSendSingleElemVector) {
    int imit_sock = 1;
    vector<vector<float>> vectors = {{1.0f}};
    string output_file = "result.txt";
    CHECK_THROW(send_vectors(imit_sock, vectors, output_file), runtime_error);
  }
  // Тестирование функции send_vectors с некорректным сокетом
TEST(TestSendVectorsInvalidSocket) {
  int invalid_sock = -1;
  vector<vector<float>> vectors = {{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f}};
  string output_file = "test_output.txt";
  CHECK_THROW(send_vectors(invalid_sock, vectors, output_file), runtime_error);
}

// Тестирование функции authenticate_client с некорректным логином
TEST(TestAuthClientInvalidLogin) {
  int imit_sock = 1;
  string login = "user";
  string password = "P@ssW0rd";
  char invalid_salt[SALT_SIZE] = "invalid_salt";
  send(imit_sock, login.c_str(), login.size(), 0);
  send(imit_sock, invalid_salt, SALT_SIZE, 0);
  string hash_output = "invalid_hash";
  send(imit_sock, hash_output.c_str(), hash_output.size(), 0);
  char response[] = "ERR";
  send(imit_sock, response, sizeof(response), 0);
  CHECK_THROW(authenticate_client(imit_sock, login, password), runtime_error);
}
}

// SUITE для тестирования функций из interface.cpp
SUITE(InterfaceTests) {
  // Тестирование функции read_vectors_from_file
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
    vector<vector<float>> vectors = read_vectors_from_file(filename);
    CHECK_EQUAL(2, vectors.size());
    CHECK_ARRAY_EQUAL(vec1, vectors[0].data(), 3);
    CHECK_ARRAY_EQUAL(vec2, vectors[1].data(), 2);
  }

  // Тестирование функции read_config
  TEST(TestReadConfig) {
    string config_file = "test_config.conf";
    string login, password;
    ofstream config(config_file);
    config << "user:P@ssW0rd";
    config.close();
    read_config(config_file, login, password);
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
    CHECK_THROW(read_config(config_file, login, password), runtime_error);
  }

  // Тест на отсутствие конфигурационного файла
  TEST(TestReadConfigNotFound) {
    string config_file = "nonexistent_config.conf";
    string login, password;
    CHECK_THROW(read_config(config_file, login, password), runtime_error);
  }

  // Тест на вывод справки с параметром -h
  TEST(TestCmdLineHelp) {
    int argc = 2;
    const char* argv[] = {"program_name", "-h"};
    string input_file, output_file, config_file;
    CHECK_THROW(parse_command_line_arguments(argc, const_cast<char**>(argv), input_file, output_file, config_file), runtime_error);
  }

  // Тест на вывод справки с параметром --help
  TEST(TestCmdLineLongHelp) {
    int argc = 2;
    const char* argv[] = {"program_name", "--help"};
    string input_file, output_file, config_file;
    CHECK_THROW(parse_command_line_arguments(argc, const_cast<char**>(argv), input_file, output_file, config_file), runtime_error);
  }

  // Тест на некорректный параметр
  TEST(TestCmdLineInvalidParam) {
    int argc = 2;
    const char* argv[] = {"program_name", "-x"};
    string input_file, output_file, config_file;
    CHECK_THROW(parse_command_line_arguments(argc, const_cast<char**>(argv), input_file, output_file, config_file), runtime_error);
  }

  // Тест на отсутствие параметра -i
  TEST(TestCmdLineMissingInput) {
    int argc = 4;
    const char* argv[] = {"program_name", "-o", "output.txt", "-c", "config.conf"};
    string input_file, output_file, config_file;
    CHECK_THROW(parse_command_line_arguments(argc, const_cast<char**>(argv), input_file, output_file, config_file), runtime_error);
  }

  // Тест на отсутствие параметра -o
  TEST(TestCmdLineMissingOutput) {
    int argc = 4;
    const char* argv[] = {"program_name", "-i", "input.bin", "-c", "config.conf"};
    string input_file, output_file, config_file;
    CHECK_THROW(parse_command_line_arguments(argc, const_cast<char**>(argv), input_file, output_file, config_file), runtime_error);
  }

  // Тест на отсутствие параметра -c
  TEST(TestCmdLineMissingConfig) {
    int argc = 4;
    const char* argv[] = {"program_name", "-i", "input.bin", "-o", "output.txt"};
    string input_file, output_file, config_file;
    CHECK_THROW(parse_command_line_arguments(argc, const_cast<char**>(argv), input_file, output_file, config_file), runtime_error);
  }
  
  // Тестирование функции read_vectors_from_file с пустым файлом
TEST(TestReadVectorsEmptyFile) {
  ofstream empty_file("empty.bin", ios::binary);
  empty_file.close();
  CHECK_THROW(read_vectors_from_file("empty.bin"), runtime_error);
  remove("empty.bin");
}

// Тестирование функции read_vectors_from_file с корректным файлом
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
  auto vectors = read_vectors_from_file("test_vectors.bin");
  CHECK_EQUAL(vectors.size(), 2);
  CHECK_EQUAL(vectors[0].size(), 3);
  CHECK_EQUAL(vectors[1].size(), 2);
  remove("test_vectors.bin");
}
// Тестирование функции read_vectors_from_file с некорректным файлом
TEST(TestReadVectorsInvalidFile) {
  CHECK_THROW(read_vectors_from_file("invalid_file.bin"), runtime_error);
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
  // Читаем векторы из файла
  vector<vector<float>> vectors = read_vectors_from_file(filename);

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

  // Читаем векторы из файла
  vector<vector<float>> vectors = read_vectors_from_file(filename);

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
  vector<vector<float>> vectors = read_vectors_from_file(filename);
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

  vector<vector<float>> vectors = read_vectors_from_file(filename);

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

  vector<vector<float>> vectors = read_vectors_from_file(filename);

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

  vector<vector<float>> vectors = read_vectors_from_file(filename);
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
  vector<vector<float>> vectors = read_vectors_from_file(filename);

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
  vector<vector<float>> vectors = read_vectors_from_file(filename);

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
  vector<vector<float>> vectors = read_vectors_from_file(filename);

  CHECK_EQUAL(1, vectors.size());
  CHECK_EQUAL(1, vectors[0].size());

  CHECK_EQUAL(1.0f, vectors[0][0]);
}
}

int main() { return UnitTest::RunAllTests(); }