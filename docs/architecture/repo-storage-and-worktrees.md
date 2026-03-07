# Repo Storage and Worktrees

## Repo-local state

On first add or first launch, Hydra should create a `.hydra/` directory at the repository root.

Default local layout:

- `.hydra/providers/`
- `.hydra/docs/`
- `.hydra/heartbeats/`
- `.hydra/session-templates/`
- `.hydra/local.json`

## Git hygiene

Hydra should add `.hydra/` to `.git/info/exclude` by default. That keeps machine-local orchestration data out of version control without forcing repo-wide ignore changes.

Hydra should:

- detect if the exclude entry already exists
- append idempotently
- report failures clearly

## Worktrees

Hydra should treat worktrees as first-class launch targets.

Worktree responsibilities:

- create and list worktrees
- bind sessions to a specific worktree path
- preserve worktree identity in persisted session metadata
- optionally store worktree-specific hints in Git's per-worktree config when useful

## Config ownership

Hydra must not make its own config format the only source of truth.

Policy:

- Hydra owns normalized launch profiles and layout metadata.
- Provider-native project config remains authoritative for provider-specific behavior.
- Hydra writes generated overlays rather than silently mutating unrelated user files.

## Docs and context packs

`.hydra/docs/` should hold repo-specific attachments that help agent sessions start consistently:

- architecture summaries
- conventions
- task packs
- frequently used context files

These are repo-local and intended for machine use as well as human inspection.
