#include <cmath>
#include <iostream>
#include <string.h>
#include <set>
#include <vector>
#include "Subject.h"
using namespace std;

class Course {

private:

    // subject : what subject this course belongs to.
    //         | for example, CS410 subject would be CS.
    Subject subject;


    // number : what is the number of this class, as a string.
    //        | must be a 3-length string that is parseable as an integer.
    //        | for example, CS410's number would be the string "410".
    string number;


    // number_as_integer : 'number', as an integer instead of string.
    //                   | only for internal use (comparing course codes, etc)
    unsigned int number_as_integer;


    // name : the name of the course, as seen in course descriptions.
    //      | for example, CS410's name would be "Software Engineering".
    string name;


    // prereqs : set containing all prerequisite classes.
    set<Course> prereqs;


    // true iff prerequisites.length() > 0
    bool has_pre;




    // validate_course_number : throws an exception if a course number is NOT:
    //                        | - a 3-length string
    //                        | - parseable to an integer
    //                        | and returns the course number as an integer.
    int validate_course_number(string num);

public:
    // constructors

    // (Subject, string/uint, string) : makes a no-prerequisite course
    Course(Subject subj, string num, string n);
    Course(Subject subj, unsigned int num_as_int, string n);

    // (Subject, string/uint, string, set) : makes a course with prerequisites from a set
    Course(Subject subj, string num, string n, set<Course> p);
    Course(Subject subj, unsigned int num_as_int, string n, set<Course> p);

    // (Subject, string/uint, string, vector) : makes a course with prerequisites from a vector
    Course(Subject subj, string num, string n, vector<Course> p_vec);
    Course(Subject subj, unsigned int num_as_int, string n, vector<Course> p_vec);
    
    // (Subject, string/uint, string, Course, Course...) : makes a course with prerequisites in-line
    //                                                   | example: CS310(CS, 310, "Algo", CS210, CS220, CS240)
    //                                                   | can take as few as 1 course or as many as you want
    template <typename... Args>
    Course(Subject subj, string num, string n, Course first, Args... rest) : Course(subj, num, n) {
        prereqs.insert(first);
        has_pre = true;
        add_prereqs(rest...);
    }
    template <typename... Args>
    Course(Subject subj, unsigned int num_as_int, string n, Course first, Args... rest) : Course(subj, num_as_int, n) {
        prereqs.insert(first);
        has_pre = true;
        add_prereqs(rest...);
    }

    // mutators
    void set_subject(Subject subj);
    void set_name(string n);
    void set_number(string num);
    void set_number(unsigned int num_as_int);
    void set_prereqs(set<Course> p);

    // accessors
    Subject get_subject();
    string get_name();
    string get_number();
    set<Course> get_prereqs();
    bool has_prereqs();
    
    // indirect accessors
    string get_subject_name();
    string get_subject_code();
    string get_full_code();
    string get_full_name();
    // get_prereq_codes_one_string : returns a string with all the prereq codes
    //                             | for example, for CS310 returns: "CS210, CS220, CS240"
    string get_prereq_codes_one_string(string between = ", ");
    // get_prereq_codes_one_string : returns a vector with all the prereq codes
    vector<string> get_prereq_codes();

    // add_prereqs : add some courses as prereqs.
    //             | called as: my_course.add_prereqs(prereq1, prereq2, prereq3...)
    template <typename... Args>
    void add_prereqs(Course first, Args... rest) {
        prereqs.insert(first);
        has_pre = true;
        add_prereqs(rest...);
    }
    void add_prereqs();



    // operator overloading (must be done in header)
    friend bool operator==(const Course& lhs, const Course& rhs)   {
        return lhs.subject == rhs.subject
            && lhs.number  == rhs.number;
    }
    friend bool operator!=(const Course& lhs, const Course& rhs)   { return !(lhs == rhs); }
    friend bool operator<(const Course& lhs, const Course& rhs)    {
        return lhs.number < rhs.number;
    }
    friend bool operator>(const Course& lhs, const Course& rhs)    { return rhs < lhs;     }
    friend bool operator<=(const Course& lhs, const Course& rhs)   { return !(rhs < lhs);  }
    friend bool operator>=(const Course& lhs, const Course& rhs)   { return !(lhs < rhs);  }

    
    // course_number_length : length of each course number (3).
    static unsigned int course_number_length;

    // none : empty set of prereqs.
    static set<Course> none;
};




