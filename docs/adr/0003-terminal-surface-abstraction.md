# ADR 0003: Abstract Terminal Rendering from Session Control

## Status

Accepted.

## Decision

Hydra will isolate terminal rendering behind a `TerminalSurface` abstraction and keep provider/session control out of the terminal backend.

## Why

- Session orchestration and terminal rendering are different risk domains.
- Hydra needs freedom to swap embedded terminal backends later.
- External-terminal handoff should remain possible without changing session-core logic.

## Consequences

- The first vertical slice should not depend on a production-grade embedded terminal.
- The terminal backend can evolve independently from provider and mux adapters.
- `libghostty` may be explored later, but only as an experimental backend until its embedding API is stable.
