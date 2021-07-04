#include "cxxtest.hpp"

#include <sstream>
#include <cxxabi.h>

extern "C" {

#include "display.h"
}

void Display::setCursor(unsigned int x, unsigned int y) {
  x_ = x;
  y_ = y;
}

void Display::setSize(unsigned int size) { size_ = size; }
void Display::setColor(unsigned int font_color) { col_ = font_color; }
void Display::clear() { LCD_clear_buffer(); }
void Display::print(string_view sv) {
  for (char c : sv) {
    LCD_show_char(x_, y_, c, size_, col_, 0);
    x_ += size_/2;
  }
  LCD_flush_buffer();
}

void Display::println(string_view sv) {
  //int x0 = x_;
  print(sv);
  //x_ = x0;
  x_ = 0;
  y_ += size_;
}

string demangle(const char* mangled){
  int status;
  string ret = abi::__cxa_demangle(mangled, 0, 0, &status);
  return ret;
}


TestClass::TestClass() { val = 0xdeadbeef; };
TestClass::~TestClass(){};
int TestClass::getVal() { return val; }

TestClass testClass;


extern "C" void cxxtest();
void cxxtest() {
      // display c++ binding!
  Display &display = Display::getInstance();
  display.setCursor(0, 20);
  display.setSize(12);
  display.setColor(255);
  display.println("Hello world from cpp");
  display.println("Line 2");

  // static initialization (fail)
  display.print("Testing static initialization...");
  if (testClass.getVal() == 0xdeadbeef) {
    display.println("OK");
  } else {
    display.println("failed");
  }

//   //exception, (not working(system hangs))
//   display.print("Testing exception...");
//   try{
//     throw 1;
//   }catch (int& e) {
//       if(e==1){
//         display.println("OK");
//         goto next;
//       }
//   }
//   display.println("failed");
// next:

  //rtti (ok)
  display.print("Testing rtti...");
  if (demangle(typeid(testClass).name()) == "TestClass"){
    display.println("OK");
  }else{
    display.println("failed");
  }


}