#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RUNTIME_DIR="$ROOT_DIR/third_party/qmltermwidget_runtime"
SOURCE_OVERRIDE_DIR="${HYDRA_QMLTERMWIDGET_SOURCE_DIR:-}"
UPSTREAM_REPO="${HYDRA_QMLTERMWIDGET_REPO:-https://github.com/Swordfish90/qmltermwidget.git}"
UPSTREAM_REF="${HYDRA_QMLTERMWIDGET_REF:-master}"
SOURCE_CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/hydra/qmltermwidget-source"

command -v qmake6 >/dev/null 2>&1 || { echo "qmake6 is required" >&2; exit 1; }
command -v make >/dev/null 2>&1 || { echo "make is required" >&2; exit 1; }
command -v git >/dev/null 2>&1 || { echo "git is required" >&2; exit 1; }

if [[ -f "$RUNTIME_DIR/QMLTermWidget/libqmltermwidget.so" ]] \
  && [[ "${FORCE_QMLTERMWIDGET_REBUILD:-0}" != "1" ]]; then
  exit 0
fi

resolve_source_dir() {
  if [[ -n "$SOURCE_OVERRIDE_DIR" ]]; then
    [[ -f "$SOURCE_OVERRIDE_DIR/qmltermwidget.pro" ]] || {
      echo "qmltermwidget source override is missing qmltermwidget.pro at $SOURCE_OVERRIDE_DIR" >&2
      exit 1
    }
    printf '%s\n' "$SOURCE_OVERRIDE_DIR"
    return
  fi

  rm -rf "$SOURCE_CACHE_DIR"
  mkdir -p "$(dirname "$SOURCE_CACHE_DIR")"
  git init -q "$SOURCE_CACHE_DIR"
  git -C "$SOURCE_CACHE_DIR" remote add origin "$UPSTREAM_REPO"
  git -C "$SOURCE_CACHE_DIR" fetch --depth 1 origin "$UPSTREAM_REF"
  git -C "$SOURCE_CACHE_DIR" checkout -q --force FETCH_HEAD
  printf '%s\n' "$SOURCE_CACHE_DIR"
}

SOURCE_DIR="$(resolve_source_dir)"

[[ -f "$SOURCE_DIR/qmltermwidget.pro" ]] || {
  echo "qmltermwidget source is missing at $SOURCE_DIR" >&2
  exit 1
}

BUILD_ROOT="/tmp/hydra_native_terminal_plugin_build"
rm -rf "$BUILD_ROOT"
mkdir -p "$BUILD_ROOT"
cp -a "$SOURCE_DIR" "$BUILD_ROOT/qmltermwidget"

python3 - <<'PY'
from pathlib import Path

path = Path("/tmp/hydra_native_terminal_plugin_build/qmltermwidget/lib/TerminalDisplay.cpp")
text = path.read_text()
old = "  setAcceptedMouseButtons(Qt::LeftButton);\n"
new = "  setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);\n"

if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget accepted mouse buttons patch target not found.")
    text = text.replace(old, new)

path.write_text(text)
PY

python3 - <<'PY'
from pathlib import Path

path = Path("/tmp/hydra_native_terminal_plugin_build/qmltermwidget/lib/TerminalDisplay.h")
text = path.read_text()
old = "    bool _mouseMarks;\n    bool _bracketedPasteMode;\n"
new = "    bool _mouseMarks;\n    bool _forceLocalMouseSelection;\n    bool _bracketedPasteMode;\n"

if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget forceLocalMouseSelection member patch target not found.")
    text = text.replace(old, new)

path.write_text(text)
PY

python3 - <<'PY'
from pathlib import Path

