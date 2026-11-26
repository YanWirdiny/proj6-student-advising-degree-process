import "./styles.css";
import { api, ApiRequestError } from "./api";
import { mountSearch } from "./search";

const app = document.querySelector<HTMLDivElement>("#app")!;

// Page shell from DESIGN_HANDOFF.md section 2. Only Dashboard is built; the other nav items
// are shown but not links yet. The student block (avatar, name) arrives with /api/me/dashboard.
app.innerHTML = `
  <div class="utility-bar">
    <span>University of Massachusetts Boston</span>
    <span class="utility-links"><span>WISER</span><span>Canvas</span><span>UMB Email</span><span>Help</span></span>
  </div>
  <header class="navbar">
    <div class="brand">
      <span class="brand-logo">UMB</span>
      <span class="brand-text"><strong>UMASS BOSTON</strong><span>Degree Advising</span></span>
    </div>
    <nav class="nav-links" aria-label="Main">
      <a class="nav-link is-active" href="/" aria-current="page">Dashboard</a>
      <span class="nav-link is-disabled" title="Coming soon">My 4-Year Plan</span>
      <span class="nav-link is-disabled" title="Coming soon">Degree Audit</span>
      <span class="nav-link is-disabled" title="Coming soon">Course Catalog</span>
      <span class="nav-link is-disabled" title="Coming soon">Advising</span>
    </nav>
    <div class="nav-search" id="nav-search"></div>
  </header>
  <main>
    <h1>Dashboard</h1>
    <section class="card" id="health"><span class="label">Loading…</span></section>
  </main>
`;

mountSearch(document.querySelector<HTMLElement>("#nav-search")!);

const card = document.querySelector<HTMLElement>("#health")!;

function line(tag: string, className: string, text: string): HTMLElement {
  const el = document.createElement(tag);
  el.className = className;
  el.textContent = text;
  return el;
}

api.health()
  .then((health) => {
    card.replaceChildren(line("div", "label", "Courses loaded"), line("div", "value", String(health.courses)));
    if (health.dataErrors.length > 0) {
      card.append(line("div", "label", `Data warnings (${health.dataErrors.length})`));
      const list = document.createElement("ul");
      for (const e of health.dataErrors) list.append(line("li", "", e));
      card.append(list);
    }
  })
  .catch((err: unknown) => {
    const message = err instanceof ApiRequestError
      ? `Server answered ${err.status}: ${err.message}`
      : "Cannot reach the advisor server. Is it running on port 8080?";
    card.replaceChildren(line("div", "error", message));
  });
