# Repository Tree Proposal

This is the recommended first-pass source tree for the actual application repo once implementation starts.

```text
HYDRA V2/
  CMakeLists.txt
  CMakePresets.json
  cmake/
    warnings.cmake
    sanitizers.cmake
    versioning.cmake
  packaging/
    cpack/
    macos/
    linux/
  docs/
    adr/
    architecture/
    planning/
    validation/
  assets/
    icons/
    fonts/
    themes/
  qml/
    Hydra/
      App.qml
      components/
      panes/
      sidebar/
      settings/
      styles/
  src/
    app/
    domain/
      events/
      models/
      services/
      value_types/
    infrastructure/
      mux/
      providers/
      persistence/
      git/
      terminal/
      telemetry/
    platform/
      macos/
      linux/
    ui/
      viewmodels/
      controllers/
  tests/
    unit/
    integration/
    ui/
  third_party/
  scripts/
```

Repository rules:

- Keep Qt/QML assets under `qml/`, not mixed into `src/`.
- Keep platform-specific code out of the domain and infrastructure cores.
- Keep provider adapters isolated under `src/infrastructure/providers/`.
- Keep terminal backends isolated under `src/infrastructure/terminal/`.
- Keep persistence schema and migrations versioned under `src/infrastructure/persistence/`.