path = Path("/tmp/hydra_native_terminal_plugin_build/qmltermwidget/lib/TerminalDisplay.cpp")
text = path.read_text()
old = """void TerminalDisplay::wheelEvent( QWheelEvent* ev )\n{\n  if (ev->angleDelta().y() == 0)\n    return;\n\n  // if the terminal program is not interested mouse events\n  // then send the event to the scrollbar if the slider has room to move\n  // or otherwise send simulated up / down key presses to the terminal program\n  // for the benefit of programs such as 'less'\n  if ( _mouseMarks )\n  {\n    bool canScroll = _scrollBar->maximum() > 0;\n      if (canScroll)\n        _scrollBar->handleEvent(ev);\n    else\n    {\n        // assume that each Up / Down key event will cause the terminal application\n        // to scroll by one line.\n        //\n        // to get a reasonable scrolling speed, scroll by one line for every 5 degrees\n        // of mouse wheel rotation.  Mouse wheels typically move in steps of 15 degrees,\n        // giving a scroll of 3 lines\n        int key = ev->angleDelta().y() > 0 ? Qt::Key_Up : Qt::Key_Down;\n\n        // QWheelEvent::angleDelta().y() gives rotation in eighths of a degree\n        int wheelDegrees = ev->angleDelta().y() / 8;\n        int linesToScroll = abs(wheelDegrees) / 5;\n\n        QKeyEvent keyScrollEvent(QEvent::KeyPress,key,Qt::NoModifier);\n\n        for (int i=0;i<linesToScroll;i++)\n            emit keyPressedSignal(&keyScrollEvent, false);\n    }\n  }\n  else\n  {\n    // terminal program wants notification of mouse activity\n\n    int charLine;\n    int charColumn;\n    getCharacterPosition( ev->position() , charLine , charColumn );\n\n    emit mouseSignal( ev->angleDelta().y() > 0 ? 4 : 5,\n                      charColumn + 1,\n                      charLine + 1 +_scrollBar->value() -_scrollBar->maximum() ,\n                      0);\n  }\n}\n"""
new = """void TerminalDisplay::wheelEvent( QWheelEvent* ev )\n{\n  int verticalDelta = ev->angleDelta().y();\n  if (verticalDelta == 0)\n    verticalDelta = ev->pixelDelta().y();\n  if (verticalDelta == 0)\n    return;\n\n  // Hydra expects forced local-selection terminals such as embedded OpenCode\n  // to hand wheel intent back to tmux scrollback instead of trying to keep\n  // the event inside the widget-local history path.\n  const bool canScroll = _scrollBar->maximum() > 0;\n  if (canScroll && !_forceLocalMouseSelection)\n  {\n    _scrollBar->handleEvent(ev);\n    return;\n  }\n\n  int charLine;\n  int charColumn;\n  getCharacterPosition( ev->position() , charLine , charColumn );\n\n  emit mouseSignal( verticalDelta > 0 ? 4 : 5,\n                    charColumn + 1,\n                    charLine + 1 +_scrollBar->value() -_scrollBar->maximum() ,\n                    0);\n}\n"""

if old not in text:
    raise SystemExit("Hydra qmltermwidget wheelEvent patch target not found.")

path.write_text(text.replace(old, new))
PY

python3 - <<'PY'
from pathlib import Path

path = Path("/tmp/hydra_native_terminal_plugin_build/qmltermwidget/lib/TerminalDisplay.cpp")
text = path.read_text()
old = """void TerminalDisplay::mousePressEvent(QMouseEvent* ev)\n{\n  if ( _possibleTripleClick && (ev->button()==Qt::LeftButton) ) {\n"""
new = """void TerminalDisplay::mousePressEvent(QMouseEvent* ev)\n{\n  setFocus(true);\n  forceActiveFocus(Qt::MouseFocusReason);\n\n  if ( _possibleTripleClick && (ev->button()==Qt::LeftButton) ) {\n"""

if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget mouse focus patch target not found.")
    text = text.replace(old, new)

path.write_text(text)
PY

python3 - <<'PY'
from pathlib import Path

path = Path("/tmp/hydra_native_terminal_plugin_build/qmltermwidget/lib/TerminalDisplay.cpp")
text = path.read_text()
old = """  else if ( ev->button() == Qt::RightButton )\n  {\n    if (_mouseMarks || (ev->modifiers() & Qt::ShiftModifier))\n        emit configureRequest(ev->pos());\n    else\n        emit mouseSignal( 2, charColumn +1, charLine +1 +_scrollBar->value() -_scrollBar->maximum() , 0);\n  }\n"""
new = """  else if ( ev->button() == Qt::RightButton )\n  {\n    emit configureRequest(ev->pos());\n  }\n"""

