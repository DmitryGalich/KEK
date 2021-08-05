#include "callback.h"

#include <iostream>

void Callback::BinaryPacket(const char* data, std::size_t size) {
  std::cout << "Binary packet(" << size << "): " << data << std::endl;
  binary_packet_ = data;
  binary_packet_size_ = size;
}

void Callback::TextPacket(const char* data, std::size_t size) {
  std::cout << "Text packet(" << size << "): " << data << std::endl;
  text_packet_ = data;
  text_packet_size_ = size;
}

const std::pair<const std::string, const std::size_t>
Callback::GetBinaryPacket() const {
  return {binary_packet_, binary_packet_size_};
}

const std::pair<const std::string, const std::size_t> Callback::GetTextPacket()
    const {
  return {text_packet_, text_packet_size_};
}
