#pragma once

//taste of c++20
#include <string_view>

using namespace std;

class TestClass {
public:
  TestClass();
  ~TestClass();
  int getVal();

private:
  int val;
};

class Display {
public:
  static Display &getInstance(){
      static Display instance;
      return instance;
  }

  void setCursor(unsigned int x, unsigned int y);
  //available:12x6,16x8,24x12
  void setSize(unsigned int size);

  void setColor(unsigned int font_color);
  void print(string_view sv);
  void println(string_view sv);
  void clear();

private:
  unsigned int x_,y_,size_,col_;
  Display(){};
  ~Display(){};
  Display(const Display&) = delete;
  Display& operator=(const Display&) = delete;
};