if old not in text:
    raise SystemExit("Hydra qmltermwidget right-click press patch target not found.")

path.write_text(text.replace(old, new))
PY

python3 - <<'PY'
from pathlib import Path

path = Path("/tmp/hydra_native_terminal_plugin_build/qmltermwidget/lib/TerminalDisplay.cpp")
text = path.read_text()
old = """\n\n  if ( !_mouseMarks &&\n       ((ev->button() == Qt::RightButton && !(ev->modifiers() & Qt::ShiftModifier))\n                        || ev->button() == Qt::MiddleButton) )\n  {\n    emit mouseSignal( ev->button() == Qt::MiddleButton ? 1 : 2,\n                      charColumn + 1,\n                      charLine + 1 +_scrollBar->value() -_scrollBar->maximum() ,\n                      2);\n  }\n}\n"""
new = """\n\n  if ( !_mouseMarks && ev->button() == Qt::MiddleButton )\n  {\n    emit mouseSignal( 1,\n                      charColumn + 1,\n                      charLine + 1 +_scrollBar->value() -_scrollBar->maximum() ,\n                      2);\n  }\n}\n"""

if old not in text:
    raise SystemExit("Hydra qmltermwidget right-click release patch target not found.")

path.write_text(text.replace(old, new))
PY

python3 - <<'PY'
from pathlib import Path

path = Path("/tmp/hydra_native_terminal_plugin_build/qmltermwidget/lib/ksession.h")
text = path.read_text()
old = "    Q_PROPERTY(QStringList  shellProgramArgs READ getShellProgramArgs WRITE setArgs)\n"
new = old + "    Q_PROPERTY(int historySize READ historySize WRITE setHistorySize NOTIFY historySizeChanged)\n"

if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget historySize patch target not found.")
    text = text.replace(old, new)
    path.write_text(text)
PY

python3 - <<'PY'
from pathlib import Path

path = Path("/tmp/hydra_native_terminal_plugin_build/qmltermwidget/lib/TerminalDisplay.h")
text = path.read_text()
old = "   Q_PROPERTY(bool useFBORendering      READ useFBORendering WRITE setUseFBORendering)\n"
new = old + "   Q_PROPERTY(bool hasSelection         READ hasSelection                        NOTIFY copyAvailable           )\n" \
          + "   Q_PROPERTY(QString selectedText     READ selectedTextValue                   NOTIFY copyAvailable           )\n" \
          + "   Q_PROPERTY(bool forceLocalMouseSelection READ forceLocalMouseSelection WRITE setForceLocalMouseSelection NOTIFY forceLocalMouseSelectionChanged)\n"

if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget hasSelection property patch target not found.")
    text = text.replace(old, new)

old = "    void selectionChanged();\n\n    // QMLTermWidget\n"
new = "    void selectionChanged();\n    bool hasSelection() const;\n    QString selectedTextValue() const;\n    void setForceLocalMouseSelection(bool forceLocalMouseSelection);\n    bool forceLocalMouseSelection() const;\n\n    // QMLTermWidget\n"
if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget hasSelection method patch target not found.")
    text = text.replace(old, new)

old = "    void copyAvailable(bool available);\n\tvoid termGetFocus();\n\tvoid termLostFocus();\n"
new = "    void copyAvailable(bool available);\n    void inputKeyTyped();\n    void forceLocalMouseSelectionChanged();\n\tvoid termGetFocus();\n\tvoid termLostFocus();\n"
if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget inputKeyTyped signal patch target not found.")
    text = text.replace(old, new)

path.write_text(text)
PY

python3 - <<'PY'
from pathlib import Path

path = Path("/tmp/hydra_native_terminal_plugin_build/qmltermwidget/lib/TerminalDisplay.cpp")
text = path.read_text()

old = ",_mouseMarks(false)\n,_disabledBracketedPasteMode(false)\n"
new = ",_mouseMarks(false)\n,_forceLocalMouseSelection(false)\n,_disabledBracketedPasteMode(false)\n"
if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget forceLocalMouseSelection constructor patch target not found.")
    text = text.replace(old, new)

