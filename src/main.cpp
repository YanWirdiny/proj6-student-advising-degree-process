#include <iostream>
#include "Course.h"
using namespace std;

int main() {
    cout << "Hello World! Student Advising\n";
    
    Course MATH130(MATH, 130, "Precalculus");
    Course MATH140(MATH, "140", "Calculus I", MATH130);

    cout << "The second class here is called: " << MATH140.get_name();

    return 0;
}