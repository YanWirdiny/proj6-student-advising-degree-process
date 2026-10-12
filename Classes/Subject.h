#ifndef SUBJECT_H
#define SUBJECT_H

#include <map>
#include <string>
using namespace std;

// Subject : enumerates every possible subject for a Course
//         | note: this is NOT the same as Majors
//         | small list for now; add if needed later
enum Subject {
    ART,
    BIOL,
    CHEM,
    CS,
    ECON,
    ENGIN,
    ENGL,
    HIST,
    MATH,
    MUSIC,
    PHIL,
    PHYSIC,
};

// subj_map_to_name : maps each Subject to its name.
//                 | for example: CS maps to Computer Science.
const map<Subject, string> subj_map_to_name = {
    { ART, "Art" },
    { BIOL, "Biology" },
    { CHEM, "Chemestry" },
    { CS, "Computer Science" },
    { ECON, "Economics" },
    { ENGIN, "Engineering" },
    { ENGL, "English" },
    { HIST, "History" },
    { MATH, "Mathematics" },
    { MUSIC, "Music" },
    { PHIL, "Philosophy" },
    { PHYSIC, "Physics" },
};

// subj_map_to_code : maps each Subject to its code, as a string.
//                 | for example: CS maps to "CS".
const map<Subject, string> subj_map_to_code = {
    { ART, "ART" },
    { BIOL, "BIOL" },
    { CHEM, "CHEM" },
    { CS, "CS" },
    { ECON, "ECON" },
    { ENGIN, "ENGIN" },
    { ENGL, "ENGL" },
    { HIST, "GIST" },
    { MATH, "MATH" },
    { MUSIC, "MUSIC" },
    { PHIL, "PHIL" },
    { PHYSIC, "PHYSIC" },
};


#endif