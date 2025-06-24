#pragma once

#include "dmmdriver.h"

class DrvVC820 : public DmmDriver
{
    Q_OBJECT
public:
  std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);

private:
  const char* vc820Digit(int byte);
};
