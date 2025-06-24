#pragma once

#include "dmmdriver.h"

class DrvM9803R : public DmmDriver
{
public:
  std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);

};
