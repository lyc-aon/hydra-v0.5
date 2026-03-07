# Hermes / Steam Image Analysis Prompt

Last updated: 2026-03-06

Use this prompt with the local `gemini` CLI against the actual workspace-local images.

Model target:

- `gemini-2.5-pro`

Prompt body:

```text
You are analyzing local inspiration images for the Hydra V2 desktop application.

Context:
- Hydra V2 is a Qt/QML desktop orchestration tool.
- The product is currently only at Phase 1/2: repo targeting, worktrees, detached tmux sessions, session board.
- Do not invent future product surfaces that do not exist.
- The current shell screenshot contact sheet is included for comparison.

Your job:
1. Analyze each inspiration image individually.
2. Record a separate section for each image.
3. Then synthesize the common visual system across the whole set.
4. Then compare that synthesized system against the current Hydra shell screenshot contact sheet.
5. Then produce concrete style directives for the next Hydra refinement pass.

For each image section, include:
- literal filename
- primary mood
- dominant palette
- contrast model
- typography cues
- geometry / shape language
- material / texture cues
- UI composition cues
- motion implications if this were translated to software
- specific details worth borrowing
- specific details that would become kitsch or overdone if copied too directly

After the per-image sections, include:
- Cross-image synthesis
- Recommended Hydra visual system
- Comparison against current Hydra shell
- Top 15 concrete implementation directives for Hydra's current Phase 1/2 shell
- A short "do not do this" list

Important:
- Be very specific.
- Treat the Hermes images, old-school Steam image, NGE images, Matrix image, and the other included references as serious visual inputs, not vague moodboard filler.
- Avoid generic design language.
- Tie every recommendation back to something visibly present in the supplied images.

Images:
- @Image Inspiration/BeNofAfraid.webp
- @Image Inspiration/Hermes.jpeg
- @Image Inspiration/Hermes2.jpeg
- @Image Inspiration/Hermes3.jpeg
- @Image Inspiration/Hermes4.jpeg
- @Image Inspiration/NGE.webp
- @Image Inspiration/NGE2.webp
- @Image Inspiration/Oldschool Steam.jpg
- @Image Inspiration/matrixrain.webp
- @Image Inspiration/neon-genesis-evangelion-screen.gif
- @.runtime/ui-captures/sidebar-sync-refine/contact-sheet.png
```
