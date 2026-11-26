# Data contract

The agreed shape of every piece of data that crosses a boundary in this project. Each side
builds against this document; if code and this document disagree, fix one of them in the
same change.

```
 Scraper ──(1. data files)──► C++ backend ──(2. HTTP API, JSON)──► Web frontend (web/)
```

Legend used below: ✅ available now · 🔧 data exists, backend code pending · ⏳ data not collected yet

---

## Conventions (apply everywhere)

- **Course codes** are canonical everywhere in data and in the API: upper-case subject followed
  directly by the number and suffix, no space: `"CS310"`, `"MATH140"`, `"CS285L"`.
  The backend produces them with `normalizeCode`. **The frontend adds the space for display only**
  (`"CS310"` → `"CS 310"`): insert a space between the leading letters and the first digit.
- **Placeholder slots** are plan or requirement items that stand for "some course of this kind",
  not a real course: `CS4XX` (CS elective), `FREE` (free elective), and gen-ed slots
  `FYS`, `INTSEM`, `AR`, `HU`, `SB`, `WL`, `DV`, `NS`. They are never looked up in the catalog and
  always come with `"isPlaceholder": true`.
- **Empty means unknown.** In data files an empty string, empty list or `0` credits means "not
  known", never "none". Loading never erases a known value with an empty one.
- **JSON field names** in the API are `camelCase`. In data files they are `snake_case` (scraper
  output) or as listed below.

---

## 1. Data files (scraper / maintainers → backend)

One owner per file. A file is only ever written by its owner.

| File | Owner | Loaded by |
|---|---|---|
| `cs_courses.json` | scraper (rewritten on every run) | `loadCourses` |
| `gen_ed_requirements.json` | scraper (rewritten on every run) | `loadCourses`, `loadGenEd`, `loadDistributionRules` |
| `course_details.json` | team, by hand | `loadCourseDetails` |
| `data/student.json` | team, by hand (later: degree-audit import) | 🔧 not loaded yet |
| `data/plan.json` | the `advisor` server | 🔧 not written yet |

Load order: `loadCourses` → `loadCourseDetails` → `checkCourseReferences` → `loadGenEd` →
`loadDistributionRules` → `checkLinks`. Problems are collected as messages; loading continues.

### 1.1 `cs_courses.json` (scraper)
```json
{
  "cs_courses":           ["CS110", "CS285L"],
  "gen_ed_courses":       ["ENGL101"],
  "distribution_courses": [ { "code": "MATH140", "area": "MT" } ]
}
```
| Field | Type | Becomes |
|---|---|---|
| `cs_courses[]` | string code | `Course{category: CS}` |
| `gen_ed_courses[]` | string code | `Course{category: GEN_ED}` |
| `distribution_courses[].code / .area` | string / area code | `Course{category: DISTRIBUTION, distArea}` |

### 1.2 `gen_ed_requirements.json` (scraper)
```json
{
  "gen_ed": [ { "phase": "first_year", "text": "Two courses in writing…", "courses": ["ENGL101", "ENGL102"], "areas": [] } ],
  "distribution_areas": [ { "name": "Arts and Humanities", "codes": ["AR", "HU"], "outcomes": ["…"],
                            "courses": [ { "code": "PHIL100", "title": "Introduction to Philosophy", "area": "HU" } ] } ],
  "distribution_rules": [ { "program": "BS (CSM)", "areas": ["AR", "HU", "SB"], "min_courses": 3,
                            "min_per_area": { "AR": 1, "HU": 1 }, "note": "" } ]
}
```
| Field | Becomes |
|---|---|
| `gen_ed[]` | `GenEdRequirement` (`phase`, `text`, `courses` normalized, `areas`) |
| `distribution_areas[].courses[]` | `Course{category: DISTRIBUTION, name: title, distArea: area}` |
| `distribution_rules[]` | `DistributionRule` (`min_per_area` object → list of (area, count)) |

`phase` is one of `first_year`, `middle`, `capstone`. Area codes: `AR HU MT NS SB WC WL`.

