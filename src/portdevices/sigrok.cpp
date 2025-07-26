#include "sigrok.h"

#include <QThread>
#include <QDebug>
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
#include <glibmm/main.h>
#include <glibmm/variant.h>
G_GNUC_END_IGNORE_DEPRECATIONS
#include <libsigrokcxx/libsigrokcxx.hpp>
#include <sstream>

using namespace sigrok;

SigrokWorker::SigrokWorker(QString type, QString device, std::function<void(const QString &)> dataCb)
  : m_type(type), m_device(device), m_dataCb(dataCb) {}

void SigrokWorker::start()
{
  try
  {
    m_ctx = Context::create();
    auto drv = m_ctx->drivers().at(m_type.toStdString());

    std::map<const ConfigKey *, Glib::VariantBase> opts;
    opts[ConfigKey::get_by_identifier("conn")] =
      Glib::Variant<Glib::ustring>::create(m_device.toStdString());

    auto devs = drv->scan(opts);
    if (devs.empty())
    {
      //emit error("Kein Gerät gefunden.");
      return;
    }

    auto dev = devs[0];
    m_session = m_ctx->create_session();
    m_session->add_device(dev);
    dev->open();

    m_session->add_datafeed_callback([this](std::shared_ptr<Device>, std::shared_ptr<Packet> pkt)
    {
      if (pkt->type()->id() != SR_DF_ANALOG)
        return;
      auto analog = std::dynamic_pointer_cast<Analog>(pkt->payload());
      if (!analog) return;

      auto raw_ptr = static_cast<float *>(analog->data_pointer());
      size_t n = analog->num_samples();
      std::string unit = analog->unit()->name();

      for (size_t i = 0; i < n; ++i)
      {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << raw_ptr[i] << " " << unit << "\n";
        m_dataCb(QString::fromStdString(oss.str()));
      }
    });

    m_session->start();

    m_loop = Glib::MainLoop::create();
    m_loop->run();

    m_session->stop();
    dev->close();
    //emit finished();

  }
  catch (const std::exception &e)
  {
    //emit error(e.what());
  }
}

void SigrokWorker::stop()
{
  if (m_loop) m_loop->quit();
}

// ---------------- SigrokDevice ----------------

SigrokDevice::SigrokDevice(const DmmDecoder::DMMInfo &info, QString device, QObject *parent)
  : QIODevice(parent), m_dmmInfo(info)
{

  auto hp = device.split(':');
  if (hp.size() == 2)
  {
    m_type = hp[0];
    m_device = hp[1];
  }

  m_thread = new QThread(this);
  m_worker = new SigrokWorker(m_type, m_device, [this](const QString & line)
  {
    QMutexLocker locker(&m_mutex);
    m_buffer.append(line.toUtf8());
    QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);
  });

  m_worker->moveToThread(m_thread);
  connect(m_thread, &QThread::started, m_worker, &SigrokWorker::start);
  connect(m_worker, &SigrokWorker::finished, this, &SigrokDevice::onFinished);
  connect(m_worker, &SigrokWorker::error, this, &SigrokDevice::onError);
  connect(this, &SigrokDevice::destroyed, this, [this]()
  {
    m_worker->stop();
    m_thread->quit();
    m_thread->wait();
  });

  open(QIODevice::ReadOnly);
  m_isOpen = true;
  m_thread->start();
}

SigrokDevice::~SigrokDevice()
{
  if (m_worker)
    m_worker->stop();
}

qint64 SigrokDevice::bytesAvailable() const
{
  QMutexLocker locker(&m_mutex);
  return m_buffer.size() + QIODevice::bytesAvailable();
}

qint64 SigrokDevice::readData(char *data, qint64 maxSize)
{
  QMutexLocker locker(&m_mutex);
  qint64 len = qMin(maxSize, static_cast<qint64>(m_buffer.size()));
  memcpy(data, m_buffer.constData(), len);
  m_buffer.remove(0, len);
  return len;
}

qint64 SigrokDevice::writeData(const char *, qint64)
{
  return -1; // Read-only
}

void SigrokDevice::close()
{
  if (m_isOpen)
  {
    m_worker->stop();
    m_thread->quit();
    m_thread->wait();
    m_isOpen = false;
  }
}

void SigrokDevice::onFinished()
{
  //emit finished();
}

void SigrokDevice::onError(QAbstractSocket::SocketError)
{
  //emit finished();
}

bool SigrokDevice::availablePorts(QStringList &list)
{
  // Optional: sigrok scan oder bekannte Geräte
  list << "uni-t-ut61e-ser:/dev/ttyUSB1";
  return true;
}
