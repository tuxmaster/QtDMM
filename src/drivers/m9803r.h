#pragma once

#include "dmmdriver.h"

class DrvM9803R : public DmmDriver
{
    Q_OBJECT
public:
  std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);
  bool checkFormat(const char* data, size_t len, ReadEvent::DataFormat df) {return false; };

};
