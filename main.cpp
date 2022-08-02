#include <iostream>
#include "ExampleClass.h"

#include <petscdm.h>
#include <petscdmda.h>
#include <petscksp.h>
#include <petscsys.h>
#include <petscvec.h>

int main()
{
  std::cout << "Hello world!!\n ";
  std::cout << "Current path is " + boost::filesystem::current_path().string();
}
