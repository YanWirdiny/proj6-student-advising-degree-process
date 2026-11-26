import type { ApiError, Course, Health, SearchQuery, SearchResponse } from "./types";

// Thrown for any non-2xx response; carries the contract's error body.
export class ApiRequestError extends Error {
  constructor(public status: number, public body: ApiError) {
    super(body.error);
  }
}

async function request<T>(path: string, init?: RequestInit): Promise<T> {
  const res = await fetch(path, {
    ...init,
    headers: { "Content-Type": "application/json", ...init?.headers },
  });
  if (!res.ok) {
    const body = (await res.json().catch(() => ({ error: res.statusText }))) as ApiError;
    throw new ApiRequestError(res.status, body);
  }
  return (res.status === 204 ? undefined : await res.json()) as T;
}

function searchPath(query: SearchQuery): string {
  const params = new URLSearchParams({ q: query.q });
  if (query.group) params.set("group", query.group);
  if (query.subject) params.set("subject", query.subject);
  if (query.limit) params.set("limit", String(query.limit));
  return `/api/courses?${params}`;
}

export const api = {
  health: () => request<Health>("/api/health"),
  // Pass an AbortSignal so a newer search can cancel an older one still in flight.
  searchCourses: (query: SearchQuery, signal?: AbortSignal) =>
    request<SearchResponse>(searchPath(query), { signal }),
  course: (code: string) =>
    request<{ course: Course; satisfies: string | null }>(`/api/courses/${encodeURIComponent(code)}`),
};
