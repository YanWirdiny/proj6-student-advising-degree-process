#pragma once
#include <string>
#include "politeness.h"

class Fetcher {
public:
    explicit Fetcher(int delayMs = 1000);
    std::string fetch(const std::string& url);
private:
    Politeness politeness_;
};
