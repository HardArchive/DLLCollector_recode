#include "info.h"

//Информация о программе
namespace Info {
const int MAJOR = 1;
const int MINOR = 6;

const QString ApplicationName = "DLLCollector_recode";
const QString OrganizationName = "Illusion";
}

//Ключи нактроек
namespace Settings {
namespace GUI {
    const QString WindowSize = "GUI/WindowSize";
    const QString EnvChecked = "GUI/EnvironmentChecked";
    const QString VisibleLog = "GUI/VisibileLog";
}

namespace Profile {
    const QString QtProfiles = "Profile/QtProfiles";
    const QString SelectedProfil = "Profile/SelectedProfil";
}
}
