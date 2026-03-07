#pragma once

#include <QDateTime>
#include <QString>

namespace hydra::domain {

struct Repository {
    QString id;
    QString name;
    QString path;
    QString description;
    QString accentColor;
    QDateTime createdAt;
    QDateTime updatedAt;
};

}  // namespace hydra::domain
