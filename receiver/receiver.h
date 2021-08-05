#ifndef RECEIVER
#define RECEIVER

#include <cstdio>
#include <memory>
#include <string>

#include "../callback/callback.h"

class IReceiver {
 public:
  virtual ~IReceiver() = default;
  virtual void Receive(const char* data, std::size_t size) = 0;
};

class Receiver : public IReceiver {
 public:
  Receiver(const std::shared_ptr<ICallback> callback);
  ~Receiver() override;

  void Receive(const char* data, std::size_t size) override final;

 private:
  class Implementation;
  std::unique_ptr<Implementation> impl_;
};

#endif
