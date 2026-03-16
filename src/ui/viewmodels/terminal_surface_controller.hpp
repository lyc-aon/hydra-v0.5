#pragma once

#include <memory>

#include <QObject>

#include "domain/ports/terminal_backend.hpp"

namespace hydra::ui {

class TerminalSurfaceController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString sessionId READ sessionId NOTIFY sessionChanged)
    Q_PROPERTY(QString sessionName READ sessionName NOTIFY sessionChanged)
    Q_PROPERTY(QString providerKey READ providerKey NOTIFY sessionChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool active READ active NOTIFY sessionChanged)

public:
    explicit TerminalSurfaceController(std::shared_ptr<domain::ports::TerminalBackend> backend,
                                       QObject *parent = nullptr);

    QString sessionId() const;
    QString sessionName() const;
    QString providerKey() const;
    QString errorMessage() const;
    bool active() const;

    Q_INVOKABLE void bindSession(const QString &sessionId,
                                 const QString &sessionName,
                                 const QString &providerKey,
                                 const QString &tmuxSessionName,
                                 const QString &paneId,
                                 const QString &workingDirectory);
    Q_INVOKABLE bool pasteClipboard();
    Q_INVOKABLE bool copyText(const QString &text);
    Q_INVOKABLE bool killInputLine();
    Q_INVOKABLE bool openExternalAttach();
    Q_INVOKABLE bool prepareInteractiveAttach(const QString &sessionName);
    Q_INVOKABLE bool scrollHistory(int lineDelta);

signals:
    void sessionChanged();
    void errorMessageChanged();

private:
    bool sendSpecialKey(const QString &keySequence, int repeatCount = 1);
    bool sendClipboardText(const QString &text);
    void clearSession();
    void setErrorMessage(const QString &errorMessage);

    std::shared_ptr<domain::ports::TerminalBackend> m_backend;
    QString m_sessionId;
    QString m_sessionName;
    QString m_providerKey;
    QString m_tmuxSessionName;
    QString m_paneId;
    QString m_workingDirectory;
    QString m_errorMessage;
};

}  // namespace hydra::ui
