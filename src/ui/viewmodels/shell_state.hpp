#pragma once

#include <QObject>

namespace hydra::ui {

class ShellState : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool sidebarCollapsed READ sidebarCollapsed WRITE setSidebarCollapsed NOTIFY sidebarCollapsedChanged)
    Q_PROPERTY(int sidebarWidth READ sidebarWidth WRITE setSidebarWidth NOTIFY sidebarWidthChanged)
    Q_PROPERTY(bool fullscreen READ fullscreen WRITE setFullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(double wideSessionBoardFraction READ wideSessionBoardFraction WRITE setWideSessionBoardFraction NOTIFY wideSessionBoardFractionChanged)
    Q_PROPERTY(double stackedSessionBoardFraction READ stackedSessionBoardFraction WRITE setStackedSessionBoardFraction NOTIFY stackedSessionBoardFractionChanged)
    Q_PROPERTY(QString activeViewMode READ activeViewMode WRITE setActiveViewMode NOTIFY activeViewModeChanged)
    Q_PROPERTY(bool masterOrbitCollapsed READ masterOrbitCollapsed WRITE setMasterOrbitCollapsed NOTIFY masterOrbitCollapsedChanged)
    Q_PROPERTY(bool masterRouterCollapsed READ masterRouterCollapsed WRITE setMasterRouterCollapsed NOTIFY masterRouterCollapsedChanged)
    Q_PROPERTY(bool masterWaveformCollapsed READ masterWaveformCollapsed WRITE setMasterWaveformCollapsed NOTIFY masterWaveformCollapsedChanged)

public:
    explicit ShellState(QObject *parent = nullptr);

    [[nodiscard]] bool sidebarCollapsed() const;
    void setSidebarCollapsed(bool sidebarCollapsed);

    [[nodiscard]] int sidebarWidth() const;
    void setSidebarWidth(int sidebarWidth);

    [[nodiscard]] bool fullscreen() const;
    void setFullscreen(bool fullscreen);

    [[nodiscard]] double wideSessionBoardFraction() const;
    void setWideSessionBoardFraction(double fraction);

    [[nodiscard]] double stackedSessionBoardFraction() const;
    void setStackedSessionBoardFraction(double fraction);

    [[nodiscard]] QString activeViewMode() const;
    void setActiveViewMode(const QString &activeViewMode);

    [[nodiscard]] bool masterOrbitCollapsed() const;
    void setMasterOrbitCollapsed(bool collapsed);

    [[nodiscard]] bool masterRouterCollapsed() const;
    void setMasterRouterCollapsed(bool collapsed);

    [[nodiscard]] bool masterWaveformCollapsed() const;
    void setMasterWaveformCollapsed(bool collapsed);

signals:
    void sidebarCollapsedChanged();
    void sidebarWidthChanged();
    void fullscreenChanged();
    void wideSessionBoardFractionChanged();
    void stackedSessionBoardFractionChanged();
    void activeViewModeChanged();
    void masterOrbitCollapsedChanged();
    void masterRouterCollapsedChanged();
    void masterWaveformCollapsedChanged();

private:
    static double clampWideFraction(double value);
    static double clampStackedFraction(double value);

    bool m_sidebarCollapsed = false;
    int m_sidebarWidth = 0;
    bool m_fullscreen = false;
    double m_wideSessionBoardFraction = 0.42;
    double m_stackedSessionBoardFraction = 0.45;
    QString m_activeViewMode = QStringLiteral("workbench");
    bool m_masterOrbitCollapsed = false;
    bool m_masterRouterCollapsed = true;
    bool m_masterWaveformCollapsed = true;
};

}  // namespace hydra::ui
