// Mirrors doc/data-contract.md section 2.2. Change both together.

export type Status = "done" | "in_progress" | "planned" | "remaining";
export type Group = "cs" | "math_science" | "gen_ed" | "elective";

export interface Course {
  code: string;
  title: string;
  credits: number | null;
  kind: "CS" | "GEN_ED" | "DISTRIBUTION";
  area: string;
  prereqs: string[];
  coreqs: string[];
  note: string;
  termsOffered?: string[];
}

export interface Term { id: string; label: string; year: number; isCurrent: boolean }

export interface PlanEntry {
  id: string;
  courseCode: string;
  termId: string;
  status: Exclude<Status, "remaining">;
  isPlaceholder: boolean;
  group: Group;
}

export interface RequirementItem {
  code: string;
  title: string;
  credits: number | null;
  status: Status;
  isPlaceholder: boolean;
}

export interface RequirementGroup { id: Group; name: string; done: number; total: number; items: RequirementItem[] }

export interface DashboardSummary {
  student: { name: string; initials: string; program: string; standing: string; expectedGrad: string };
  creditsEarned: number;
  creditsRequired: number;
  gpa: number | null;
  requirementsLeft: number;
  inProgressCount: number;
}

export interface Health { status: "ok"; courses: number; dataErrors: string[] }

export interface ApiError { error: string; unmetPrereqs?: string[] }
