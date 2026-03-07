# ADR 0002: tmux as the Primary Session Backend

## Status

Accepted.

## Decision

Hydra will use `tmux` as the primary `MuxAdapter` implementation for macOS and Ubuntu-class Linux.

## Why

- Stable detach/reattach semantics.
- Mature pane and session model.
- Background persistence without inventing custom process resurrection.
- Control mode exposes structured events and metadata.

## Consequences

- Hydra must model session identity around `tmux` sessions and panes.
- `tmux` becomes a runtime dependency for the first shipping version.
- Alternate backends remain optional and should only exist if they solve a real problem.
