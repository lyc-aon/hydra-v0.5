#include <QClipboard>
#include <QGuiApplication>
#include <QImage>
#include <QTextStream>

#include <memory>
#include <stdexcept>

#include "domain/ports/terminal_backend.hpp"
#include "ui/viewmodels/terminal_surface_controller.hpp"

namespace {

using hydra::domain::ports::TerminalBackend;

void require(const bool condition, const QString &message)
{
    if (!condition) {
        throw std::runtime_error(message.toStdString());
    }
}

class FakeTerminalBackend final : public TerminalBackend {
public:
    bool pasteText(const QString &, const QString &text, QString *errorMessage) const override
    {
        lastPastedText = text;
        ++pasteCount;
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    bool sendSpecialKey(const QString &, const QString &keySequence, int repeatCount, QString *errorMessage) const override
    {
        lastSpecialKey = keySequence;
        lastSpecialRepeat = repeatCount;
        ++specialCount;
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    bool prepareInteractiveAttach(const QString &, bool enableMouse, QString *errorMessage) const override
    {
        lastMouseEnabled = enableMouse;
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    bool scrollHistory(const QString &, int, QString *errorMessage) const override
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    bool openExternalAttach(const QString &, const QString &, QString *errorMessage) const override
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    mutable QString lastPastedText;
    mutable QString lastSpecialKey;
    mutable int lastSpecialRepeat = 0;
    mutable int pasteCount = 0;
    mutable int specialCount = 0;
    mutable bool lastMouseEnabled = false;
};

}  // namespace

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    QGuiApplication app(argc, argv);

    auto backend = std::make_shared<FakeTerminalBackend>();
    hydra::ui::TerminalSurfaceController controller(backend);
    controller.bindSession(QStringLiteral("session-1"),
                           QStringLiteral("Hydra V2 [Codex]"),
                           QStringLiteral("codex"),
                           QStringLiteral("hydra-test"),
                           QStringLiteral("%0"),
                           QStringLiteral("/tmp"));

    QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::red);
    QGuiApplication::clipboard()->setImage(image);
    require(controller.pasteClipboard(), QStringLiteral("Codex image paste should succeed"));
    require(backend->specialCount == 1 && backend->lastSpecialKey == QStringLiteral("C-v"),
            QStringLiteral("Codex image paste should route through Ctrl+V"));
    require(backend->pasteCount == 0,
            QStringLiteral("Codex image paste should not use tmux text paste"));

    QGuiApplication::clipboard()->setText(QStringLiteral("line one\nline two"));
    require(controller.pasteClipboard(), QStringLiteral("Codex text paste should succeed"));
    require(backend->pasteCount == 1
                && backend->lastPastedText == QStringLiteral("line one\nline two"),
            QStringLiteral("Codex text paste should still use bracketed text paste"));

    require(controller.prepareInteractiveAttach(QStringLiteral("hydra-test")),
            QStringLiteral("Codex interactive attach should succeed"));
    require(!backend->lastMouseEnabled,
            QStringLiteral("Codex interactive attach should keep tmux mouse disabled"));

    controller.bindSession(QStringLiteral("session-2"),
                           QStringLiteral("Hydra V2 [Gemini]"),
                           QStringLiteral("gemini"),
                           QStringLiteral("hydra-test"),
                           QStringLiteral("%1"),
                           QStringLiteral("/tmp"));
    QGuiApplication::clipboard()->setImage(image);
    require(!controller.pasteClipboard(),
            QStringLiteral("Non-Codex image paste should remain unavailable"));
    require(controller.errorMessage().contains(QStringLiteral("Image paste is not available")),
            QStringLiteral("Unsupported image paste should expose a useful error"));

    controller.bindSession(QStringLiteral("session-3"),
                           QStringLiteral("Hydra V2 [Hermes]"),
                           QStringLiteral("hermes"),
                           QStringLiteral("hydra-test"),
                           QStringLiteral("%2"),
                           QStringLiteral("/tmp"));
    QGuiApplication::clipboard()->setText(QStringLiteral("line one\nline two"));
    require(controller.pasteClipboard(), QStringLiteral("Hermes text paste should succeed"));
    require(backend->lastPastedText == QStringLiteral("line one\nline two"),
            QStringLiteral("Hermes controller paste should remain a direct tmux fallback"));

    controller.bindSession(QStringLiteral("session-4"),
                           QStringLiteral("Hydra V2 [OpenCode]"),
                           QStringLiteral("opencode"),
                           QStringLiteral("hydra-test"),
                           QStringLiteral("%3"),
                           QStringLiteral("/tmp"));
    require(controller.prepareInteractiveAttach(QStringLiteral("hydra-test")),
            QStringLiteral("OpenCode interactive attach should succeed"));
    require(backend->lastMouseEnabled,
            QStringLiteral("OpenCode interactive attach should enable tmux mouse so the native alt-screen UI keeps wheel scrolling."));

    controller.bindSession(QStringLiteral("session-5"),
                           QStringLiteral("Hydra V2 [Hermes]"),
                           QStringLiteral("hermes"),
                           QStringLiteral("hydra-test"),
                           QStringLiteral("%4"),
                           QStringLiteral("/tmp"));
    QGuiApplication::clipboard()->setText(QStringLiteral("single line"));
    require(controller.pasteClipboard(), QStringLiteral("Hermes single-line paste should succeed"));
    require(backend->lastPastedText == QStringLiteral("single line"),
            QStringLiteral("Hermes single-line paste should remain direct"));

    QTextStream(stdout) << "terminal clipboard routing smoke passed" << Qt::endl;
    return 0;
}
