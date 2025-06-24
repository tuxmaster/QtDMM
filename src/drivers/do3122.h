#pragma once

#include "dmmdriver.h"

class DrvDO3122 : public DmmDriver
{
  Q_OBJECT
public:
  std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);

private:
  const char* DO3122Digit(int byte, bool *convOk);
};
