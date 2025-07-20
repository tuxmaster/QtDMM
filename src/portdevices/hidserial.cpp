#include "hidserial.h"

HIDSerialDevice::HIDSerialDevice(const DmmDecoder::DMMInfo info, QString device, QObject *p)
  : QIODevice(p)
  , m_dmmInfo(info)
{
  if (!device.isNull() && (m_handle = hid_open_path(device.toUtf8().data())))
  {
    m_isOpen = true;
    QThread* thread = new QThread;
    this->moveToThread(thread);
    connect(thread, SIGNAL( started() ), this, SLOT( run() ));
    connect(this, SIGNAL( finished() ), thread, SLOT( quit() ));
    connect(thread, SIGNAL( finished() ), thread, SLOT( deleteLater() ));
    thread->start();
  }
}

HIDSerialDevice::~HIDSerialDevice()
{
  close();
}


bool HIDSerialDevice::availablePorts(QStringList &portlist)
{
  qint64 portlist_len = portlist.size();
  HIDSerialDevice::availablePorts(portlist,0x04fa, 0x2490);
  HIDSerialDevice::availablePorts(portlist,0x1a86, 0xe008);

  return portlist.size() > portlist_len;
}

bool HIDSerialDevice::availablePorts(QStringList &portlist,unsigned short vendor_id, unsigned short product_id)
{
  int dev_cnt;
  struct hid_device_info *devs, *cur_dev;


  devs = hid_enumerate(vendor_id, product_id); // all chips this SW belongs to...
  for (dev_cnt = 0, cur_dev = devs; cur_dev != Q_NULLPTR; cur_dev = cur_dev->next) {
    dev_cnt++;
  }

  for (cur_dev = devs; cur_dev != Q_NULLPTR; cur_dev = cur_dev->next) {
    portlist << QString("HID 0x%1:0x%2 %3")
    .arg(vendor_id, 4, 16, QLatin1Char('0'))
    .arg(product_id, 4, 16, QLatin1Char('0'))
    .arg(QString::fromLatin1(cur_dev->path));
  }
  hid_free_enumeration(devs);

  return (dev_cnt > 0);
}

void HIDSerialDevice::close()
{
  if (m_isOpen)
  {
    Q_EMIT aboutToClose();
    m_isOpen = false;

    if (m_handle != Q_NULLPTR)
    {
      hid_close(m_handle);
      m_handle = Q_NULLPTR;
    }
  }
}

void HIDSerialDevice::run()
{
  if (m_isOpen)
  {
    memset(m_buffer, 0, m_buflen);

    unsigned int bps = 19200; // possibly take baud rate from DmmInfo?
    // Send a Feature Report to the device
    m_buffer[0] = 0x0; // report ID
    m_buffer[1] = bps;
    m_buffer[2] = bps >> 8;
    m_buffer[3] = bps >> 16;
    m_buffer[4] = bps >> 24;
    m_buffer[5] = 0x03; // 3 = enable?
    int res = hid_send_feature_report(m_handle, m_buffer, 6); // 6 bytes

    if (res < 0)
    {
      qCritical() << "Unable to send a feature report.";
      close();
    }
    else
    {
      memset(m_buffer, 0, m_buflen);
      QThread::usleep(100);

      do
      {
        unsigned char buf[32];

        res = 0;
        while (res == 0)
        {
          res = hid_read(m_handle, buf, sizeof(buf));
          if (res < 0)
            close();
        }

        if (res > 0)
        {
          // format data
          int len = buf[0] & 0x07; // the first byte contains the length in the lower 3 bits ( 111 = 7 )
          for (int i = 1; i <= len; i++)
          {
            m_buffer[m_buffer_w] = buf[i] & 0x7f; // bitwise and with 0111 1111, mask the upper bit which is always 1
            m_buffer_w = (m_buffer_w + 1) % m_buflen;
          }

          if (len > 0)
            Q_EMIT readyRead();
        }
      }
      while (m_isOpen && res >= 0);
    }
  }

  Q_EMIT finished();
}


qint64 HIDSerialDevice::bytesAvailable()
{
  return (m_buffer_w + m_buflen - m_buffer_r) % m_buflen;;
}


qint64 HIDSerialDevice::readData(char *data, qint64 maxSize)
{
  if (m_isOpen)
  {
    unsigned int len = 0;

    while (maxSize > 0 && bytesAvailable() > 0)
    {
      data[len] = m_buffer[m_buffer_r];
      len++;
      maxSize--;
      m_buffer_r = (m_buffer_r + 1) % m_buflen;
    }

    return len;
  }
  return -1;
};


qint64 HIDSerialDevice::writeData(const char *data, qint64 len)
{
 // not implemented and possible not neccessary
 return 0;
};

