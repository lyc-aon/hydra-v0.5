#pragma once

#include <QString>

#include "domain/ports/terminal_backend.hpp"
#include "infrastructure/process/process_runner.hpp"

namespace hydra::infrastructure {

class TmuxTerminalBackend final : public domain::ports::TerminalBackend {
public:
    TmuxTerminalBackend();

    bool pasteText(const QString &target,
                   const QString &text,
                   QString *errorMessage = nullptr) const override;
    bool pasteTextAndSubmit(const QString &target,
                            const QString &text,
                            QString *errorMessage = nullptr) const override;
    bool sendSpecialKey(const QString &target,
                        const QString &keySequence,
                        int repeatCount = 1,
                        QString *errorMessage = nullptr) const override;
    bool prepareInteractiveAttach(const QString &sessionName,
                                  bool enableMouse,
                                  QString *errorMessage = nullptr) const override;
    bool scrollHistory(const QString &target,
                       int lineDelta,
                       QString *errorMessage = nullptr) const override;
    bool openExternalAttach(const QString &sessionName,
                            const QString &workingDirectory,
                            QString *errorMessage = nullptr) const override;

private:
    using CommandResult = process::ProcessResult;
    struct ScrollState {
        bool inCopyMode = false;
        int scrollPosition = 0;
    };

    bool loadTextBuffer(const QString &text,
                        QString *bufferName,
                        QString *errorMessage) const;
    ScrollState queryScrollState(const QString &target,
                                 QString *errorMessage = nullptr) const;
    CommandResult runTmuxCommand(const QStringList &arguments) const;
    bool commandSucceeded(const CommandResult &result, QString *errorMessage) const;

    QString m_tmuxPath;
};

}  // namespace hydra::infrastructure
