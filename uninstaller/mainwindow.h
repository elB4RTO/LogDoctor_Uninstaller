#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <QStandardPaths>
#include <QString>
#include <QTimer>

#include <filesystem>
#include <string>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:

    // language

    void menu_actionEnglish_triggered();

    void menu_actionEspanol_triggered();

    void menu_actionFrancais_triggered();

    void menu_actionItaliano_triggered();

    // uninstallation

    void Uninstall();

    void checkUninstallProgress();

    // actions

    void on_checkBox_ConfigFile_toggled(bool checked);

    void on_checkBox_Databases_toggled(bool checked);

    void on_button_Uninstall_clicked();

    void on_button_Close_clicked();

private:
    QSharedPointer<Ui::MainWindow> ui;

    // operating system
    const std::string cleanPath( const QString& path );

    const std::string home_path{ this->cleanPath( QStandardPaths::locate( QStandardPaths::HomeLocation, "", QStandardPaths::LocateDirectory ) ) };
    #if defined( Q_OS_MACOS )
        const std::filesystem::path exec_path{ "/Applications" };
        const std::filesystem::path conf_path{ home_path + "/Lybrary/Preferences/LogDoctor" };
        const std::filesystem::path data_path{ home_path + "/Lybrary/Application Support/LogDoctor" };
    #elif defined( Q_OS_WINDOWS )
        const std::filesystem::path exec_path{ std::filesystem::path{home_path.substr(0,2) + "/Program Files"}.make_preferred() };
        const std::filesystem::path conf_path{ std::filesystem::path{home_path + "/AppData/Local/LogDoctor"}.make_preferred() };
        const std::filesystem::path data_path{ std::filesystem::path{home_path + "/AppData/Local/LogDoctor"}.make_preferred() };
    #elif defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
        const std::filesystem::path exec_path{ "/usr/bin" };
        const std::filesystem::path conf_path{ home_path + "/.config/LogDoctor" };
        const std::filesystem::path data_path{ "/usr/share/LogDoctor" };
    #else
        #error "System not supported"
    #endif

    // language
    QTranslator translator;
    std::string language{ "en" };
    void updateUiLanguage();

    // work related
    bool remove_config_file{ false };
    bool remove_databases{   false };
    // db related
    bool db_data_found{   false };
    bool db_hashes_found{ false };
    std::filesystem::path db_data_path;
    std::filesystem::path db_hashes_path;
    // resources
    const std::vector<std::string> resources{
        "licenses",
        "help" };
    // uninstall
    QScopedPointer<QTimer> waiter_timer;
    QScopedPointer<QTimer> uninstaller_timer;
    bool uninstalling;
    void startUninstalling();
    bool checkDatabases();
    #if !defined( Q_OS_MACOS )
        bool removeMenuEntry();
        bool removeIcon();
    #endif
    bool removeDatabases();
    bool removeConfigfile();
    bool removeAppdata();
    bool removeExecutable();
    bool removeSelf();

    // for the configs
    void readConfigs( const std::filesystem::path& file_path );
    void readFile( const std::filesystem::path& path, std::string& output );
    void splitrip( std::vector<std::string>& output, const std::string& input, std::string_view separator="\n" );
    void split( std::vector<std::string>& output, const std::string& input, std::string_view separator );
    std::string strip(  const std::string& target, std::string_view chars=" \n\v\r\t\b" );
    std::string lstrip( const std::string& target, std::string_view chars );
    std::string rstrip( const std::string& target, std::string_view chars );
    size_t count( std::string_view str, std::string_view flag );
};

#endif // MAINWINDOW_H