old = """void TerminalDisplay::copyClipboard()\n{\n  if ( !_screenWindow )\n      return;\n\n  QString text = _screenWindow->selectedText(_preserveLineBreaks);\n  if (!text.isEmpty())\n    QApplication::clipboard()->setText(text);\n}\n"""
new = """void TerminalDisplay::copyClipboard()\n{\n  if ( !_screenWindow )\n      return;\n\n  QString text = _screenWindow->selectedText(_preserveLineBreaks);\n  if (!text.isEmpty())\n    QApplication::clipboard()->setText(text);\n}\n\nbool TerminalDisplay::hasSelection() const\n{\n  return _screenWindow && !_screenWindow->selectedText(false).isEmpty();\n}\n\nQString TerminalDisplay::selectedTextValue() const\n{\n  return _screenWindow ? _screenWindow->selectedText(_preserveLineBreaks) : QString();\n}\n"""
if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget hasSelection implementation patch target not found.")
    text = text.replace(old, new)

old = """void TerminalDisplay::keyPressEvent( QKeyEvent* event )\n{\n    _actSel=0; // Key stroke implies a screen update, so TerminalDisplay won't\n              // know where the current selection is.\n\n    if (_hasBlinkingCursor)\n    {\n      // see TerminalDisplay::setBlinkingCursor\n      _blinkCursorTimer->start(std::max(QApplication::cursorFlashTime(), 1000) / 2);\n      if (_cursorBlinking)\n        blinkCursorEvent();\n      else\n        _cursorBlinking = false;\n    }\n\n    emit keyPressedSignal(event, false);\n\n    event->accept();\n}\n"""
new = """void TerminalDisplay::keyPressEvent( QKeyEvent* event )\n{\n    _actSel=0; // Key stroke implies a screen update, so TerminalDisplay won't\n              // know where the current selection is.\n\n    if (_hasBlinkingCursor)\n    {\n      // see TerminalDisplay::setBlinkingCursor\n      _blinkCursorTimer->start(std::max(QApplication::cursorFlashTime(), 1000) / 2);\n      if (_cursorBlinking)\n        blinkCursorEvent();\n      else\n        _cursorBlinking = false;\n    }\n\n    emit keyPressedSignal(event, false);\n\n    if (!event->text().isEmpty() || event->key() == Qt::Key_Backspace\n        || event->key() == Qt::Key_Delete || event->key() == Qt::Key_Return\n        || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Tab)\n        emit inputKeyTyped();\n\n    event->accept();\n}\n"""
if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget keyPressEvent patch target not found.")
    text = text.replace(old, new)

old = """void TerminalDisplay::inputMethodEvent( QInputMethodEvent* event )\n{\n    QKeyEvent keyEvent(QEvent::KeyPress,0,Qt::NoModifier,event->commitString());\n    emit keyPressedSignal(&keyEvent, false);\n\n    _inputMethodData.preeditString = event->preeditString().toStdWString();\n    update(preeditRect() | _inputMethodData.previousPreeditRect);\n\n    event->accept();\n}\n"""
new = """void TerminalDisplay::inputMethodEvent( QInputMethodEvent* event )\n{\n    QKeyEvent keyEvent(QEvent::KeyPress,0,Qt::NoModifier,event->commitString());\n    emit keyPressedSignal(&keyEvent, false);\n    if (!event->commitString().isEmpty())\n        emit inputKeyTyped();\n\n    _inputMethodData.preeditString = event->preeditString().toStdWString();\n    update(preeditRect() | _inputMethodData.previousPreeditRect);\n\n    event->accept();\n}\n"""
if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget inputMethodEvent patch target not found.")
    text = text.replace(old, new)

old = """void TerminalDisplay::setUsesMouse(bool on)\n{\n    if (_mouseMarks != on) {\n        _mouseMarks = on;\n        setCursor( _mouseMarks ? Qt::IBeamCursor : Qt::ArrowCursor );\n        emit usesMouseChanged();\n    }\n}\n"""
new = """void TerminalDisplay::setUsesMouse(bool on)\n{\n    if (_mouseMarks != on) {\n        _mouseMarks = on;\n        setCursor((_mouseMarks || _forceLocalMouseSelection) ? Qt::IBeamCursor : Qt::ArrowCursor);\n        emit usesMouseChanged();\n    }\n}\n\nvoid TerminalDisplay::setForceLocalMouseSelection(bool on)\n{\n    if (_forceLocalMouseSelection != on) {\n        _forceLocalMouseSelection = on;\n        setCursor((_mouseMarks || _forceLocalMouseSelection) ? Qt::IBeamCursor : Qt::ArrowCursor);\n        emit forceLocalMouseSelectionChanged();\n    }\n}\n\nbool TerminalDisplay::forceLocalMouseSelection() const\n{\n    return _forceLocalMouseSelection;\n}\n"""
if new not in text:
    if old not in text:
        raise SystemExit("Hydra qmltermwidget forceLocalMouseSelection implementation patch target not found.")
    text = text.replace(old, new)

