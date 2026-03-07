#include "domain/support/slug.hpp"

#include <QRegularExpression>

namespace hydra::domain::support {

QString slugifyLowerDash(const QString &value)
{
    QString slug = value.toLower();
    slug.replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("-"));
    slug.remove(QRegularExpression(QStringLiteral("(^-+|-+$)")));
    return slug;
}

}  // namespace hydra::domain::support
