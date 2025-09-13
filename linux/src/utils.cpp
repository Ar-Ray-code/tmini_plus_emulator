#include "utils.hpp"

std::vector<uint8_t> hex_to_bytes_from_tokens(const std::vector<std::string> &tokens) {
  std::vector<uint8_t> out;
  for (const auto &t : tokens) {
    if (t.empty())
      continue;
    if (t.size() > 2) {
      bool all_hex = true;
      for (char c : t) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
          all_hex = false;
          break;
        }
      }
      if (all_hex && (t.size() % 2 == 0)) {
        for (size_t i = 0; i < t.size(); i += 2) {
          auto byte_str = t.substr(i, 2);
          uint8_t val = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
          out.push_back(val);
        }
        continue;
      }
    }
    uint8_t val = static_cast<uint8_t>(std::stoul(t, nullptr, 16));
    out.push_back(val);
  }
  return out;
}

std::vector<std::string> split_ws(const std::string &s) {
  std::istringstream iss(s);
  std::vector<std::string> r;
  std::string tok;
  while (iss >> tok)
    r.push_back(tok);
  return r;
}