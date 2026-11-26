// Navbar course search (DESIGN_HANDOFF.md section 4).
// Typing opens a results dropdown with a scrim over the page; Esc, the close button or a click
// on the scrim closes it; Cmd+K / Ctrl+K focuses the box from anywhere.

import { api, ApiRequestError } from "./api";
import { displayCode } from "./format";
import type { Course, SearchQuery, SearchResponse } from "./types";

type Filter = "all" | "cs" | "math" | "gen_ed";

const FILTERS: { id: Filter; label: string }[] = [
  { id: "all", label: "All" },
  { id: "cs", label: "CS" },
  { id: "math", label: "MATH" },
  { id: "gen_ed", label: "Gen Ed" },
  // "term" and "Open seats" from the design wait for offering / seat data (doc/data-contract.md ⏳).
];

const DEBOUNCE_MS = 150;
const LIMIT = 20;

function filterQuery(q: string, filter: Filter): SearchQuery {
  switch (filter) {
    case "cs":     return { q, group: "cs", limit: LIMIT };
    case "math":   return { q, subject: "MATH", limit: LIMIT };
    case "gen_ed": return { q, group: "gen_ed", limit: LIMIT };
    default:       return { q, limit: LIMIT };
  }
}

// Small DOM helper: element with class and text (textContent, never innerHTML, for server data).
function el<K extends keyof HTMLElementTagNameMap>(tag: K, className = "", text = ""): HTMLElementTagNameMap[K] {
  const node = document.createElement(tag);
  if (className) node.className = className;
  if (text) node.textContent = text;
  return node;
}

function kindLabel(course: Course): string {
  if (course.kind === "CS") return "CS";
  if (course.kind === "GEN_ED") return "Gen Ed";
  return course.area ? `Distribution · ${course.area}` : "Distribution";
}

function metaLine(course: Course): string {
  const credits = course.credits === null ? "— cr" : `${course.credits} cr`;
  return [course.title || "Title not available", credits, kindLabel(course)].join(" · ");
}

export function mountSearch(container: HTMLElement): void {
  // ---- static structure ----
  const box = el("div", "search");
  const input = el("input", "search-input");
  input.type = "search";
  input.placeholder = "Search courses…";
  input.setAttribute("aria-label", "Search courses");
  input.setAttribute("aria-controls", "search-results");
  input.setAttribute("aria-expanded", "false");
  input.autocomplete = "off";
  const hint = el("kbd", "search-hint", navigator.platform.includes("Mac") ? "⌘K" : "Ctrl K");
  box.append(input, hint);

  const scrim = el("div", "search-scrim");
  scrim.hidden = true;

  const panel = el("section", "search-panel");
  panel.id = "search-results";
  panel.hidden = true;
  panel.setAttribute("aria-label", "Course search results");

  const header = el("div", "search-panel-header");
  const titles = el("div");
  const heading = el("h2", "search-panel-title");
  const subtitle = el("p", "search-panel-subtitle", "Adding to your plan arrives with the planner");
  titles.append(heading, subtitle);
  const closeBtn = el("button", "search-close", "✕");
  closeBtn.type = "button";
  closeBtn.setAttribute("aria-label", "Close search");
  header.append(titles, closeBtn);

  const pills = el("div", "search-pills");
  pills.setAttribute("role", "group");
  pills.setAttribute("aria-label", "Filter results");
  let filter: Filter = "all";
  const pillButtons = FILTERS.map(({ id, label }) => {
    const b = el("button", "pill", label);
    b.type = "button";
    b.setAttribute("aria-pressed", String(id === filter));
    b.addEventListener("click", () => {
      filter = id;
      pillButtons.forEach((p, i) => p.setAttribute("aria-pressed", String(FILTERS[i].id === filter)));
      runSearch();
      input.focus();
    });
    pills.append(b);
    return b;
  });

  const countRow = el("div", "search-count");
  const list = el("ul", "search-list");
  panel.append(header, pills, countRow, list);

  container.append(box);
  document.body.append(scrim, panel);

  // ---- behaviour ----
  let timer: number | undefined;
  let inFlight: AbortController | undefined;

  function open(): void {
    panel.hidden = false;
    scrim.hidden = false;
    input.setAttribute("aria-expanded", "true");
    positionPanel();
  }

  function close(): void {
    panel.hidden = true;
    scrim.hidden = true;
    input.setAttribute("aria-expanded", "false");
    inFlight?.abort();
  }

  // Anchor the dropdown under the input (right-aligned, clamped to the viewport).
  function positionPanel(): void {
    const r = box.getBoundingClientRect();
    panel.style.top = `${r.bottom + 8}px`;
    const width = panel.offsetWidth;
    const left = Math.max(16, Math.min(r.right - width, window.innerWidth - width - 16));
    panel.style.left = `${left}px`;
    scrim.style.top = `${r.bottom + 16}px`;
  }

  function render(q: string, data: SearchResponse): void {
    heading.textContent = `Results for "${q}"`;
    const shown = data.results.length;
    const count = data.total === shown ? `${data.total} result${data.total === 1 ? "" : "s"}`
                                       : `Showing ${shown} of ${data.total}`;
    countRow.replaceChildren(el("span", "", count), el("span", "", "Sorted by relevance"));
    list.replaceChildren();
    if (shown === 0) {
      list.append(el("li", "search-empty", `No courses match "${q}".`));
      return;
    }
    for (const { course } of data.results) {
      const row = el("li", "search-row");
      const text = el("div", "search-row-text");
      text.append(el("span", "search-row-code", displayCode(course.code)), el("span", "search-row-meta", metaLine(course)));
      const add = el("button", "search-add", "+ Add");
      add.type = "button";
      add.disabled = true;
      add.title = "Adding to your plan arrives with the planner";
      row.append(text, add);
      list.append(row);
    }
  }

  function renderMessage(q: string, message: string): void {
    heading.textContent = `Results for "${q}"`;
    countRow.replaceChildren();
    list.replaceChildren(el("li", "search-empty", message));
  }

  async function runSearch(): Promise<void> {
    const q = input.value.trim();
    if (!q) { close(); return; }
    open();
    inFlight?.abort();
    const controller = new AbortController();
    inFlight = controller;
    panel.classList.add("is-loading");
    try {
      const data = await api.searchCourses(filterQuery(q, filter), controller.signal);
      if (controller.signal.aborted) return;
      render(q, data);
    } catch (err) {
      if (controller.signal.aborted) return;   // replaced by a newer search
      renderMessage(q, err instanceof ApiRequestError
        ? `Search failed: ${err.message}`
        : "Search is unavailable: cannot reach the advisor server.");
    } finally {
      if (inFlight === controller) panel.classList.remove("is-loading");
    }
  }

  input.addEventListener("input", () => {
    window.clearTimeout(timer);
    timer = window.setTimeout(runSearch, DEBOUNCE_MS);
  });
  input.addEventListener("focus", () => { if (input.value.trim()) runSearch(); });
  closeBtn.addEventListener("click", () => { close(); input.focus(); });
  scrim.addEventListener("click", close);
  window.addEventListener("resize", () => { if (!panel.hidden) positionPanel(); });

  document.addEventListener("keydown", (e) => {
    if ((e.metaKey || e.ctrlKey) && e.key.toLowerCase() === "k") {
      e.preventDefault();
      input.focus();
      input.select();
    } else if (e.key === "Escape" && !panel.hidden) {
      close();
      input.focus();
    }
  });
}