### 1.3 `course_details.json` (hand-maintained)
```json
{
  "_meta": { "source": "…", "prereqRules": ["…"] },
  "courses": [
    { "code": "CS310", "category": "CS", "area": "", "title": "Advanced Data Structures and Algorithms",
      "credits": 3, "prereqs": ["CS210", "CS220", "CS240"], "coreqs": [],
      "note": "CS210 and CS220 with C- or better; MATH320 can replace CS220" }
  ]
}
```
| Field | Type | Rule |
|---|---|---|
| `code` | string | required |
| `category` | `"CS"` \| `"GEN_ED"` \| `"DISTRIBUTION"` | required; anything else rejects the entry |
| `area` | area code or `""` | `""` = unknown, keeps the scraped area |
| `title` | string | |
| `credits` | number ≥ 0 | |
| `prereqs` | codes | completed **before** the term (all of them) |
| `coreqs` | codes | same term **or earlier** |
| `note` | string | rules not modelled (OR-alternatives, grades, permission); shown to the student as-is |

Every `prereqs` / `coreqs` code must exist in the catalog and prerequisites must not form a cycle
(`checkCourseReferences`). A value that disagrees with the scraped data is reported, and the
scraped value is kept.

### 1.4 `data/student.json` 🔧 (gap C)
```json
{
  "info": { "name": "Yan Moise", "schoolId": "00000000", "major": "Computer Science, B.S.",
            "catalogYear": "2023-2024", "entryTerm": "2023FA", "expectedGrad": "2027SP" },
  "records": [
    { "code": "CS110", "term": "2023FA", "grade": "A",  "credits": 4, "status": "COMPLETED",   "source": "UMB" },
    { "code": "CS446", "term": "2026FA", "grade": "",   "credits": 3, "status": "IN_PROGRESS", "source": "UMB" }
  ]
}
```
Maps 1:1 onto `StudentInfo` and `CourseRecord` (`src/student.h`). `status`: `COMPLETED` |
`IN_PROGRESS`; `source`: `UMB` | `TRANSFER` | `AP`. Completed needs a grade; in progress must not
have one (same rules as `Student::addRecord`).

### 1.5 `data/plan.json` 🔧 (gap D, written only by the server)
```json
{ "nextId": 4,
  "entries": [ { "id": "e1", "courseCode": "CS4XX", "termId": "2027SP", "isPlaceholder": true },
               { "id": "e2", "courseCode": "CS436", "termId": "2027SP", "isPlaceholder": false } ] }
```
Only **planned** courses live here. Done and in-progress courses come from `data/student.json`.

### 1.6 Major rules 🔧 (gap B — not in any file yet)
Which CS courses are core and how many electives are needed. Proposed addition to
`course_details.json`:
```json
"cs_major": { "creditsRequired": 120,
              "core": ["CS110", "CS210", "CS220", "CS240", "CS310", "CS410", "CS420", "CS444", "CS446", "CS450", "CS451"],
              "electives": { "count": 7, "slot": "CS4XX" } }
```

---

## 2. HTTP API (backend → web frontend)

Served by the `advisor` program on `http://127.0.0.1:8080`. All bodies are JSON
(`Content-Type: application/json`). There is no login: **`/api/me` is the single student in
`data/student.json`**.

### 2.1 Errors
Every non-2xx response has this body, plus extra fields where listed:
```json
{ "error": "unknown course CS999" }
```
| Status | When |
|---|---|
| 400 | malformed body or parameter |
| 404 | unknown course, entry or route |
| 409 | the request conflicts with the rules (unmet prereqs, course already planned/completed) |
| 500 | bug; message is for developers |

### 2.2 Shared types (TypeScript notation; the backend produces exactly these)
```ts
type Status = "done" | "in_progress" | "planned" | "remaining";

// Which requirement group a course fills FOR THIS STUDENT. Computed by the Advisor, not stored
// on the course: CS446 is "cs" when it is a core course, "elective" when used as an elective.
type Group = "cs" | "math_science" | "gen_ed" | "elective";

interface Course {
  code: string;             // "CS310" ✅
  title: string;            // ✅ ("" when unknown)
  credits: number | null;   // ✅ for courses in course_details.json, null otherwise
  kind: "CS" | "GEN_ED" | "DISTRIBUTION";  // catalog category ✅
  area: string;             // "MT", "HU", … or "" ✅
  prereqs: string[];        // ✅
  coreqs: string[];         // ✅
  note: string;             // ✅
  termsOffered?: string[];  // ⏳ omitted until offering data exists
}

interface Term {
  id: string;        // "2026FA" (year + FA/SP)
  label: string;     // "Fall 2026"
  year: number;      // 1..4, the student's year in the program (for "Year 3 · 2025–26")
  isCurrent: boolean;
}

interface PlanEntry {
  id: string;                    // "e7" for planned entries; "r:CS110:2023FA" for record-backed ones
  courseCode: string;            // real code, or a placeholder slot code
  termId: string;
  status: "done" | "in_progress" | "planned";
  isPlaceholder: boolean;
  group: Group;                  // band in the tree
}

interface RequirementItem {
  code: string;                  // real code or placeholder slot ("AR", "CS4XX")
  title: string;                 // "Arts", "CS Elective", or the course title
  credits: number | null;
  status: Status;
  isPlaceholder: boolean;
}

interface RequirementGroup {
  id: Group;
  name: string;                  // "Major Core (CS)", "Mathematics & Science", …
  done: number;                  // items with status "done"
  total: number;
  items: RequirementItem[];
}

interface DashboardSummary {
  student: { name: string; initials: string; program: string; standing: string; expectedGrad: string };
  creditsEarned: number;
  creditsRequired: number;
  gpa: number | null;            // null when no graded course yet
  requirementsLeft: number;
  inProgressCount: number;
}
```

