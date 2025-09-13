#ifndef TYPE_HPP
#define TYPE_HPP

#include <cstdint>
#include <cstring>
#include <map>
#include <optional>
#include <string>
#include <sstream>
#include <vector>

struct ParsedData {
  std::map<std::string, std::vector<uint8_t>> cmd_to_resp; // key: raw bytes stored in std::string
  std::optional<std::vector<uint8_t>> start_ack;
  std::vector<std::vector<uint8_t>> frames;
};


#endif // TYPE_HPP