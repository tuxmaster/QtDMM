#pragma once

#include "dmmdriver.h"

class DrvQM1537 : public DmmDriver
{
public:
  std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);

};
