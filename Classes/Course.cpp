#include <cmath>
#include <iostream>
#include <string.h>
#include <set>
#include <vector>
#include "Course.h" // header
#include "Subject.h"
using namespace std;


// validate_course_number : throws an exception if a course number is NOT:
//                        | - a 3-length string
//                        | - parseable to an integer
//                        | and returns the course number as an integer.
int Course::validate_course_number(string num) {
    if (num.length() > course_number_length) {
        throw invalid_argument("Course number cannot be greater than " + to_string(course_number_length) + " digits.");
    }
    while (num.length() < course_number_length)   num = "0" + num;
    int n;
    try {
        n = stoi(num);
    }
    catch (const invalid_argument&) { // string is not parseable as int
        throw invalid_argument("Course number must be a number.");
        return 0;
    }
    if (n < 0 || n >= pow(10, course_number_length)) {
        string min_num = "";
        string max_num = "";
        for (int i = 0; i < course_number_length; i++)   min_num += "0", max_num += "9";
        throw invalid_argument("Course number out of bounds (must be between: " + min_num + ", " + max_num + ").");
        return 0;
    }

    return n;
}


// constructors

// (Subject, string/uint, string) : makes a no-prerequisite course
Course::Course(Subject subj, string num, string n) {
    while (num.length() < course_number_length)   num = "0" + num;
    number_as_integer = validate_course_number(num);
    subject = subj;
    number = num;
    name = n;
    has_pre = false;
}
Course::Course(Subject subj, unsigned int num_as_int, string n) {
    string num = to_string(num_as_int);
    while (num.length() < course_number_length)   num = "0" + num;
    number_as_integer = validate_course_number(num);
    subject = subj;
    number = num;
    name = n;
    has_pre = false;
}

// (Subject, string/uint, string, set) : makes a course with prerequisites from a set
Course::Course(Subject subj, string num, string n, set<Course> p) : Course::Course(subj, num, n) {
    prereqs = p;
    has_pre = prereqs.size() > 0;
}
Course::Course(Subject subj, unsigned int num_as_int, string n, set<Course> p) : Course::Course(subj, num_as_int, n) {
    prereqs = p;
    has_pre = prereqs.size() > 0;
}

// (Subject, string/uint, string, vector) : makes a course with prerequisites from a vector
Course::Course(Subject subj, string num, string n, vector<Course> p_vec) : Course::Course(subj, num, n) {
    for (Course c : p_vec) { prereqs.insert(c); }
    has_pre = prereqs.size() > 0;
}
Course::Course(Subject subj, unsigned int num_as_int, string n, vector<Course> p_vec) : Course::Course(subj, num_as_int, n) {
    for (Course c : p_vec) { prereqs.insert(c); }
    has_pre = prereqs.size() > 0;
}

// (Subject, string/uint, string, Course, Course...) : makes a course with prerequisites in-line
//                                                   | example: CS310(CS, 310, "Algo", CS210, CS220, CS240)
//                                                   | can take as few as 1 course or as many as you want
// implemented in header file due to template


// mutators
void Course::set_subject(Subject subj) { subject = subj; }
void Course::set_name(string n) { name = n; }
void Course::set_number(string num) {
    number_as_integer = validate_course_number(num);
    number = num;
}
void Course::set_number(unsigned int num_as_int) {
    string num = to_string(num_as_int);
    number_as_integer = validate_course_number(num);
    number = num;
}
void Course::set_prereqs(set<Course> p) {
    prereqs = p;
    has_pre = prereqs.size() > 0;
}

// accessors
Subject Course::get_subject()     { return subject; }
string Course::get_name()         { return name;    }
string Course::get_number()       { return number;  }
set<Course> Course::get_prereqs() { return prereqs; }
bool Course::has_prereqs()        { return has_pre; }

// indirect accessors
string Course::get_subject_name() { return subj_map_to_name.at(subject); }
string Course::get_subject_code() { return subj_map_to_code.at(subject); }
string Course::get_full_code()    { return get_subject_code() + number; }
string Course::get_full_name()    { return get_subject_code() + number + " " + name; }

// get_prereq_codes_one_string : returns a string with all the prereq codes
//                             | for example, for CS310 returns: "CS210, CS220, CS240"
string Course::get_prereq_codes_one_string(string between) {
    string out = "";
    bool first = true;
    for (Course prereq : prereqs) {
        if (!first)   out += between;
        else          first = false;
        out += prereq.get_full_code();
    }
    return out;
}
// get_prereq_codes_one_string : returns a vector with all the prereq codes
vector<string> Course::get_prereq_codes() {
    vector<string> out;
    bool first = true;
    for (Course prereq : prereqs) {
        out.push_back(prereq.get_full_code());
    }
    return out;
}

// add_prereqs : add some courses as prereqs.
//             | called as: my_course.add_prereqs(prereq1, prereq2, prereq3...)
// implemented in header file due to template usage
void Course::add_prereqs() {}



// operator overloading done in header file: ==, !=, <, >, <=, >=.


// static definitions
unsigned int Course::course_number_length = 3;
set<Course> Course::none = {};