replacements = {
    "if (_mouseMarks || (ev->modifiers() & Qt::ShiftModifier))": "if (_mouseMarks || _forceLocalMouseSelection || (ev->modifiers() & Qt::ShiftModifier))",
    "if ( _mouseMarks || (ev->modifiers() & Qt::ShiftModifier) )": "if ( _mouseMarks || _forceLocalMouseSelection || (ev->modifiers() & Qt::ShiftModifier) )",
    "if (!_mouseMarks && !(ev->modifiers() & Qt::ShiftModifier))": "if (!(_mouseMarks || _forceLocalMouseSelection) && !(ev->modifiers() & Qt::ShiftModifier))",
    "if ( !_mouseMarks && ev->button() == Qt::MiddleButton )": "if ( !(_mouseMarks || _forceLocalMouseSelection) && ev->button() == Qt::MiddleButton )",
}
for old, new in replacements.items():
    if old in text:
        text = text.replace(old, new)

path.write_text(text)
PY

pushd "$BUILD_ROOT/qmltermwidget" >/dev/null
qmake6 qmltermwidget.pro
make -j2
popd >/dev/null

rm -rf "$RUNTIME_DIR"
mkdir -p "$RUNTIME_DIR"
cp -a "$BUILD_ROOT/qmltermwidget/QMLTermWidget" "$RUNTIME_DIR/"
RUNTIME_DIR="$RUNTIME_DIR" python3 - <<'PY'
import os
from pathlib import Path

runtime_dir = Path(os.environ["RUNTIME_DIR"]) / "QMLTermWidget"
qmldir = runtime_dir / "qmldir"
qmldir_text = qmldir.read_text()
if "typeinfo plugins.qmltypes" not in qmldir_text:
    qmldir.write_text(qmldir_text.rstrip() + "\ntypeinfo plugins.qmltypes\n")

color_dir = runtime_dir / "color-schemes"
color_dir.mkdir(parents=True, exist_ok=True)

