## Classes / Types:

### `enum Subject`
- Represents a subject for a Course

### `class Course`
| Vis | Member | Type | Explanation |
|-----|--------|------|-------------|
| - | `subject` | `Subject` | subject of the course (for CS410, would be CS) |
| - | `number` | `string` | the course number (for CS410, would be "410") |
| - | `number_as_int` | `uint` | the course number (for CS410, would be 410) |
| - | `name` | `string` | name of the course (for CS410, would be "Software Engineering") |
| - | `prereqs` | `set<Course>` | set of prerequisite courses that must be complete to enroll |
| - | `has_pre` | `bool` | whether or not this course has prerequisites (same as if `prereqs` is non-empty) |
| - | `validate_course_number` | `string -> int` | takes in a potential course number string (like "410") and throws an error if it is invalid. returns the integer value. |
| + | `Course` | `Subject`, `string`/`uint`, `string`, `Course...`/`set<Course>`/`vector<Course>`/none -> `Course` | constructor |
| + | `set_subject` | `Subject` -> `void` | mutator |
| + | `set_name` | `string` -> `void` | mutator |
| + | `set_number` | `string` -> `void` | mutator |
| + | `set_number` | `uint` -> `void` | mutator |
| + | `set_prereqs` | `set<Course>` -> `void` | mutator |
| + | `get_name` | -> `string` | accessor |
| + | `get_number` | -> `string` | accessor |
| + | `get_prereqs` | -> `set<Course>` | accessor |
| + | `has_prereqs` | -> `bool` | accessor |
| + | `get_subject_name` | -> `string` | returns the subject's name (for CS410, would be "Computer Science") |
| + | `get_subject_code` | -> `string` | returns the subject as a string (for CS410, would be "CS") |
| + | `get_full_code` | -> `string` | returns the subject's full code (for CS410, would be "CS410") |
| + | `get_full_name` | -> `string` | returns the subject's name (for CS410, would be "CS410 Software Engineering") |
| + | `get_prereq_codes_one_string` | `string` (optional) -> `string` | returns all prereq course codes in one string (for CS310, would be "CS210 CS220 CS240") |
| + | `get_prereq_codes` | -> `string` | returns all prereq course codes in a vector (for CS310, would be {"CS210", "CS220", "CS240"}) |
| + | `add_prereqs` | `Course...` -> `void` | adds prerequisite course(s) to a class |
| + | `course_number_length = 3` | `static uint` | length of each course number, in digits (would be 3 for UMass Boston) |
| + | `none = {}` | `static set<Course> | reference to set of no prerequisite courses



Cirriculum has set<Course>
Student has set<Course>
Course has Subject

Potentially: should change Course to be interdependant with Requirement; see later
Requirement := something that can be required by a Course or Cirriculum



