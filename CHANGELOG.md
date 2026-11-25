# Change Log
All notable changes to this project will be documented in this file.
 
The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).
 
## [Unreleased] - 2026-11-24
 
Branch `yanmoise/story-8-storing-student`. Separates the scraper from the advising backend
and adds the backend's course catalog, loaded from the JSON files the scraper wrote.
After pulling, re-run `cmake` once: the source layout and target names changed (see Changed).
 
### Added
- `catalog_loader.cpp` to read from both `cs_courses.json` and `gen_ed_requirements.json`.
  The only backend code that knows about JSON.
  - `loadCourses` builds one catalog from both files (523 courses with the current data).
  - `loadGenEd` reads the general education requirements, course codes normalized.
  - `loadDistributionRules` reads how many distribution courses each program needs.
  - `checkLinks` reports requirements that name a course missing from the catalog.
  - Problems (missing file, invalid JSON, bad or conflicting entry) are collected in an
    error list instead of stopping the load.

- `courses.cpp` : class `Courses`, the course catalog. Each course is stored once, keyed by
  its normalized code, and can be found by any spelling (`"cs 110"`, `"CS110"`).
  - `add(course)` stores a new course; rejects malformed codes and duplicates.
  - `merge(course)` combines a course listed in more than one place: fills missing fields,
    keeps the stronger category (CS > DISTRIBUTION > GEN_ED), and rejects real conflicts
    (e.g. area MT vs NS) without changing anything.
  - `find`, `contains`, `size`, `all`, `inCategory`, `inArea` for lookups.

- `student.cpp` : `Student` class and `CourseRecord` struct — a student's course history from
  the degree audit. Credits and class standing are always computed from the records.
- nlohmann/json 3.11.3, downloaded by CMake like GoogleTest; used only by the backend.
- Tests: `courses_test.cpp` (14) and `catalog_loader_test.cpp` (16), using small sample files
  and the real JSON files. 56 tests in total, all passing.
 
### Changed
- `struct Course` : added new variables `name`, `credits` and `prereqs`. They come after the
  existing fields and have defaults, so the scraper code did not need to change. A CS course
  can now also carry a distribution area (CS110 is CS and counts toward MT).
- Scraper separated from the backend: scraper files are now moved to `src/scraper/` and their tests to
  `test/scraper/`. Backend stays in `src/` and cannot include
  scraper headers.
- `CMakeLists.txt` builds from folders instead of file lists: `src/*.cpp` is the `backend`
  library, `src/scraper/*.cpp` the scraper. Every `test/*_test.cpp` and
  `test/scraper/*_test.cpp` becomes a test automatically — new files need no CMake edit.
- The scraper program is now called `scraper` (was `advisor`), keeping `advisor` free for the
  advising program.

### Fixed
- `struct Course` : `category` had no default value, so a default-constructed course held
  garbage. It now defaults to `GEN_ED`.

### Removed
- Stray tracked file `origin/yanmoise/story-9/course-json`.

### Known issues
- BC290 is not in the catalog. The writing proficiency requirement allows it for College of
  Management students, but the scraper only collects CS, gen-ed and distribution courses.
  `checkLinks` reports it, and `CheckLinksTest.RealFilesLinkUpExceptBC290` expects exactly
  this one gap. (AI find!)
- Course `credits` and `prereqs` are empty for now: the JSON files do not contain them yet.
