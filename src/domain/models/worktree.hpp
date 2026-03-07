#pragma once

#include <QString>

namespace hydra::domain {

struct Worktree {
    QString path;
    QString branchName;
    QString headCommit;
    bool isMain = false;
};

}  // namespace hydra::domain
