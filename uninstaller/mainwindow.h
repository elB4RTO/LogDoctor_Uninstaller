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
    ~MainWindow();

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
    Ui::MainWindow *ui;

    // operating system
    const std::string cleanPath( const QString& path );
    // 1: linux, 2:windows, 3:mac
    const std::string home_path = this->cleanPath( QStandardPaths::locate( QStandardPaths::HomeLocation, "", QStandardPaths::LocateDirectory ) );
    #if defined( Q_OS_UNIX )
        // Unix-like systems: Linux, BSD and SysV
        const unsigned int OS = 1;
        const std::filesystem::path exec_path = "/usr/bin";
        const std::filesystem::path conf_path = home_path + "/.config/LogDoctor";
        const std::filesystem::path data_path = home_path + "/.local/share/LogDoctor";
    #elif defined( Q_OS_WIN )
        // Microsoft Windows systems
        const unsigned int OS = 2;
        const std::filesystem::path exec_path = home_path.substr(0,2) + "/ProgramFiles/LogDoctor";
        const std::filesystem::path conf_path = home_path + "/AppData/Local/LogDoctor";
        const std::filesystem::path data_path = home_path + "/AppData/Local/LogDoctor";
    #elif defined( Q_OS_DARWIN )
        // Darwin-based systems: macOS, macOS, iOS, watchOS and tvOS.
        const unsigned int OS = 3;
        const std::filesystem::path exec_path = "/Applications";
        const std::filesystem::path conf_path = home_path + "/Lybrary/Preferences/LogDoctor";
        const std::filesystem::path data_path = home_path + "/Lybrary/Application Support/LogDoctor";
    #else
        #error "System not supported"
    #endif

    // language
    QTranslator translator;
    std::string language = "en";
    void updateUiLanguage();

    // work related
    bool remove_config_file = false;
    bool remove_databases   = false;
    // uninstall
    QTimer* waiter_timer = new QTimer();
    QTimer* uninstaller_timer = new QTimer();
    bool uninstalling;
    void startUninstalling();

    // for the configs
    void readConfigs( std::filesystem::path& file_path, std::filesystem::path& db_data_path, std::filesystem::path& db_hashes_path );
    void readFile( std::filesystem::path& path, std::string& output );
    void splitrip( std::vector<std::string>& output, const std::string& input, const std::string& separator="\n" );
    void split( std::vector<std::string>& output, const std::string& input, const std::string& separator );
    void strip(  std::string& target, const std::string& chars=" \n\v\r\t\b" );
    void lstrip( std::string& target, const std::string& chars );
    void rstrip( std::string& target, const std::string& chars );
};

#endif // MAINWINDOW_H
