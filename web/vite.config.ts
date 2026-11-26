import { defineConfig } from "vite";

// `npm run dev` serves the page on :5173 and forwards /api/* to the C++ advisor on :8080,
// so the frontend and the server can be restarted independently.
export default defineConfig({
  server: {
    port: 5173,
    proxy: { "/api": "http://127.0.0.1:8080" },
  },
});
