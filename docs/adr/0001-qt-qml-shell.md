# ADR 0001: Qt 6 + Qt Quick/QML App Shell

## Status

Accepted.

## Decision

Hydra will use C++23 with Qt 6 and Qt Quick/QML for the application shell.

## Why

- Native desktop fit on macOS and Linux.
- Mature application framework with strong tooling.
- Good match for animated, stateful desktop UI.
- Built-in controls reduce time-to-value while still allowing a custom design system.

## Consequences

- UI logic must stay disciplined so QML remains declarative and thin.
- Blocking work must move to background execution paths.
- Design-system work should customize stock controls rather than replacing them outright.
