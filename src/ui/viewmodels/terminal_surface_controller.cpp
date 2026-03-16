#include "ui/viewmodels/terminal_surface_controller.hpp"

#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>

namespace hydra::ui {

TerminalSurfaceController::TerminalSurfaceController(
    std::shared_ptr<domain::ports::TerminalBackend> backend,
    QObject *parent)
    : QObject(parent),
      m_backend(std::move(backend))
{
}

QString TerminalSurfaceController::sessionId() const
{
    return m_sessionId;
}

QString TerminalSurfaceController::sessionName() const
{
    return m_sessionName;
}

QString TerminalSurfaceController::providerKey() const
{
    return m_providerKey;
}

QString TerminalSurfaceController::errorMessage() const
{
    return m_errorMessage;
}

bool TerminalSurfaceController::active() const
{
    return !m_sessionId.isEmpty() && !m_tmuxSessionName.isEmpty();
}

void TerminalSurfaceController::bindSession(const QString &sessionId,
                                            const QString &sessionName,
                                            const QString &providerKey,
                                            const QString &tmuxSessionName,
                                            const QString &paneId,
                                            const QString &workingDirectory)
{
    const QString nextSessionId = sessionId.trimmed();
    const QString nextSessionName = sessionName.trimmed();
    const QString nextProviderKey = providerKey.trimmed();
    const QString nextTmuxSessionName = tmuxSessionName.trimmed();
    const QString nextPaneId = paneId.trimmed();
    const QString nextWorkingDirectory = workingDirectory.trimmed();

    if (m_sessionId == nextSessionId && m_sessionName == nextSessionName
        && m_providerKey == nextProviderKey
        && m_tmuxSessionName == nextTmuxSessionName && m_paneId == nextPaneId
        && m_workingDirectory == nextWorkingDirectory)
    {
        return;
    }

    clearSession();
    m_sessionId = nextSessionId;
    m_sessionName = nextSessionName;
    m_providerKey = nextProviderKey;
    m_tmuxSessionName = nextTmuxSessionName;
    m_paneId = nextPaneId;
    m_workingDirectory = nextWorkingDirectory;
    setErrorMessage(QString());
    emit sessionChanged();

    if (!active()) {
        clearSession();
    }
}

bool TerminalSurfaceController::pasteClipboard()
{
    if (!active()) {
        return false;
    }

    const QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard == nullptr) {
        return false;
    }

    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData != nullptr && mimeData->hasImage()) {
        if (m_providerKey == QStringLiteral("codex")) {
            return sendSpecialKey(QStringLiteral("C-v"));
        }

        setErrorMessage(QStringLiteral("Image paste is not available for this provider yet."));
        return false;
    }

    return sendClipboardText(clipboard->text());
}

bool TerminalSurfaceController::copyText(const QString &text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard == nullptr) {
        return false;
    }

    const QString normalized = text;
    if (normalized.isEmpty()) {
        return false;
    }

    clipboard->setText(normalized, QClipboard::Clipboard);
    if (clipboard->supportsSelection()) {
        clipboard->setText(normalized, QClipboard::Selection);
    }
    return true;
}

bool TerminalSurfaceController::killInputLine()
{
    if (!active() || m_backend == nullptr) {
        return false;
    }

    // readline: beginning-of-line (C-a) then kill-to-end (C-k)
    QString errorMessage;
    m_backend->sendSpecialKey(m_tmuxSessionName, QStringLiteral("C-a"), 1, &errorMessage);
    const bool ok =
        m_backend->sendSpecialKey(m_tmuxSessionName, QStringLiteral("C-k"), 1, &errorMessage);
    setErrorMessage(ok ? QString() : errorMessage);
    return ok;
}

bool TerminalSurfaceController::openExternalAttach()
{
    if (!active() || m_backend == nullptr) {
        return false;
    }

    QString errorMessage;
    const bool ok =
        m_backend->openExternalAttach(m_tmuxSessionName, m_workingDirectory, &errorMessage);
    setErrorMessage(ok ? QString() : errorMessage);
    return ok;
}

bool TerminalSurfaceController::prepareInteractiveAttach(const QString &sessionName)
{
    if (m_backend == nullptr) {
        return false;
    }

    const QString target = sessionName.trimmed();
    if (target.isEmpty()) {
        return false;
    }

    QString errorMessage;
    const bool enableMouse = m_providerKey == QStringLiteral("opencode");
    const bool ok = m_backend->prepareInteractiveAttach(target,
                                                        enableMouse,
                                                        &errorMessage);
    if (!ok && !errorMessage.trimmed().isEmpty()) {
        setErrorMessage(errorMessage);
    }
    return ok;
}

bool TerminalSurfaceController::scrollHistory(const int lineDelta)
{
    if (!active() || m_backend == nullptr) {
        return false;
    }

    QString errorMessage;
    const bool ok = m_backend->scrollHistory(m_tmuxSessionName, lineDelta, &errorMessage);
    setErrorMessage(ok ? QString() : errorMessage);
    return ok;
}

bool TerminalSurfaceController::sendSpecialKey(const QString &keySequence, const int repeatCount)
{
    if (m_backend == nullptr || !active()) {
        return false;
    }

    QString errorMessage;
    const bool ok = m_backend->sendSpecialKey(m_tmuxSessionName, keySequence, repeatCount, &errorMessage);
    setErrorMessage(ok ? QString() : errorMessage);
    return ok;
}

bool TerminalSurfaceController::sendClipboardText(const QString &text)
{
    if (text.isEmpty() || m_backend == nullptr || !active()) {
        return false;
    }

    QString normalized = text;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.remove(u'\r');

    QString errorMessage;
    const bool ok = m_backend->pasteText(m_tmuxSessionName, normalized, &errorMessage);
    setErrorMessage(ok ? QString() : errorMessage);
    return ok;
}

void TerminalSurfaceController::clearSession()
{
    m_sessionId.clear();
    m_sessionName.clear();
    m_providerKey.clear();
    m_tmuxSessionName.clear();
    m_paneId.clear();
    m_workingDirectory.clear();
}

void TerminalSurfaceController::setErrorMessage(const QString &errorMessage)
{
    if (m_errorMessage == errorMessage) {
        return;
    }

    m_errorMessage = errorMessage;
    emit errorMessageChanged();
}

}  // namespace hydra::ui