palettes = {
    "HydraSteam": {
        "shellBg": "#3D4435",
        "shellDepth": "#31382A",
        "shellDepthSoft": "#384032",
        "borderDark": "#7C856F",
        "textOnDark": "#D1DAC7",
        "textOnDarkMuted": "#BFC7B5",
        "accentBronze": "#BDBA73",
        "accentEmber": "#9C995C",
        "accentReady": "#C8CEAB",
        "accentReadyDeep": "#69735E",
        "accentOrangeStrong": "#D0CA76",
        "accentSteel": "#75806B",
        "accentSteelBright": "#BCC3AF",
        "accentCream": "#D1DAC7",
        "accentSignalDeep": "#615F31",
        "accentPhosphor": "#D1DAC7",
        "accentPhosphorSoft": "#BAC2AE",
        "accentHermes": "#75806B",
        "accentHermesBright": "#AFB8A5",
        "danger": "#D7B277",
        "warning": "#CCBF68",
    },
    "HydraHermes": {
        "shellBg": "#000000",
        "shellDepth": "#000000",
        "shellDepthSoft": "#020812",
        "borderDark": "#1555C0",
        "textOnDark": "#E8F4FF",
        "textOnDarkMuted": "#86A9D6",
        "accentBronze": "#1F8DFF",
        "accentEmber": "#0D57C8",
        "accentReady": "#66C7FF",
        "accentReadyDeep": "#0E3D86",
        "accentOrangeStrong": "#63B7FF",
        "accentSteel": "#0D3D75",
        "accentSteelBright": "#7ACFFF",
        "accentCream": "#E8F4FF",
        "accentSignalDeep": "#0A3263",
        "accentPhosphor": "#66C7FF",
        "accentPhosphorSoft": "#9ADDFF",
        "accentHermes": "#1F8DFF",
        "accentHermesBright": "#8BDEFF",
        "danger": "#FF6B5E",
        "warning": "#F2BF52",
    },
    "HydraOpenAI": {
        "shellBg": "#F7F7F5",
        "shellDepth": "#EEF0EB",
        "shellDepthSoft": "#E5E8E1",
        "borderDark": "#D0D5CC",
        "textOnDark": "#101113",
        "textOnDarkMuted": "#6A726A",
        "accentBronze": "#101113",
        "accentEmber": "#2B312D",
        "accentReady": "#10A37F",
        "accentReadyDeep": "#0A6A53",
        "accentOrangeStrong": "#101113",
        "accentSteel": "#68746D",
        "accentSteelBright": "#9BA59D",
        "accentCream": "#FCFCFA",
        "accentSignalDeep": "#0A6A53",
        "accentPhosphor": "#10A37F",
        "accentPhosphorSoft": "#70C6B2",
        "accentHermes": "#2B312D",
        "accentHermesBright": "#68746D",
        "danger": "#BE624F",
        "warning": "#A88A49",
    },
    "HydraChatGPT": {
        "shellBg": "#343541",
        "shellDepth": "#202123",
        "shellDepthSoft": "#444654",
        "borderDark": "#565869",
        "textOnDark": "#ECECF1",
        "textOnDarkMuted": "#ACACBE",
        "accentBronze": "#10A37F",
        "accentEmber": "#0B7158",
        "accentReady": "#19C37D",
        "accentReadyDeep": "#0A7158",
        "accentOrangeStrong": "#D9D9E3",
        "accentSteel": "#565869",
        "accentSteelBright": "#ACACBE",
        "accentCream": "#ECECF1",
        "accentSignalDeep": "#075746",
        "accentPhosphor": "#19C37D",
        "accentPhosphorSoft": "#6EE7B7",
        "accentHermes": "#8E8EA0",
        "accentHermesBright": "#D9D9E3",
        "danger": "#F87171",
        "warning": "#FBBF24",
    },
    "HydraClaudePaper": {
        "shellBg": "#FAF9F5",
        "shellDepth": "#F0EEE6",
        "shellDepthSoft": "#E8E6DC",
        "borderDark": "#D8D3C6",
        "textOnDark": "#141413",
        "textOnDarkMuted": "#87867F",
        "accentBronze": "#D97757",
        "accentEmber": "#BF684A",
        "accentReady": "#788C5D",
        "accentReadyDeep": "#586744",
        "accentOrangeStrong": "#D97757",
        "accentSteel": "#8EA3BC",
        "accentSteelBright": "#6A9BCC",
        "accentCream": "#FAF9F5",
        "accentSignalDeep": "#9D5238",
        "accentPhosphor": "#788C5D",
        "accentPhosphorSoft": "#99A98A",
        "accentHermes": "#6A9BCC",
        "accentHermesBright": "#8FB2D6",
        "danger": "#B85A45",
        "warning": "#D97757",
    },
    "HydraClaudeInk": {
        "shellBg": "#141413",
        "shellDepth": "#131314",
        "shellDepthSoft": "#1B1A19",
        "borderDark": "#3D3D3A",
        "textOnDark": "#FAF9F5",
        "textOnDarkMuted": "#B0AEA5",
        "accentBronze": "#D97757",
        "accentEmber": "#B85A45",
        "accentReady": "#788C5D",
        "accentReadyDeep": "#4D5B3C",
        "accentOrangeStrong": "#D97757",
        "accentSteel": "#557A9F",
        "accentSteelBright": "#6A9BCC",
        "accentCream": "#FAF9F5",
        "accentSignalDeep": "#7E412E",
        "accentPhosphor": "#788C5D",
        "accentPhosphorSoft": "#96A786",
        "accentHermes": "#6A9BCC",
        "accentHermesBright": "#90B8E0",
        "danger": "#E0876D",
        "warning": "#D97757",
    },
    "HydraEva": {
        "shellBg": "#000000",
        "shellDepth": "#000000",
        "shellDepthSoft": "#000000",
        "borderDark": "#CC0000",
        "textOnDark": "#FFFFFF",
        "textOnDarkMuted": "#E04040",
        "accentBronze": "#CC0000",
        "accentEmber": "#7A2020",
        "accentReady": "#CC0000",
        "accentReadyDeep": "#441111",
        "accentOrangeStrong": "#CC0000",
        "accentSteel": "#882222",
        "accentSteelBright": "#DD4444",
        "accentCream": "#FFFFFF",
        "accentSignalDeep": "#331010",
        "accentPhosphor": "#CC0000",
        "accentPhosphorSoft": "#CC0000",
        "accentHermes": "#CC0000",
        "accentHermesBright": "#CC0000",
        "danger": "#FF0000",
        "warning": "#FFAA00",
    },
}

