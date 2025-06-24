#pragma once

#include "dmmdriver.h"

class DrvRS22812 : public DmmDriver
{
  Q_OBJECT
public:
  std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);

private:
  const char *RS22812Digit( int byte );
};
