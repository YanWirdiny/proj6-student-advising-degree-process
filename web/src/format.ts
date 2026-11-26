// Canonical "CS310" -> display "CS 310" (doc/data-contract.md, Conventions).
// Placeholder slots without digits ("FREE", "AR") are returned unchanged.
export function displayCode(code: string): string {
  return code.replace(/^([A-Z]+)(\d)/, "$1 $2");
}
