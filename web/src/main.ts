import "./styles.css";
import { api, ApiRequestError } from "./api";

const app = document.querySelector<HTMLDivElement>("#app")!;

app.innerHTML = `
  <header class="navbar"><strong>UMASS BOSTON</strong><span>Degree Advising</span></header>
  <main>
    <h1>Backend status</h1>
    <section class="card" id="health"><span class="label">Loading…</span></section>
  </main>
`;

const card = document.querySelector<HTMLElement>("#health")!;

// textContent (never innerHTML) for data from the server.
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
