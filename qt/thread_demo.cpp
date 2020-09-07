/*
  1. Tasks that don't need the event loop. Use QtConcurrent and
     QThreadPool + QRunnable
  2. Task that use signal slot and need the event loop. Use
     worker-threads moved to + QThread
*/
#include <cstdlib>
#include <iostream>

#include "thread_demo.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

// The better solution relies on using QEventLoop provided by
// QThread. The idea is simple: we use a signal/slot mechanism to issue
// requests, and the event loop running inside the thread will serve as a
// queue allowing just one slot at a time to be executed.


LogWorker::LogWorker(QObject *parent) : QObject(parent) {

}

void LogWorker::logEvent(const QString &event) {
  qDebug() << "logEvent executed in " << QThread::currentThreadId();
}

LogService::LogService(QObject *parent) : QObject(parent) {
  thread = new QThread(this);
  worker = new LogWorker;
  worker->moveToThread(thread);
  connect(this, &LogService::logEvent, worker, &LogWorker::logEvent);
  connect(thread, &QThread::finished, worker, &QObject::deleteLater);
  thread->start();
}

LogService::~LogService() {
  thread->quit();
  thread->wait();
}


int main(int argc, char* argv[]) {
  qDebug() << "Main thread " << QThread::currentThreadId();

  QCoreApplication app(argc, argv);

  bool exitFlag = false;

  auto f = std::async(std::launch::async, [&exitFlag] {
    std::getchar();
    exitFlag = true;
  });

  QTimer exitTimer;
  exitTimer.setInterval(500);
  exitTimer.setSingleShot(false);

  QObject::connect(&exitTimer,
                   &QTimer::timeout,
  [&app, &exitFlag] {
    if (exitFlag)
      app.quit();
  });

  exitTimer.start();

  LogService* pService = new  LogService(&app);
  emit pService->logEvent("Hello");

  std::cout << "Started! Press Enter to quit...";

  int ret =  app.exec();
  f.wait();
  return ret;

}
