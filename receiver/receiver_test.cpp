#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "callback.h"
#include "receiver.h"

class TestClass : public testing::Test {
 public:
  TestClass()
      : kCallback_(std::make_shared<Callback>()), kReceiver_(kCallback_) {}

  const std::string kTextPacketEnd_{"\r\n\r\n"};
  const std::string kBinaryPacketFirstSymbol_{0x24};

  const std::shared_ptr<ICallback> kCallback_;

  union BinaryMessageSize {
    int number;
    char array[4];
  } binary_message_size_;

  Receiver kReceiver_;
};

TEST_F(TestClass, BinaryPacketReceiving) {
  const std::string message{"QWERTY"};

  binary_message_size_.number = message.size();
  std::reverse(std::begin(binary_message_size_.array),
               std::end(binary_message_size_.array));

  kReceiver_.Receive(kBinaryPacketFirstSymbol_.c_str(), 1);

  // There may be (0x00) terminators, so it sends separately
  kReceiver_.Receive(&binary_message_size_.array[0], 1);
  kReceiver_.Receive(&binary_message_size_.array[1], 1);
  kReceiver_.Receive(&binary_message_size_.array[2], 1);
  kReceiver_.Receive(&binary_message_size_.array[3], 1);

  kReceiver_.Receive(message.c_str(), message.size());

  auto data = kCallback_->GetBinaryPacket();
  EXPECT_EQ(message, data.first);
  EXPECT_EQ(message.size(), data.second);
}

TEST_F(TestClass, BinaryPacketReceivingByParts) {
  const std::string message{"QWERTY"};

  binary_message_size_.number = message.size();
  std::reverse(std::begin(binary_message_size_.array),
               std::end(binary_message_size_.array));

  kReceiver_.Receive(kBinaryPacketFirstSymbol_.c_str(), 1);

  // There may be (0x00) terminators, so it sends separately
  kReceiver_.Receive(&binary_message_size_.array[0], 1);
  kReceiver_.Receive(&binary_message_size_.array[1], 1);
  kReceiver_.Receive(&binary_message_size_.array[2], 1);
  kReceiver_.Receive(&binary_message_size_.array[3], 1);

  kReceiver_.Receive(message.substr(0, 2).c_str(), 2);
  kReceiver_.Receive(message.substr(2, 4).c_str(), 4);

  auto data = kCallback_->GetBinaryPacket();
  EXPECT_EQ(message, data.first);
  EXPECT_EQ(message.size(), data.second);
}

TEST_F(TestClass, MixedBinaryPacketsReceiving) {
  const std::string message_0{"QWERTY"};

  binary_message_size_.number = message_0.size();
  std::reverse(std::begin(binary_message_size_.array),
               std::end(binary_message_size_.array));

  kReceiver_.Receive(kBinaryPacketFirstSymbol_.c_str(), 1);

  // There may be (0x00) terminators, so it sends separately
  kReceiver_.Receive(&binary_message_size_.array[0], 1);
  kReceiver_.Receive(&binary_message_size_.array[1], 1);
  kReceiver_.Receive(&binary_message_size_.array[2], 1);
  kReceiver_.Receive(&binary_message_size_.array[3], 1);

  kReceiver_.Receive(
      static_cast<std::string>(message_0 + kBinaryPacketFirstSymbol_).c_str(),
      message_0.size());

  {
    auto data = kCallback_->GetBinaryPacket();
    EXPECT_EQ(message_0, data.first);
    EXPECT_EQ(message_0.size(), data.second);
  }

  const std::string message_1{"HESOYAM"};

  binary_message_size_.number = message_1.size();
  std::reverse(std::begin(binary_message_size_.array),
               std::end(binary_message_size_.array));

  // There may be (0x00) terminators, so it sends separately
  kReceiver_.Receive(&binary_message_size_.array[0], 1);
  kReceiver_.Receive(&binary_message_size_.array[1], 1);
  kReceiver_.Receive(&binary_message_size_.array[2], 1);
  kReceiver_.Receive(&binary_message_size_.array[3], 1);

  kReceiver_.Receive(message_1.c_str(), message_1.size());

  {
    auto data = kCallback_->GetBinaryPacket();
    EXPECT_EQ(message_1, data.first);
    EXPECT_EQ(message_1.size(), data.second);
  }
}

