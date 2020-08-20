#pragma once

#include <QObject>
#include <QString>
#include <QThread>

class LogWorker: public QObject {
  Q_OBJECT
 public:
  explicit LogWorker(QObject *parent = nullptr);
 public slots:
  // this slot will be executed by event loop (one call at a time)
  void logEvent(const QString &event);
};

// interface
class LogService : public QObject {
  Q_OBJECT
 public:
  explicit LogService(QObject *parent = nullptr);
  ~LogService();

 signals:
  // to use the service, just call this signal to send a request:
  // logService->logEvent("event");
  void logEvent(const QString &event);

 private:
  QThread *thread;
  LogWorker *worker;
};
