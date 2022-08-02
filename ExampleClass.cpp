

#include "ExampleClass.h"

namespace toy {
  std::string ExampleClass::helloThere() {
    return "Current path is " + boost::filesystem::current_path().string();
  }
}