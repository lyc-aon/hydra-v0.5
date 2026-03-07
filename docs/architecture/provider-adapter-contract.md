# Provider Adapter Contract

Each provider adapter must expose a concrete, testable contract. Hydra should not rely on vague "run command" hooks.

## Required capabilities

### Detection

- executable discovery
- version probe
- install-state classification

### Authentication

- auth lane discovery
- auth status probe
- credential storage expectations
- failure classification for expired or missing credentials

### Configuration

- user config location knowledge
- project config location knowledge
- repo-local overlay writer
- merge strategy that avoids clobbering user-owned files

### Launch

- launch command builder
- environment map builder
- working-directory rules
- risk-profile mapping
- worktree compatibility hints

### Resume

- hard-resume compatibility
- provider-native resume command builder
- session identity extraction and persistence strategy

### Models

- alias catalog
- exact-id catalog, when available
- auth-mode-aware discovery strategy
- TTL and refresh policy

### Events and status

- structured hook or telemetry ingestion strategy
- prompt-marker integration hints
- normalized-state mapping rules

## Adapter output types

Every adapter should produce typed outputs rather than raw strings:

- `ProviderProbe`
- `ProviderAuthStatus`
- `ProviderLaunchPlan`
- `ProviderResumePlan`
- `ProviderModelCatalog`
- `ProviderEvent`
- `ProviderOverlayWriteResult`

## Launch plan shape

Each `ProviderLaunchPlan` should include:

- executable path
- argument vector
- environment delta
- working directory
- overlay files to materialize
- session identity extraction strategy
- expected event sources

## Resume plan shape

Each `ProviderResumePlan` should include:

- resume mode
- command vector
- required identifiers
- fallback behavior
- confidence level

## Initial per-provider posture

### Claude Code

Start with:

- settings overlay support
- model alias selection
- hook registration support
- continue/resume command support

### Codex CLI

Start with:

- `.codex/config.toml` overlay support
- model and provider configuration
- approval and sandbox profile mapping
- `resume` and `fork` support
- OTEL configuration support

### Gemini CLI

Start with:

- `.gemini/settings.json` overlay support
- auth-lane detection
- model alias selection
- restore/checkpoint support

### GenericCommandAdapter

Support:

- executable path
- argument template
- environment template
- optional prompt markers
- optional hard-resume only

## Design rule

Provider adapters are allowed to expose provider-specific features. Hydra should normalize what can be normalized, but it should not erase real provider differences that matter operationally.