def rgb(hex_color: str) -> str:
    hex_color = hex_color.lstrip("#")
    return ",".join(str(int(hex_color[index:index + 2], 16)) for index in (0, 2, 4))

template = """[Background]
Color={Background}

[BackgroundFaint]
Color={BackgroundFaint}

[BackgroundIntense]
Color={BackgroundIntense}

[Color0]
Color={Color0}

[Color0Faint]
Color={Color0Faint}

[Color0Intense]
Color={Color0Intense}

[Color1]
Color={Color1}

[Color1Faint]
Color={Color1Faint}

[Color1Intense]
Color={Color1Intense}

[Color2]
Color={Color2}

[Color2Faint]
Color={Color2Faint}

[Color2Intense]
Color={Color2Intense}

[Color3]
Color={Color3}

[Color3Faint]
Color={Color3Faint}

[Color3Intense]
Color={Color3Intense}

[Color4]
Color={Color4}

[Color4Faint]
Color={Color4Faint}

[Color4Intense]
Color={Color4Intense}

[Color5]
Color={Color5}

[Color5Faint]
Color={Color5Faint}

[Color5Intense]
Color={Color5Intense}

[Color6]
Color={Color6}

[Color6Faint]
Color={Color6Faint}

[Color6Intense]
Color={Color6Intense}

[Color7]
Color={Color7}

[Color7Faint]
Color={Color7Faint}

[Color7Intense]
Color={Color7Intense}

[Foreground]
Color={Foreground}

[ForegroundFaint]
Color={ForegroundFaint}

[ForegroundIntense]
Color={ForegroundIntense}

[General]
Description={Description}
Opacity=1
Wallpaper=
"""

for name, palette in palettes.items():
    values = {
        "Description": name,
        "Background": rgb(palette["shellBg"]),
        "BackgroundFaint": rgb(palette["shellDepthSoft"]),
        "BackgroundIntense": rgb(palette["shellDepth"]),
        "Color0": rgb(palette["shellDepth"]),
        "Color0Faint": rgb(palette["shellDepthSoft"]),
        "Color0Intense": rgb(palette["borderDark"]),
        "Color1": rgb(palette["danger"]),
        "Color1Faint": rgb(palette["accentEmber"]),
        "Color1Intense": rgb(palette["danger"]),
        "Color2": rgb(palette["accentReady"]),
        "Color2Faint": rgb(palette["accentReadyDeep"]),
        "Color2Intense": rgb(palette["accentPhosphor"]),
        "Color3": rgb(palette["warning"]),
        "Color3Faint": rgb(palette["accentSignalDeep"]),
        "Color3Intense": rgb(palette["accentOrangeStrong"]),
        "Color4": rgb(palette["accentSteelBright"]),
        "Color4Faint": rgb(palette["accentSteel"]),
        "Color4Intense": rgb(palette["accentHermesBright"]),
        "Color5": rgb(palette["accentBronze"]),
        "Color5Faint": rgb(palette["accentEmber"]),
        "Color5Intense": rgb(palette["accentOrangeStrong"]),
        "Color6": rgb(palette["accentPhosphorSoft"]),
        "Color6Faint": rgb(palette["accentHermes"]),
        "Color6Intense": rgb(palette["accentHermesBright"]),
        "Color7": rgb(palette["textOnDark"]),
        "Color7Faint": rgb(palette["textOnDarkMuted"]),
        "Color7Intense": rgb(palette["accentCream"]),
        "Foreground": rgb(palette["textOnDark"]),
        "ForegroundFaint": rgb(palette["textOnDarkMuted"]),
        "ForegroundIntense": rgb(palette["accentCream"]),
    }
    (color_dir / f"{name}.colorscheme").write_text(template.format(**values))
