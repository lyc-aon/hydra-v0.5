#include "ui/viewmodels/shell_state.hpp"

#include <QSettings>

#include <algorithm>

namespace hydra::ui {

namespace {

constexpr auto kSidebarCollapsedKey = "shell/sidebar_collapsed";
constexpr auto kSidebarWidthKey = "shell/sidebar_width";
constexpr auto kFullscreenKey = "shell/fullscreen";
constexpr auto kWideSessionBoardFractionKey = "shell/wide_session_board_fraction";
constexpr auto kStackedSessionBoardFractionKey = "shell/stacked_session_board_fraction";
constexpr auto kActiveViewModeKey = "shell/active_view_mode";
constexpr auto kMasterOrbitCollapsedKey = "shell/master_orbit_collapsed";
constexpr auto kMasterRouterCollapsedKey = "shell/master_router_collapsed";
constexpr auto kMasterWaveformCollapsedKey = "shell/master_waveform_collapsed";

}  // namespace

ShellState::ShellState(QObject *parent)
    : QObject(parent)
{
    QSettings settings;
    m_sidebarCollapsed = settings.value(QString::fromUtf8(kSidebarCollapsedKey), false).toBool();
    m_sidebarWidth = std::max(0, settings.value(QString::fromUtf8(kSidebarWidthKey), 0).toInt());
    m_fullscreen = settings.value(QString::fromUtf8(kFullscreenKey), true).toBool();
    m_wideSessionBoardFraction = clampWideFraction(
        settings.value(QString::fromUtf8(kWideSessionBoardFractionKey), m_wideSessionBoardFraction)
            .toDouble());
    m_stackedSessionBoardFraction = clampStackedFraction(
        settings.value(QString::fromUtf8(kStackedSessionBoardFractionKey),
                       m_stackedSessionBoardFraction)
            .toDouble());
    const QString viewMode = settings.value(QString::fromUtf8(kActiveViewModeKey),
                                            m_activeViewMode).toString();
    m_activeViewMode = (viewMode == QStringLiteral("master")) ? viewMode : QStringLiteral("workbench");
    m_masterOrbitCollapsed = settings.value(QString::fromUtf8(kMasterOrbitCollapsedKey), false).toBool();
    m_masterRouterCollapsed = settings.value(QString::fromUtf8(kMasterRouterCollapsedKey), true).toBool();
    m_masterWaveformCollapsed = settings.value(QString::fromUtf8(kMasterWaveformCollapsedKey), true).toBool();
}

bool ShellState::sidebarCollapsed() const
{
    return m_sidebarCollapsed;
}

void ShellState::setSidebarCollapsed(const bool sidebarCollapsed)
{
    if (m_sidebarCollapsed == sidebarCollapsed) {
        return;
    }

    m_sidebarCollapsed = sidebarCollapsed;
    QSettings settings;
    settings.setValue(QString::fromUtf8(kSidebarCollapsedKey), m_sidebarCollapsed);
    settings.sync();
    emit sidebarCollapsedChanged();
}

int ShellState::sidebarWidth() const
{
    return m_sidebarWidth;
}

void ShellState::setSidebarWidth(const int sidebarWidth)
{
    const int normalizedWidth = std::max(0, sidebarWidth);
    if (m_sidebarWidth == normalizedWidth) {
        return;
    }

    m_sidebarWidth = normalizedWidth;
    QSettings settings;
    settings.setValue(QString::fromUtf8(kSidebarWidthKey), m_sidebarWidth);
    settings.sync();
    emit sidebarWidthChanged();
}

bool ShellState::fullscreen() const
{
    return m_fullscreen;
}

void ShellState::setFullscreen(const bool fullscreen)
{
    if (m_fullscreen == fullscreen) {
        return;
    }

    m_fullscreen = fullscreen;
    QSettings settings;
    settings.setValue(QString::fromUtf8(kFullscreenKey), m_fullscreen);
    settings.sync();
    emit fullscreenChanged();
}

double ShellState::wideSessionBoardFraction() const
{
    return m_wideSessionBoardFraction;
}

void ShellState::setWideSessionBoardFraction(const double fraction)
{
    const double normalizedFraction = clampWideFraction(fraction);
    if (qFuzzyCompare(m_wideSessionBoardFraction, normalizedFraction)) {
        return;
    }

    m_wideSessionBoardFraction = normalizedFraction;
    QSettings settings;
    settings.setValue(QString::fromUtf8(kWideSessionBoardFractionKey),
                      m_wideSessionBoardFraction);
    settings.sync();
    emit wideSessionBoardFractionChanged();
}

double ShellState::stackedSessionBoardFraction() const
{
    return m_stackedSessionBoardFraction;
}

void ShellState::setStackedSessionBoardFraction(const double fraction)
{
    const double normalizedFraction = clampStackedFraction(fraction);
    if (qFuzzyCompare(m_stackedSessionBoardFraction, normalizedFraction)) {
        return;
    }

    m_stackedSessionBoardFraction = normalizedFraction;
    QSettings settings;
    settings.setValue(QString::fromUtf8(kStackedSessionBoardFractionKey),
                      m_stackedSessionBoardFraction);
    settings.sync();
    emit stackedSessionBoardFractionChanged();
}

QString ShellState::activeViewMode() const
{
    return m_activeViewMode;
}

void ShellState::setActiveViewMode(const QString &activeViewMode)
{
    const QString normalized = (activeViewMode == QStringLiteral("master"))
                                   ? activeViewMode
                                   : QStringLiteral("workbench");
    if (m_activeViewMode == normalized) {
        return;
    }

    m_activeViewMode = normalized;
    QSettings settings;
    settings.setValue(QString::fromUtf8(kActiveViewModeKey), m_activeViewMode);
    settings.sync();
    emit activeViewModeChanged();
}

bool ShellState::masterOrbitCollapsed() const
{
    return m_masterOrbitCollapsed;
}

void ShellState::setMasterOrbitCollapsed(const bool collapsed)
{
    if (m_masterOrbitCollapsed == collapsed) {
        return;
    }

    m_masterOrbitCollapsed = collapsed;
    QSettings settings;
    settings.setValue(QString::fromUtf8(kMasterOrbitCollapsedKey), m_masterOrbitCollapsed);
    settings.sync();
    emit masterOrbitCollapsedChanged();
}

bool ShellState::masterRouterCollapsed() const
{
    return m_masterRouterCollapsed;
}

void ShellState::setMasterRouterCollapsed(const bool collapsed)
{
    if (m_masterRouterCollapsed == collapsed) {
        return;
    }

    m_masterRouterCollapsed = collapsed;
    QSettings settings;
    settings.setValue(QString::fromUtf8(kMasterRouterCollapsedKey), m_masterRouterCollapsed);
    settings.sync();
    emit masterRouterCollapsedChanged();
}

bool ShellState::masterWaveformCollapsed() const
{
    return m_masterWaveformCollapsed;
}

void ShellState::setMasterWaveformCollapsed(const bool collapsed)
{
    if (m_masterWaveformCollapsed == collapsed) {
        return;
    }

    m_masterWaveformCollapsed = collapsed;
    QSettings settings;
    settings.setValue(QString::fromUtf8(kMasterWaveformCollapsedKey), m_masterWaveformCollapsed);
    settings.sync();
    emit masterWaveformCollapsedChanged();
}

double ShellState::clampWideFraction(const double value)
{
    return std::clamp(value, 0.28, 0.72);
}

double ShellState::clampStackedFraction(const double value)
{
    return std::clamp(value, 0.30, 0.70);
}

}  // namespace hydra::ui
