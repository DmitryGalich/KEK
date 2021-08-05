#include "receiver.h"

#include <algorithm>
#include <iostream>
#include <string>

namespace {

const std::string kTextPacketEndSequence_{"\r\n\r\n"};
const char kBinaryPacketFirstByte_{0x24};

enum class PacketType { kUnknown = 0, kBinary, kText };

struct BinaryPacketReceivingInfo {
  union MessageSize {
    int number;
    char bytes_array[4];
  } size_;

  size_t packet_bytes_count_{0};
  std::string usefull_message_;
  bool is_message_completely_received_{false};

  void Reset() {
    size_.number = 0;
    packet_bytes_count_ = 0;
    usefull_message_.clear();
    is_message_completely_received_ = false;
  }
};

struct TextPacketReceivingInfo {
  std::string usefull_message_;
  size_t end_sequence_index_;

  void Reset() {
    end_sequence_index_ = 0;
    usefull_message_.clear();
  }
};

}  // namespace

class Receiver::Implementation {
 public:
  Implementation(const std::shared_ptr<ICallback> callback);
  ~Implementation() = default;

  void Receive(const char* data, std::size_t size);

 private:
  const std::shared_ptr<ICallback> kCallback_;

  void ProcessPacketType(const std::string& data_string, const size_t index);
  void ProcessBinaryPacket(const std::string& data_string, size_t& index);
  void ProcessTextPacket(const std::string& data_string, size_t& index);

  void ProcessCompletedPacket(const std::string& message);

  bool is_receiving_in_process_{false};
  PacketType packet_type_{PacketType::kUnknown};
};

Receiver::Implementation::Implementation(
    const std::shared_ptr<ICallback> callback)
    : kCallback_(callback) {}

void Receiver::Implementation::Receive(const char* data, std::size_t /*size*/) {
  const std::string data_string{data};
  size_t index = 0;

  do {
    ProcessPacketType(data_string, index);

    if (packet_type_ == PacketType::kBinary)
      ProcessBinaryPacket(data_string, index);
    else
      ProcessTextPacket(data_string, index);

  } while (index < data_string.size());
}

void Receiver::Implementation::ProcessPacketType(const std::string& data_string,
                                                 const size_t index) {
  if (!is_receiving_in_process_) {
    is_receiving_in_process_ = true;

    if (!data_string.empty() &&
        (data_string.at(index) == kBinaryPacketFirstByte_))
      packet_type_ = PacketType::kBinary;
    else
      packet_type_ = PacketType::kText;
  }
}

void Receiver::Implementation::ProcessBinaryPacket(
    const std::string& data_string,
    size_t& index) {
  static BinaryPacketReceivingInfo info;

  const auto ProcessFirstByte = [&]() {
    if (data_string.at(index) == kBinaryPacketFirstByte_) {
      info.packet_bytes_count_ = 1;
      index++;
    } else {
      info.Reset();
      throw std::logic_error("Binary packet first byte (0x24) was not sent");
    }
  };

  const auto ProcessSizeBytes = [&]() {
    if (data_string.empty()) {
      // There is a string with (0x00) value. The string here is ended in cause
      // that (0x00) symvol is a terminator

      info.size_.bytes_array[info.packet_bytes_count_ + 1] = 0x00;
      info.packet_bytes_count_++;
      return false;
    }

    for (; index < data_string.size(); index++) {
      // -1 in cause of binary packet first byte (0x24)
      info.size_.bytes_array[info.packet_bytes_count_ - 1] =
          data_string.at(index);
      info.packet_bytes_count_++;
    }

    // Getting size of usefull message

    // +1 in cause of binary packet first byte (0x24)
    if (info.packet_bytes_count_ == sizeof(info.size_.bytes_array) + 1) {
      std::reverse(std::begin(info.size_.bytes_array),
                   std::end(info.size_.bytes_array));
    }

    return true;
  };

  const auto ProcessUsefullMessageBytes = [&]() {
    size_t needed = info.size_.number - info.usefull_message_.size();

    size_t accessible = (data_string.size() - index);
    if (accessible <= 0)
      return;

    if (accessible < needed) {
      // Grab whole accessible message
      info.usefull_message_ += data_string.substr(index, accessible);
      index += accessible;
      return;
    }

    // Grab whole needed message
    info.usefull_message_ += data_string.substr(index, needed);
    index += needed;
    info.is_message_completely_received_ = true;
  };

  //  ================================

  if (info.packet_bytes_count_ == 0)
    ProcessFirstByte();

  if (info.packet_bytes_count_ < sizeof(info.size_.bytes_array) + 1)
    if (!ProcessSizeBytes())
      return;

  if (info.packet_bytes_count_ > sizeof(info.size_.bytes_array))
    ProcessUsefullMessageBytes();

  if (info.is_message_completely_received_) {
    ProcessCompletedPacket(info.usefull_message_);
    info.Reset();
  }
}

void Receiver::Implementation::ProcessTextPacket(const std::string& data_string,
                                                 size_t& index) {
  static TextPacketReceivingInfo info;

  size_t found_index = index;

  while (true) {
    found_index =
        data_string.find(kTextPacketEndSequence_.front(), found_index + 1);

    if (found_index == std::string::npos) {
      // There is no end sequence in this packet
      info.usefull_message_ += data_string.substr(index);
      index = data_string.size();
    }

    // Checking end symbols sequence exsistance one by one
    while (info.end_sequence_index_ < kTextPacketEndSequence_.size()) {
      char end_symbol = kTextPacketEndSequence_.at(info.end_sequence_index_);
      char data_symbol = data_string.at(found_index + info.end_sequence_index_);

      if (end_symbol == data_symbol) {
        info.end_sequence_index_++;
      } else {
        info.end_sequence_index_ = 0;
        break;
      }
    }

    if (info.end_sequence_index_ >= kTextPacketEndSequence_.size()) {
      size_t length = found_index - index;

      // Sequence is correct
      ProcessCompletedPacket(info.usefull_message_ +
                             data_string.substr(index, length));
      index = found_index + kTextPacketEndSequence_.size();
      info.Reset();
      return;
    }
  }
}

void Receiver::Implementation::ProcessCompletedPacket(
    const std::string& message) {
  if (packet_type_ == PacketType::kBinary)
    kCallback_->BinaryPacket(message.c_str(), message.size());

  if (packet_type_ == PacketType::kText)
    kCallback_->TextPacket(message.c_str(), message.size());

  packet_type_ = PacketType::kUnknown;
  is_receiving_in_process_ = false;
}

// =====================================================

Receiver::Receiver(const std::shared_ptr<ICallback> callback)
    : impl_{std::make_unique<Implementation>(callback)} {}

Receiver::~Receiver() = default;

void Receiver::Receive(const char* data, std::size_t size) {
  impl_->Receive(data, size);
}
