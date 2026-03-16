#pragma once

#include <QObject>
#include <QVariantList>

namespace hydra::ui {

class ThemeState : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentThemeId READ currentThemeId WRITE setCurrentThemeId NOTIFY currentThemeIdChanged)
    Q_PROPERTY(QString currentThemeLabel READ currentThemeLabel NOTIFY currentThemeLabelChanged)
    Q_PROPERTY(QVariantList themeOptions READ themeOptions CONSTANT)

public:
    struct ThemeOption {
        const char *id;
        const char *label;
    };

    explicit ThemeState(QObject *parent = nullptr);

    [[nodiscard]] QString currentThemeId() const;
    [[nodiscard]] QString currentThemeLabel() const;
    [[nodiscard]] QVariantList themeOptions() const;

public slots:
    void setCurrentThemeId(const QString &themeId);
    void cycleTheme();

signals:
    void currentThemeIdChanged();
    void currentThemeLabelChanged();

private:
    static bool isValidThemeId(const QString &themeId);
    static QString labelForThemeId(const QString &themeId);

    QString m_currentThemeId;
};

}  // namespace hydra::ui