TEST_F(TestClass, TextPacketReceiving) {
  const std::string message{"QWERTY" + kTextPacketEnd_};
  kReceiver_.Receive(message.c_str(), message.size());

  auto data = kCallback_->GetTextPacket();
  EXPECT_EQ("QWERTY", data.first);
  EXPECT_EQ(6, data.second);
}

TEST_F(TestClass, TextPacketWithPartOfFalseEndSequenceReceiving) {
  const std::string message{"QWE\r\nRTY" + kTextPacketEnd_};
  kReceiver_.Receive(message.c_str(), message.size());

  auto data = kCallback_->GetTextPacket();
  EXPECT_EQ("QWE\r\nRTY", data.first);
  EXPECT_EQ(8, data.second);
}

TEST_F(TestClass, TextPacketsReceiving) {
  const std::string message{"QWERTY" + kTextPacketEnd_ + "HESOYAM" +
                            kTextPacketEnd_};
  kReceiver_.Receive(message.c_str(), message.size());

  auto data = kCallback_->GetTextPacket();
  EXPECT_EQ("HESOYAM", data.first);
  EXPECT_EQ(7, data.second);
}

TEST_F(TestClass, TextAndBinaryPacketsReceiving) {
  // Text packet

  const std::string message_0{"QWERTY" + kTextPacketEnd_ +
                              kBinaryPacketFirstSymbol_};
  kReceiver_.Receive(message_0.c_str(), message_0.size());

  {
    auto data = kCallback_->GetTextPacket();
    EXPECT_EQ("QWERTY", data.first);
    EXPECT_EQ(6, data.second);
  }

  // Binary packet

  const std::string message_1{"QWERTY"};
  binary_message_size_.number = message_1.size();
  std::reverse(std::begin(binary_message_size_.array),
               std::end(binary_message_size_.array));

  kReceiver_.Receive(&binary_message_size_.array[0], 1);
  kReceiver_.Receive(&binary_message_size_.array[1], 1);
  kReceiver_.Receive(&binary_message_size_.array[2], 1);
  kReceiver_.Receive(&binary_message_size_.array[3], 1);

  kReceiver_.Receive(message_1.c_str(), message_1.size());

  {
    auto data = kCallback_->GetTextPacket();
    EXPECT_EQ("QWERTY", data.first);
    EXPECT_EQ(6, data.second);
  }
}

TEST_F(TestClass, BinaryAndTextPacketsReceiving) {
  // Binary packet

  const std::string binary_message{"QWERTY"};

  binary_message_size_.number = binary_message.size();
  std::reverse(std::begin(binary_message_size_.array),
               std::end(binary_message_size_.array));

  kReceiver_.Receive(kBinaryPacketFirstSymbol_.c_str(), 1);

  // There may be (0x00) terminators, so it sends separately
  kReceiver_.Receive(&binary_message_size_.array[0], 1);
  kReceiver_.Receive(&binary_message_size_.array[1], 1);
  kReceiver_.Receive(&binary_message_size_.array[2], 1);
  kReceiver_.Receive(&binary_message_size_.array[3], 1);

  kReceiver_.Receive(binary_message.c_str(), binary_message.size());

  {
    auto data = kCallback_->GetBinaryPacket();
    EXPECT_EQ(binary_message, data.first);
    EXPECT_EQ(binary_message.size(), data.second);
  }

  // Text packet

  const std::string text_message{"QWERTY" + kTextPacketEnd_};
  kReceiver_.Receive(text_message.c_str(), text_message.size());

  {
    auto data = kCallback_->GetTextPacket();
    EXPECT_EQ("QWERTY", data.first);
    EXPECT_EQ(6, data.second);
  }
}
