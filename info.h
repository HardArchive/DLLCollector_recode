#ifndef INFO
#define INFO
#include <QString>

//TODO: Не забывать инкрементировать версию;D

//Информация о программе
namespace Info {
    extern const int MAJOR;
    extern const int MINOR;

    extern const QString ApplicationName;
    extern const QString OrganizationName;
}

//Ключи нактроек
namespace Settings {
    namespace GUI {
        extern const QString WindowSize;
        extern const QString EnvChecked;
        extern const QString VisibleLog;
    }

    namespace Profile {
        extern const QString QtProfiles;
        extern const QString SelectedProfil;
    }
}

#endif // INFO
