#pragma once

#include <QString>

namespace hydra::ui::router_presets {

inline constexpr char kDirectKey[] = "direct";
inline constexpr char kStandardKey[] = "standard";
inline constexpr char kStrategicKey[] = "strategic";
inline constexpr char kCustomPresetPrefix[] = "custom:";

bool isBuiltInKey(const QString &presetKey);
QString builtInLabel(const QString &presetKey);
QString builtInDescription(const QString &presetKey);
QString builtInPrompt(const QString &presetKey, const QString &userDefaultContext);
QString makeCustomPresetId();
QString controlGuideContents();
QString controlScriptPath();
QString wrapperContents();

}  // namespace hydra::ui::router_presets
