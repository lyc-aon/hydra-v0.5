#include "ui/viewmodels/theme_state.hpp"

#include <QSettings>
#include <QStringLiteral>
#include <array>

namespace hydra::ui {

namespace {

constexpr auto kThemeOptions = std::array{
    ThemeState::ThemeOption{"og_steam", "STEAM"},
    ThemeState::ThemeOption{"hermes_veil", "HERMES"},
    ThemeState::ThemeOption{"openai", "OPENAI"},
    ThemeState::ThemeOption{"chatgpt", "CHATGPT"},
    ThemeState::ThemeOption{"claude_paper", "CLAUDE"},
    ThemeState::ThemeOption{"claude_ink", "CLAUDE INK"},
    ThemeState::ThemeOption{"eva", "HYDRA"},
};

constexpr auto kThemeSettingKey = "appearance/current_theme_id";

}  // namespace

ThemeState::ThemeState(QObject *parent)
    : QObject(parent),
      m_currentThemeId(QStringLiteral("og_steam"))
{
    const QString persistedThemeId =
        QSettings().value(QString::fromUtf8(kThemeSettingKey), m_currentThemeId).toString();
    if (isValidThemeId(persistedThemeId)) {
        m_currentThemeId = persistedThemeId;
    }
}

QString ThemeState::currentThemeId() const
{
    return m_currentThemeId;
}

QString ThemeState::currentThemeLabel() const
{
    return labelForThemeId(m_currentThemeId);
}

QVariantList ThemeState::themeOptions() const
{
    QVariantList options;
    options.reserve(static_cast<qsizetype>(kThemeOptions.size()));

    for (const ThemeOption &option : kThemeOptions) {
        QVariantMap entry;
        entry.insert(QStringLiteral("id"), QString::fromUtf8(option.id));
        entry.insert(QStringLiteral("label"), QString::fromUtf8(option.label));
        options.push_back(entry);
    }

    return options;
}

void ThemeState::setCurrentThemeId(const QString &themeId)
{
    if (!isValidThemeId(themeId) || themeId == m_currentThemeId) {
        return;
    }

    m_currentThemeId = themeId;
    QSettings().setValue(QString::fromUtf8(kThemeSettingKey), m_currentThemeId);
    emit currentThemeIdChanged();
    emit currentThemeLabelChanged();
}

void ThemeState::cycleTheme()
{
    for (qsizetype index = 0; index < static_cast<qsizetype>(kThemeOptions.size()); ++index) {
        if (m_currentThemeId == QString::fromUtf8(kThemeOptions.at(index).id)) {
            const qsizetype nextIndex = (index + 1) % static_cast<qsizetype>(kThemeOptions.size());
            setCurrentThemeId(QString::fromUtf8(kThemeOptions.at(nextIndex).id));
            return;
        }
    }

    setCurrentThemeId(QStringLiteral("og_steam"));
}

bool ThemeState::isValidThemeId(const QString &themeId)
{
    for (const ThemeOption &option : kThemeOptions) {
        if (themeId == QString::fromUtf8(option.id)) {
            return true;
        }
    }

    return false;
}

QString ThemeState::labelForThemeId(const QString &themeId)
{
    for (const ThemeOption &option : kThemeOptions) {
        if (themeId == QString::fromUtf8(option.id)) {
            return QString::fromUtf8(option.label);
        }
    }

    return QStringLiteral("STEAM");
}

}  // namespace hydra::ui
