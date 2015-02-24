#ifndef INFO
#define INFO
#include <QString>

//Информация о программе
//TODO: Не забывать инкрементировать версию;D
namespace Info {
static const int MAJOR = 1;
static const int MINOR = 5;

static const char* ApplicationName = "DLLCollector_recode";
static const char* OrganizationName = "Illusion";
}

//Ключи нактроек
namespace Settings {
namespace GUI {
    static const QString WindowSize = "GUI/WindowSize";
    static const QString EnvChecked = "GUI/EnvironmentChecked";
    static const QString VisibleLog = "GUI/VisibileLog";
}

namespace Profile {
    static const QString QtProfiles = "Profile/QtProfiles";
    static const QString SelectedProfil = "Profile/SelectedProfil";
}
}

#endif // INFO
