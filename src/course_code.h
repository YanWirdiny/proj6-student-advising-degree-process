#pragma once
#include <string>

// Canonical form used everywhere a course is identified, so a record parsed from the
// audit and an entry loaded from the catalog JSON compare equal: upper-case subject
// followed directly by the number and suffix, e.g. "CS110", "MATH140", "CS285L".
//
// Accepts the spellings the audit and catalog pages print: "CS 110", "cs110",
// "Math 140", "Physics113", "English  101", "EDC U 212".
// Returns "" when there is no subject or no number.
std::string normalizeCode(const std::string& raw);

// Subject part of a canonical code: "CS310" -> "CS", "PHYSIC113" -> "PHYSIC".
// A code without digits (placeholder slot such as "FREE") is returned whole.
std::string subjectOf(const std::string& code);
