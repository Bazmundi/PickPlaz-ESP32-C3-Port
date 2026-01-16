# PickPlaz ESP32-C3 Port

This Sphinx documentation site now hosts the human-authored project docs. The
source files were moved under `docs/source/` to keep authored content separate
from generated output.

## Structure

- `design/` for architecture and porting design notes
- `stages/` for stage plans and lessons learned
- `tooling/` for QEMU/PICSimLab integration notes
- `workflows.md` for developer workflows

Doxygen-generated API docs remain in `docs/doxygen/html/` and can be browsed
via `docs/doxygen/html/index.html`.

```{toctree}
:maxdepth: 2
:caption: Contents

design/porting_design
stages/overview
stages/stage1_lessons
stages/stage1_qemu_lessons
stages/stage2_plan
stages/stage3_plan
stages/stage4_plan
workflows
tooling/picsimlab
tooling/qemu_setup
tooling/qemu_notes
tooling/qemu_summary
api/index
```
