#ifndef CALLBACK
#define CALLBACK

#include <cstdio>
#include <string>

struct ICallback {
  virtual ~ICallback() = default;

  virtual void BinaryPacket(const char* data, std::size_t size) = 0;
  virtual void TextPacket(const char* data, std::size_t size) = 0;

  virtual const std::pair<const std::string, const std::size_t>
  GetBinaryPacket() const = 0;
  virtual const std::pair<const std::string, const std::size_t> GetTextPacket()
      const = 0;
};

struct Callback : public ICallback {
  Callback() = default;
  ~Callback() override = default;

  void BinaryPacket(const char* data, std::size_t size) override final;
  const std::pair<const std::string, const std::size_t> GetBinaryPacket()
      const override;

  void TextPacket(const char* data, std::size_t size) override final;
  const std::pair<const std::string, const std::size_t> GetTextPacket()
      const override;

 private:
  std::string text_packet_;
  std::size_t text_packet_size_;

  std::string binary_packet_;
  std::size_t binary_packet_size_;
};

#endif
