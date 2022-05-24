#include <iostream>
#include <boost/filesystem/operations.hpp>

int main()
{
  std::cout << "Hello world\n";
  std::cout << boost::filesystem::current_path().string();
}
