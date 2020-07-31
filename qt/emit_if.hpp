#pragma once

class EmitSomethingIF {
public:
  virtual ~EmitSomethingIF() = default;
 signals:
  virtual void someThingHappened() = 0;
};

Q_DECLARE_INTERFACE(EmitSomethingIF, "EmitSomethingIF")
