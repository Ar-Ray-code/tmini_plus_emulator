#ifndef UTILS_HPP
#define UTILS_HPP

#include "type.hpp"

std::vector<uint8_t> hex_to_bytes_from_tokens(const std::vector<std::string> &);
std::vector<std::string> split_ws(const std::string &);

#endif // UTILS_HPP