PY
cat > "$RUNTIME_DIR/QMLTermWidget/plugins.qmltypes" <<'EOF'
import QtQuick.tooling 1.2

Module {
    Component {
        name: "QMLTermSession"
        accessSemantics: "reference"
        prototype: "QObject"
        exports: ["QMLTermWidget/QMLTermSession 2.0"]
        Property {
            name: "historySize"
            type: "int"
            index: 0
        }
        Property {
            name: "initialWorkingDirectory"
            type: "QString"
            index: 1
        }
        Property {
            name: "shellProgram"
            type: "QString"
            index: 2
        }
        Property {
            name: "shellProgramArgs"
            type: "QStringList"
            index: 3
        }
        Property {
            name: "hasActiveProcess"
            type: "bool"
            index: 4
            isReadonly: true
        }
        Method {
            name: "startShellProgram"
        }
    }
    Component {
        name: "QMLTermWidget"
        accessSemantics: "reference"
        prototype: "QQuickItem"
        exports: ["QMLTermWidget/QMLTermWidget 2.0"]
        Property {
            name: "focus"
            type: "bool"
            index: 0
        }
        Property {
            name: "activeFocus"
            type: "bool"
            index: 1
            isReadonly: true
        }
        Property {
            name: "activeFocusOnTab"
            type: "bool"
            index: 2
        }
        Property {
            name: "session"
            type: "QMLTermSession"
            index: 3
        }
        Property {
            name: "font"
            type: "QFont"
            index: 4
        }
        Property {
            name: "colorScheme"
            type: "QString"
            index: 5
        }
        Property {
            name: "enableBold"
            type: "bool"
            index: 6
        }
        Property {
            name: "enableItalic"
            type: "bool"
            index: 7
        }
        Property {
            name: "antialiasText"
            type: "bool"
            index: 8
        }
        Property {
            name: "useFBORendering"
            type: "bool"
            index: 9
        }
        Property {
            name: "hasSelection"
            type: "bool"
            index: 10
            isReadonly: true
        }
        Property {
            name: "selectedText"
            type: "QString"
            index: 11
            isReadonly: true
        }
        Property {
            name: "terminalSize"
            type: "QSizeF"
            index: 12
            isReadonly: true
        }
        Property {
            name: "screenLines"
            type: "int"
            index: 13
            isReadonly: true
        }
        Property {
            name: "screenColumns"
            type: "int"
            index: 14
            isReadonly: true
        }
        Property {
            name: "lines"
            type: "int"
            index: 15
            isReadonly: true
        }
        Property {
            name: "columns"
            type: "int"
            index: 16
            isReadonly: true
        }
        Property {
            name: "forceLocalMouseSelection"
            type: "bool"
            index: 17
        }
        Signal { name: "activeFocusChanged" }
        Signal {
            name: "copyAvailable"
            Parameter { name: "available"; type: "bool" }
        }
        Signal {
            name: "configureRequest"
            Parameter { name: "position"; type: "QPointF" }
        }
        Signal {
            name: "mouseSignal"
            Parameter { name: "button"; type: "int" }
            Parameter { name: "column"; type: "int" }
            Parameter { name: "line"; type: "int" }
            Parameter { name: "eventType"; type: "int" }
        }
        Signal { name: "inputKeyTyped" }
        Method {
            name: "copyClipboard"
        }
        Method {
            name: "pasteClipboard"
        }
        Method {
            name: "forceActiveFocus"
            Parameter { name: "reason"; type: "int" }
        }
        Method {
            name: "setSelectionStart"
            Parameter { name: "row"; type: "int" }
            Parameter { name: "column"; type: "int" }
        }
        Method {
            name: "setSelectionEnd"
            Parameter { name: "row"; type: "int" }
            Parameter { name: "column"; type: "int" }
        }
    }
}
EOF
