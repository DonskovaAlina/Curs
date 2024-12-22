#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>

void parse_command_line_arguments(int argc, char *argv[], std::string &input_file, std::string &output_file, std::string &config_file);
std::vector<std::vector<float>> read_vectors_from_file(const std::string &filename);
void read_config(const std::string &config_file, std::string &login, std::string &password);

#endif // INTERFACE_H