### 2.3 Endpoints

| Method | Path | Status |
|---|---|---|
| GET | `/api/health` | 🔧 phase 0 |
| GET | `/api/courses` | 🔧 phase 2 |
| GET | `/api/courses/:code` | 🔧 phase 2 |
| GET | `/api/me/dashboard` | 🔧 needs gaps B, C, E |
| POST | `/api/plan/entries` | 🔧 needs gap D |
| PATCH | `/api/plan/entries/:id` | 🔧 needs gap D |
| DELETE | `/api/plan/entries/:id` | 🔧 needs gap D |

#### `GET /api/health`
```json
{ "status": "ok", "courses": 524, "dataErrors": [] }
```
`dataErrors` lists every problem found while loading the data files.

#### `GET /api/courses?q=cs%204&group=cs&limit=20`
| Param | Meaning |
|---|---|
| `q` | code prefix (any spelling: `cs 4`, `CS4`) or words in the title; empty = all |
| `group` | `cs` \| `math_science` \| `gen_ed` \| `elective` (optional) |
| `subject` | `CS`, `MATH`, … (optional) |
| `term`, `open` | ⏳ accepted and ignored until offering / seat data exists |
| `limit` | default 20, max 100 |

```json
{ "total": 5,
  "results": [ { "course": { "code": "CS437", "title": "Database-Backed Web Sites and Web Services", "credits": 3, "kind": "CS",
                             "area": "", "prereqs": ["CS310", "CS430"], "coreqs": [], "note": "" },
                 "satisfies": "CS Elective" } ] }
```
Sorted by relevance: exact code match, then code prefix, then title match; ties by code.
`satisfies` (🔧) is the requirement the course would fill for this student, or `null`.

#### `GET /api/courses/:code`
`200` → `{ "course": Course, "satisfies": string | null }` · `404` unknown code.

#### `GET /api/me/dashboard`
```json
{ "summary": DashboardSummary,
  "terms": Term[],                        // 8 terms from the entry term
  "plan": PlanEntry[],                    // records (done / in_progress) + planned entries
  "courses": { "CS110": Course, … },      // every real course referenced by plan or requirements
  "requirements": RequirementGroup[] }    // cs, math_science, gen_ed, elective — in that order
```

#### `POST /api/plan/entries`
Request `{ "courseCode": "CS436", "termId": "2027SP" }` → `201` the new `PlanEntry`.

Checks, in order: the course exists (or is a placeholder) → `404`; not already completed,
in progress or planned → `409`; prerequisites done, in progress or planned in an **earlier**
term and co-requisites in the same term or earlier → otherwise
`409 { "error": "unmet prerequisites", "unmetPrereqs": ["CS430"] }`.
Placeholders skip the prerequisite check.

#### `PATCH /api/plan/entries/:id`
Request `{ "termId": "2027FA" }` → `200` the updated `PlanEntry`. Same `409` rules as POST,
checked for the new term. Also `409` if a course planned later depends on this one being earlier.
Only planned entries can move (`409` for done / in-progress).

#### `DELETE /api/plan/entries/:id`
`204`, no body. `404` unknown id; `409` for done / in-progress entries.

---

## 3. Development setup

- `advisor` serves the API on port 8080 and, in production, the built frontend from `web/dist`.
- During frontend work, `npm run dev` in `web/` runs Vite on port 5173 and forwards `/api/*` to
  8080, so both reload independently.
- Until an endpoint exists, the frontend mocks it with fixtures that match this document exactly.

## 4. Changing this contract
1. Change this document and the code on both sides in the same branch.
2. A new field is optional for the other side until both implement it.
3. Never repurpose an existing field; add a new one and deprecate the old.
