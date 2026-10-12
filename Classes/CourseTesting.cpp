#include <iostream>
#include <string.h>
#include <set>
#include <vector>
#include "Course.h"
using namespace std;



int main() {
    cout << "Hello World!" << endl;

    Course MATH130(MATH, 130, "Precalculus");
    Course CS110(CS, 110, "Intro to Computing", MATH130);
    Course CS210(CS, 210, "Intermediate Computing W/ Data Structures", CS110);
    Course CS220(CS, 220, "Applied Discrete Mathematics", CS110);
    Course CS240(CS, 240, "Programming in C", CS110);
    Course CS310(CS, 310, "Advanced Data Structures & Algorithms", CS210, CS220, CS240);

    cout << CS310.get_prereq_codes_one_string() << endl;

    cout << "Hello World 2!" << endl;
    return 0;
}


