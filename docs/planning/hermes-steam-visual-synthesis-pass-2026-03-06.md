# Hermes / Steam Visual Synthesis Pass

Last updated: 2026-03-06

## Status

- Phase: `Complete`
- Scope: Phase 1 / 2 shell visual-direction refinement

## Purpose

Use the local inspiration set in `Image Inspiration/` to tighten Hydra's default visual language before Phase 3 lands more product complexity.

This is a style-and-composition refinement pass, not an architecture rewrite.

Keep:

- Phase 1 / 2 product boundaries
- repo / worktree / session model separation
- validated divider, resize, and rail motion behavior
- current test loop: build, capture, GUI validation, image review

Refine:

- default color palette
- surface material treatment
- edge language
- typography emphasis
- state color semantics
- atmospheric details that support the workbench tone

## Active aesthetic hypothesis

The user direction is converging on:

- old-school Steam desktop mood as the default palette baseline
- Hermes contest adjacency as a secondary influence through the provided Hermes images
- mythic / operational tone rather than fantasy decoration
- NGE control-screen discipline
- Matrix-style phosphor / scanline undertones used sparingly

The expected outcome is a sharper, darker, more industrial control-room shell.

## Inputs

Inspiration images currently identified:

- `Image Inspiration/Oldschool Steam.jpg`
- `Image Inspiration/Hermes.jpeg`
- `Image Inspiration/Hermes2.jpeg`
- `Image Inspiration/Hermes3.jpeg`
- `Image Inspiration/Hermes4.jpeg`
- `Image Inspiration/NGE.webp`
- `Image Inspiration/NGE2.webp`
- `Image Inspiration/matrixrain.webp`
- `Image Inspiration/BeNofAfraid.webp`

Current implementation comparison baseline:

- `.runtime/ui-captures/sidebar-sync-refine/contact-sheet.png`

## Analysis method

1. catalog each inspiration asset
2. run image analysis over the actual files through the local Gemini CLI OAuth path already used by FPH
3. save raw analysis output into repo docs
4. synthesize common design signals into a Hydra-specific visual system
5. compare that synthesis to the current shell
6. implement only the deltas that fit the current Phase 1 / 2 architecture
7. revalidate with:
   - desktop build
   - deterministic screenshot capture
   - Wayland semantic UI validation
   - X11 divider pointer validation when shell mechanics are touched
   - Gemini post-change image critique

## Deliverables

- raw per-image analysis document
- synthesized visual-direction document
- concrete implementation deltas
- updated screenshots and validation artifacts
- updated handoff and plan notes

## Completion

Outputs produced:

- `docs/planning/hermes-steam-image-analysis-prompt-2026-03-06.md`
- `docs/validation/gemini-inspiration-image-analysis-2026-03-06.md`
- `docs/validation/inspiration-palette-extraction-2026-03-06.md`
- `docs/recommendations/hermes-steam-visual-synthesis-2026-03-06.md`
- `docs/validation/gemini-hermes-steam-pass-review-2026-03-06.md`
- `.runtime/ui-captures/hermes-steam-pass/contact-sheet.png`

Implementation outcome:

- the default shell palette now reads closer to old-school Steam plus Hermes control-screen cues
- the shell geometry is slightly sharper and less soft
- primary actions remain amber-led while secondary controls carry more steel/industrial tone
- live-state motion now leans green instead of orange

Validation outcome:

- desktop build still passes
- deterministic screenshot capture still passes
- `scripts/validate_wayland_ui.py` passes after being hardened to scope AT-SPI lookups to the launched Hydra PID
- `scripts/validate_divider_drag_x11.py` passes after being updated to use the same PID-scoped lookup path

Known carry-forward issues:

- the refresh confirmation banner still needs a clear transient lifecycle
- the palette is still synthesis-driven rather than sampled exactly from the reference images
- the next continuation plan is `docs/planning/phase-1-2-followup-refinement-2026-03-06.md`
