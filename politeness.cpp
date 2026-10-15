#include <chrono>
#include <thread>
#include "politeness.h"
#include <iostream>
using namespace std ; 

Politeness::Politeness(int delay) : delay_(delay){} 

 void Politeness::wait() const{
    this_thread::sleep_for(chrono::milliseconds(delay_));
 }
   