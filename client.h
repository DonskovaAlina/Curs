#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <string>

#define DEFAULT_PORT 33333
#define SERVER_ADDRESS "127.0.0.1" // Фиксированный адрес сервера
#define SALT_SIZE 16


std::string SHA224_hash(const std::string &hsh);
int connect_to_server();
void authenticate_client(int sock, const std::string &login, const std::string &password);
void send_vectors(int sock, const std::vector<std::vector<float>> &vectors, const std::string &output_file);

#endif // CLIENT_H
