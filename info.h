#ifndef INFO
#define INFO
#include <QString>

//TODO: Не забывать инкрементировать версию;D

//Информация о программе
namespace Info
{
    extern const int MAJOR;
    extern const int MINOR;
    
    extern const QString ApplicationName;
    extern const QString OrganizationName;
}

//Ключи нактроек
namespace Settings
{
    namespace GUI
    {
        extern const QString WindowSize;
        extern const QString EnvChecked;
        extern const QString VisibleLog;
    }
    
    namespace Profile
    {
        extern const QString QtProfile;
        extern const QString SelectedProfil;
        
        namespace Keys
        {
            extern const QString Inclusions;
            extern const QString QtLibs;
            extern const QString QtPlugins;
        }
    }
}

#endif // INFO
