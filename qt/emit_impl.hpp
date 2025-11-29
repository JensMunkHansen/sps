#pragma once

#include <EmitSomethingIF.hpp>

class EmitSomethingImpl
  : public QObject
  , public EmitSomethingIF
{
  Q_OBJECT
  Q_INTERFACES("EmitSomethingIF")
public:
signals:
  void someThingHappened();
};
