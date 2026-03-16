#pragma once

#include <QString>

namespace hydra::domain::ports {

class TerminalBackend {
public:
    virtual ~TerminalBackend() = default;

    virtual bool pasteText(const QString &target,
                           const QString &text,
                           QString *errorMessage = nullptr) const = 0;
    virtual bool pasteTextAndSubmit(const QString &target,
                                    const QString &text,
                                    QString *errorMessage = nullptr) const
    {
        if (!pasteText(target, text, errorMessage)) {
            return false;
        }
        return sendSpecialKey(target, QStringLiteral("Enter"), 1, errorMessage);
    }
    virtual bool sendSpecialKey(const QString &target,
                                const QString &keySequence,
                                int repeatCount = 1,
                                QString *errorMessage = nullptr) const = 0;
    virtual bool prepareInteractiveAttach(const QString &sessionName,
                                          bool enableMouse,
                                          QString *errorMessage = nullptr) const = 0;
    virtual bool scrollHistory(const QString &target,
                               int lineDelta,
                               QString *errorMessage = nullptr) const = 0;
    virtual bool openExternalAttach(const QString &sessionName,
                                    const QString &workingDirectory,
                                    QString *errorMessage = nullptr) const = 0;
};

}  // namespace hydra::domain::ports
