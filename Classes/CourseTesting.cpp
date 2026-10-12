#include <iostream>
#include <string.h>
#include <set>
#include <vector>
#include "Course.h"
using namespace std;

template <typename T>
void check_eq(string test_name, const T& arg1, const T& arg2) {
    if (!(arg1 == arg2)) {
        cout << "Test failed: " << test_name << endl;
    }
}

template <typename T>
void check_ne(string test_name, const T& arg1, const T& arg2) {
    if ((arg1 == arg2)) {
        cout << "Test failed: " << test_name << endl;
    }
}

void check_is(string test_name, bool condition) {
    if (!condition) {
        cout << "Test failed: " << test_name << endl;
    }
}

void check_nt(string test_name, bool condition) {
    if (condition) {
        cout << "Test failed: " << test_name << endl;
    }
}



int main() {
    cout << "Hello World!" << endl;

    // lots of courses to use as examples
    Course MATH130(MATH, 130, "Precalculus");
    Course MATH140(MATH, "140", "Calculus I", MATH130);
    Course CS110(CS, 110, "Intro to Computing", MATH130);
    Course CS210(CS, 210, "Intermediate Computing W/ Data Structures", CS110);
    Course CS220(CS, "220", "Applied Discrete Mathematics", CS110, MATH140);
    Course CS240(CS, 240, "Programming in C", CS110);
    Course CS310(CS, 310, "Advanced Data Structures & Algorithms", CS210, CS220, CS240);
    Course CS341(CS, "341", "Computer Architecture & Organization", CS240);
    set<Course> p_410;
    p_410.insert(CS310);
    Course CS410(CS, 410, "Intro to Software Engineering", p_410);
    set<Course> p_444;
    p_444.insert(CS310);
    p_444.insert(CS341);
    Course CS444(CS, "444", "Intro to Operating Systems", p_444);
    vector<Course> p_450 = {CS220, CS310};
    Course CS450(CS, 450, "Structure of Higher Level Languages", p_450);
    Course CS420(CS, "420", "Intro to Theory of Computation", CS220);
    Course CS451(CS, 451, "Compilers", CS310, CS420);

    set<Course> just_110; just_110.insert(CS110);

    // accessor / mutator tests
    // subject
    check_eq("acc1", MATH130.get_subject(), MATH);
    MATH130.set_subject(ART);
    check_ne("mut1", MATH130.get_subject(), MATH);
    MATH130.set_subject(MATH);
    // name
    check_is("acc2", MATH130.get_name() == "Precalculus");
    MATH130.set_name("Calculus");
    check_nt("mut2", MATH130.get_name() == "Precalculus");
    MATH130.set_name("Prealculus");
    // number
    check_is("acc3", MATH130.get_number() == "130");
    MATH130.set_number("230");
    check_nt("mut3", MATH130.get_number() == "130");
    MATH130.set_number("130");
    // prereqs
    check_is("acc4", CS210.get_prereqs() == CS240.get_prereqs());
    CS210.set_prereqs(p_444);
    check_nt("mut4", CS210.get_prereqs() == CS240.get_prereqs());
    CS210.set_prereqs(just_110);
    check_is("acc4.5", CS210.get_prereqs() == CS240.get_prereqs());

    cout << "Hello World 2!" << endl;
    return 0;
}


