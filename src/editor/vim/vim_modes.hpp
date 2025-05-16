#pragma once

namespace Jino::Editor::Vim {

enum class Mode { Insert, Normal, Visual };

inline const char *modeToString(Mode mode) {
  switch (mode) {
  case Mode::Insert:
    return "INS";
  case Mode::Normal:
    return "NORM";
  case Mode::Visual:
    return "VIS";
  default:
    return "???";
  }
}

} // namespace Jino::Editor::Vim
