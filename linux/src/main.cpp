#include <algorithm>
#include <chrono>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <cmath>
#include <fstream>

#include "type.hpp"
#include "utils.hpp"

static volatile std::sig_atomic_t g_stop = 0;

void handle_sigint(int) { g_stop = 1; }

ParsedData parse_text_file(const std::string &path) {
  ParsedData out;
  out.start_ack = std::vector<uint8_t>{0xA5, 0x5A, 0x05, 0x00, 0x00, 0x40, 0x81};

  out.cmd_to_resp[std::string("\xA5\x92", 2)] = std::vector<uint8_t>{0xA5, 0x5A, 0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00};
  out.cmd_to_resp[std::string("\xA5\x90", 2)] = std::vector<uint8_t>{0xA5, 0x5A, 0x14, 0x00, 0x00, 0x00, 0x04, 0x97, 0x02, 0x01, 0x01, 0x02, 0x00, 0x02, 0x05, 0x00, 0x04, 0x02, 0x04, 0x00, 0x00, 0x00, 0x09, 0x00, 0x01, 0x09, 0x04};
  out.cmd_to_resp[std::string("\xA5\x98", 2)] = std::vector<uint8_t>{0xA5, 0x5A, 0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00};
  out.cmd_to_resp[std::string("\xA5\x60", 2)] = std::vector<uint8_t>{0xA5, 0x5A, 0x05, 0x00, 0x00, 0x40, 0x81};

  FILE *fp = fopen(path.c_str(), "r");
  if (!fp) {
    throw std::runtime_error("Failed to open text file: " + path);
  }

  char *lineptr = nullptr;
  size_t n = 0;
  ssize_t len;

  while ((len = getline(&lineptr, &n, fp)) != -1) {
    std::string l(lineptr, len);
    std::vector<uint8_t> data = hex_to_bytes_from_tokens(
      split_ws(l)
    );
    
    if (data.size() >= 2 && data[0] == 0xAA && data[1] == 0x55) {
      out.frames.emplace_back(std::move(data));
    }
  }

  if (lineptr)
    free(lineptr);
  fclose(fp);
  return out;
}


int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <tty device path> <target file path>\n";
    return 1;
  }
  std::string tty = argv[1];
  std::string text = argv[2];

  double pps = 90.0;

  try {
    ParsedData parsed = parse_text_file(text);
    std::cerr << "Frames loaded: " << parsed.frames.size() << "\n";

    int fd = open(tty.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
      perror("open tty");
      return 1;
    }

    std::signal(SIGINT, handle_sigint);

    std::vector<uint8_t> rbuf;
    rbuf.reserve(1024);
    bool scanning = false;
    size_t frame_idx = 0; // for text replay
    using clock = std::chrono::steady_clock;
    auto period = std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(1.0 / pps));
    auto next_send = clock::now();
    auto t0 = next_send;
    const uint8_t LSN = 40;

    while (!g_stop) {
      struct pollfd pfd;
      pfd.fd = fd;
      pfd.events = POLLIN;
      int timeout_ms = 10; // 10ms slices
      int pr = poll(&pfd, 1, timeout_ms);
      if (pr > 0 && (pfd.revents & POLLIN)) {
        uint8_t buf[64];
        ssize_t nr = read(fd, buf, sizeof(buf));
        if (nr > 0) {
          rbuf.insert(rbuf.end(), buf, buf + nr);
        }
      }

      while (rbuf.size() >= 2) {
        if (rbuf[0] != 0xA5) {
          rbuf.erase(rbuf.begin());
          continue;
        }
        std::vector<uint8_t> cmd{rbuf[0], rbuf[1]};
        rbuf.erase(rbuf.begin(), rbuf.begin() + 2);

        std::string key(reinterpret_cast<const char *>(cmd.data()), cmd.size());
        auto it = parsed.cmd_to_resp.find(key);
        const std::vector<uint8_t> *resp = (it != parsed.cmd_to_resp.end()) ? &it->second : nullptr;

        if (cmd.size() >= 2 && cmd[0] == 0xA5 && cmd[1] == 0x60) {
          if (!resp && parsed.start_ack)
            resp = &*parsed.start_ack;
          if (resp && !resp->empty()) {
            (void)write(fd, resp->data(), resp->size());
          }
          scanning = true;
          frame_idx = 0;
          next_send = clock::now();
          t0 = next_send;
        } else if (cmd.size() >= 2 && cmd[0] == 0xA5 && cmd[1] == 0x65) {
          scanning = false;
        } else {
          if (resp && !resp->empty()) {
            (void)write(fd, resp->data(), resp->size());
          }
        }
      }

      if (scanning) {
        auto now = clock::now();
        if (now >= next_send) {
           if (!parsed.frames.empty()) {
            const auto &f = parsed.frames[frame_idx];
            if (!f.empty()) {
              (void)write(fd, f.data(), f.size());
            }
            frame_idx = (frame_idx + 1) % parsed.frames.size();
          }
          next_send += period;
          if (next_send < now - 5 * period)
            next_send = now + period;
        }
      }
    }

    close(fd);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
