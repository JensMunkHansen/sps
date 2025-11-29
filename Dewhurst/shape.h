//
// Copyright (c) 2017 by Daniel Saks and Stephen C. Dewhurst.
//
// Permission to use, copy, modify, distribute, and sell this
// software for any purpose is hereby granted without fee, provided
// that the above copyright notice appears in all copies and
// that both that copyright notice and this permission notice
// appear in supporting documentation.
// The authors make no representation about the suitability of this
// software for any purpose. It is provided "as is" without express
// or implied warranty.
//
#ifndef INCLUDED_SHAPE
#define INCLUDED_SHAPE

class ShapeBase
{
};

class Rollable
{
public:
  virtual ~Rollable() = default;
  virtual void roll() = 0;
};

class Shape : public ShapeBase
{
public:
  virtual ~Shape() = default;
  virtual void draw() const = 0;
};

class Circle
  : public Shape
  , public Rollable
{
public:
  Circle() {}
  void draw() const override {}
  void roll() override {}
};

class Square : public Shape
{
};

class Triangle : public Shape
{
public:
  Triangle() {}
  void draw() const override {}

private:
  int a_;
};

#endif
