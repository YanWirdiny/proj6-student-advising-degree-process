# UML Class Diagram

Class overview for the **student-advising-degree-process** crawler. It fetches
UMass Boston seed pages, strips HTML to text, extracts CS course codes, and
stores the de-duplicated result as JSON.

Visibility: `+` public &nbsp; `-` private &nbsp; `#` protected

---

## Data / value types

### `enum SeedCategory`

| Value |
|-------|
| `CS_COURSE` |
| `CATALOG` |
| `GEN_ED` |
| `DISTRIBUTION` |

### `struct Course`

| Vis | Member | Type |
|-----|--------|------|
| + | `code` | `string` |

### `struct Seed`

| Vis | Member | Type |
|-----|--------|------|
| + | `url` | `string` |
| + | `category` | `SeedCategory` |

---

## Behavior classes

### `class Fetcher`

| Vis | Method | Returns |
|-----|--------|---------|
| + | `fetch(url: string)` | `string` |

### `class HtmlParser`

| Vis | Method | Returns |
|-----|--------|---------|
| + | `extractText(html: string) const` | `string` |

### `class CourseExtractor`

| Vis | Method | Returns |
|-----|--------|---------|
| + | `extract(text: string) const` | `vector<Course>` |

### `class CourseStorage`

| Vis | Member / Method | Type / Returns |
|-----|-----------------|----------------|
| - | `filepath_` | `string` |
| - | `seen_` | `set<string>` |
| - | `courses_` | `vector<Course>` |
| + | `CourseStorage(filepath: string)` | *ctor* |
| + | `add(courses: vector<Course>)` | `void` |
| + | `save() const` | `void` |

### `class Politeness`

| Vis | Member / Method | Type / Returns |
|-----|-----------------|----------------|
| - | `delay_` | `int` |
| + | `Politeness(delay: int = 1000)` | *ctor* |
| + | `wait() const` | `void` |

---

## Free functions

| Location | Function | Returns |
|----------|----------|---------|
| `seeds.h` | `getSeeds()` | `vector<Seed>` |
| `main.cpp` | `main()` | `int` |

---

## Relationships

| From | To | Type | Note |
|------|----|------|------|
| `Seed` | `SeedCategory` | association | `category` field |
| `CourseStorage` | `Course` | composition (`0..*`) | owns `courses_` |
| `CourseExtractor` | `Course` | dependency | produces courses |
| `getSeeds()` | `Seed` | dependency | creates seeds |
| `main()` | `getSeeds()` | dependency | obtains seeds |
| `main()` | `Fetcher` | dependency | `fetch()` |
| `main()` | `HtmlParser` | dependency | `extractText()` |
| `main()` | `CourseExtractor` | dependency | `extract()` |
| `main()` | `CourseStorage` | dependency | `add()` / `save()` |

---

## Pipeline flow

```
getSeeds() → [filter CS_COURSE] → Fetcher.fetch(url)
          → HtmlParser.extractText(html)
          → CourseExtractor.extract(text)
          → CourseStorage.add(courses)
          → CourseStorage.save()  →  cs_courses.json
```

---

## Notes

- **Course** and **Seed** are plain data structs (all members public).
- **SeedCategory** is a scoped enum; only `CS_COURSE` seeds are currently
  processed by `main`.
- **CourseStorage** owns its `Course` objects (composition) and uses `seen_`
  to de-duplicate by course code.
- **Politeness** is defined but not yet wired into `main.cpp` or
  `CMakeLists.txt`, so it has no relationships in the pipeline above. (Shown
  here as-is; note `politeness.cpp` currently has a mismatched constructor
  name and `wait()` is not declared in the header.)
