#pragma once

#include <QString>

namespace hydra::domain::support {

[[nodiscard]] QString slugifyLowerDash(const QString &value);

}  // namespace hydra::domain::support
