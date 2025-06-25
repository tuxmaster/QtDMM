#pragma once

#include "dmmdriver.h"

class DrvDO3122 : public DmmDriver
{
  Q_OBJECT
public:
  std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);
  bool checkFormat(const char* data, size_t len, ReadEvent::DataFormat df) {return false; };

private:
  const char* DO3122Digit(int byte, bool *convOk);
};
