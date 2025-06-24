#pragma once

#include "dmmdriver.h"

class DrvIsoTech : public DmmDriver
{
  Q_OBJECT
public:
  std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);

};
