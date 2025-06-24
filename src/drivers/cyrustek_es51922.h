#pragma once

#include "dmmdriver.h"

class DrvCyrusTekES51922 : public DmmDriver
{
public:
  std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);

};
