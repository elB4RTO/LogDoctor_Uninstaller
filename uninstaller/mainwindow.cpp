
#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "modules/dialogs/dialogbool.h"

#include <fstream>

#include <QFontDatabase>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // load the font
    const QString font_family = QFontDatabase::applicationFontFamilies(
        QFontDatabase::addApplicationFont(":/fonts/Metropolis")).at(0);
    // initialize the fonts
    const QFont font = QFont(
        font_family,
        13 );
    // apply the fonts
    this->ui->menubar->setFont( font );
    this->ui->centralwidget->setFont( font );
}

MainWindow::~MainWindow()
{
    delete this->ui;
    delete this->waiter_timer;
    delete this->uninstaller_timer;
}

void MainWindow::on_button_Close_clicked()
{
    this->close();
}


///////////////
//// UTILS ////
///////////////
const std::string MainWindow::cleanPath( const QString& path )
{
    if ( path.endsWith('/') || path.endsWith('\\') ) {
        return path.toStdString().substr( 0, path.size()-1 );
    } else {
        return path.toStdString();
    }
}



/////////////////
//// CONFIGS ////
/////////////////
void MainWindow::readFile( std::filesystem::path& path, std::string& output )
{
    std::ifstream file;
    try {
        file = std::ifstream( path );
        if ( ! file.is_open() ) {
            throw std::ios_base::failure( "file is not open" );
        }
        if ( ! file.good() ) {
            throw std::ios_base::failure( "file is not good" );
        }
        // add bit exceptions
        file.exceptions( std::ifstream::failbit );
        file.exceptions( std::ios_base::badbit );
        // read the whole file
        output = std::string(
            (std::istreambuf_iterator<char>( file )),
            std::istreambuf_iterator<char>() );

    } catch (...) {
        if ( file.is_open() ) {
            file.close();
        }
        throw std::exception();
    }

    if ( file.is_open() ) {
        file.close();
    }
}

void MainWindow::splitrip( std::vector<std::string>& output, const std::string& input, const std::string& separator )
{
    std::vector<std::string> aux;
    std::string str = input;
    this->strip( str );
    this->split( aux, str, separator );
    for ( std::string str_ : aux ) {
        this->strip( str_ );
        if ( str_.size() > 0 ) {
            output.push_back( str_ );
        }
    }
    aux.clear();
}

void MainWindow::split( std::vector<std::string>& output, const std::string& input, const std::string& separator )
{
    std::string slice;
    size_t start=0, stop;
    const size_t sep_size = separator.size();
    if ( input.size() > 0 ) {
        while (true) {
            stop = input.find( separator, start );
            if ( stop == std::string::npos ) {
                slice = input.substr( start );
                if ( slice.size() > 0 ) {
                    output.push_back( slice );
                }
                break;
            } else {
                slice = input.substr( start, stop-start );
                if ( slice.size() > 0 ) {
                    output.push_back( slice );
                }
                start = stop+sep_size;
            }
        }
    }
}

void MainWindow::strip( std::string& target, const std::string& chars )
{
    this->lstrip( target, chars );
    if ( target.size() > 1 ) { // if just 1 char left, means that it's a good one, or it had been stripped out
        this->rstrip( target, chars );
    }
}
void MainWindow::lstrip( std::string& target, const std::string& chars )
{
    // left strip
    bool found;
    const int max = target.size()-1;
    int i = 0;
    while ( i <= max ) {
        found = false;
        char str_index = target.at( i );
        for ( const char& chr : chars ) {
            if ( str_index == chr ) {
                found = true;
                break;
            }
        }
        if ( ! found ) {
            break;
        }
        i++;
    }
    if ( i <= max ) {
        target = target.substr( i );
    } else {
        target = "";
    }
}
void MainWindow::rstrip( std::string& target, const std::string& chars )
{
    // right strip
    bool found;
    const int max = target.size()-1;
    int i = max;
    while ( i >= 0 ) {
        found = false;
        char str_index = target.at( i );
        for ( const char& chr : chars ) {
            if ( str_index == chr ) {
                found = true;
                break;
            }
        }
        if ( ! found ) {
            break;
        }
        i--;
    }
    if ( i >= 0 ) {
        target = target.substr( 0, max - (max - i) + 1 );
    } else {
        target = "";
    }
}

void MainWindow::readConfigs( std::filesystem::path& file_path )
{
    // read the whole file
    std::string content;
    std::vector<std::string> configs;
    this->readFile( file_path, content );
    if ( content.size() > 0 ) {
        this->splitrip( configs, content );
    }

    if ( configs.size() == 0 ) {
        // failed to read
        throw std::exception();
    } else {
        // parse the configs
        bool d_found = false,
             h_found = false;
        std::vector<std::string> aux;
        for ( const std::string& line : configs ) {
            if ( line.at(0) == '[' ) {
                // section descriptor
                continue;
            }
            aux.clear();
            this->splitrip( aux, line, "=" );
            if ( aux.size() < 2 ) {
                // nothing to do
                continue;
            }
            // if here, a value is present
            const std::string& var = aux.at( 0 ),
                               val = aux.at( 1 );

            if ( val.size() == 0 ) {
                // nothing to do, no value stored
                continue;
            }

            if ( var == "DatabaseDataPath" ) {
                std::string aux_ = std::filesystem::canonical( val ).string();
                this->rstrip( aux_, "/" );
                if ( aux_.size() > 0 ) {
                    d_found = true;
                    this->db_data_path = aux_ + "/collection.db";
                }

            } else if ( var == "DatabaseHashesPath" ) {
                std::string aux_ = std::filesystem::canonical( val ).string();
                this->rstrip( aux_, "/" );
                if ( aux_.size() > 0 ) {
                    h_found = true;
                    this->db_hashes_path = aux_ + "/hashes.db";
                }
            }

            if ( d_found && h_found ) {
                // done
                break;
            }
        }
    }
}



//////////////
//// MENU ////
/// //////////
// switch language
void MainWindow::menu_actionEnglish_triggered()
{
    this->ui->actionEnglish->setChecked(   true );
    this->ui->actionEspanol->setChecked(  false );
    this->ui->actionFrancais->setChecked( false );
    this->ui->actionItaliano->setChecked( false );
    this->language = "en";
    this->updateUiLanguage();
}
void MainWindow::menu_actionEspanol_triggered()
{
    this->ui->actionEnglish->setChecked(  false );
    this->ui->actionEspanol->setChecked(   true );
    this->ui->actionFrancais->setChecked( false );
    this->ui->actionItaliano->setChecked( false );
    this->language = "es";
    this->updateUiLanguage();
}
void MainWindow::menu_actionFrancais_triggered()
{
    this->ui->actionEnglish->setChecked(  false );
    this->ui->actionEspanol->setChecked(  false );
    this->ui->actionFrancais->setChecked(  true );
    this->ui->actionItaliano->setChecked( false );
    this->language = "fr";
    this->updateUiLanguage();
}
void MainWindow::menu_actionItaliano_triggered()
{

    this->ui->actionEnglish->setChecked(  false );
    this->ui->actionEspanol->setChecked(  false );
    this->ui->actionFrancais->setChecked( false );
    this->ui->actionItaliano->setChecked(  true );
    this->language = "it";
    this->updateUiLanguage();
}

//////////////////
//// LANGUAGE ////
//////////////////
void MainWindow::updateUiLanguage()
{
    // remove the old translator
    QCoreApplication::removeTranslator( &this->translator );
    if ( this->translator.load( QString(":/translations/%1").arg(QString::fromStdString( this->language )) ) ) {
        // apply the new translator
        QCoreApplication::installTranslator( &this->translator );
        this->ui->retranslateUi( this );
    }
}



/////////////////
//// ACTIONS ////
/////////////////
void MainWindow::on_checkBox_ConfigFile_toggled(bool checked)
{
    this->remove_config_file = checked;
}

void MainWindow::on_checkBox_Databases_toggled(bool checked)
{
    this->remove_databases = checked;
}


void MainWindow::on_button_Uninstall_clicked()
{
    DialogBool dialog = DialogBool(
            MainWindow::tr( "Uninstall LogDoctor" ),
            MainWindow::tr( "Continue?" ) );
    const bool response = dialog.exec();
    if ( response ) {
        this->startUninstalling();
    }
}



////////////////////////
//// UNINSTALLATION ////
////////////////////////
void MainWindow::startUninstalling()
{
    this->uninstalling = true;
    // checker
    delete this->waiter_timer;
    this->waiter_timer = new QTimer(this);
    connect(this->waiter_timer, SIGNAL(timeout()), this, SLOT(checkUninstallProgress()));
    // worker
    delete this->uninstaller_timer;
    this->uninstaller_timer = new QTimer(this);
    this->uninstaller_timer->setSingleShot( true );
    connect(this->uninstaller_timer, SIGNAL(timeout()), this, SLOT(Uninstall()));
    // run
    this->waiter_timer->start(250);
    this->uninstaller_timer->start(250);
}

void MainWindow::checkUninstallProgress()
{
    if ( ! this->uninstalling ) {
        this->waiter_timer->stop();
        this->ui->stacked_Main->setCurrentIndex( 2 );
    }
}

void MainWindow::Uninstall()
{
    bool ok = true;

    this->ui->progressBar_Uninstall->setValue( 0 );
    this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Checking databases ..." ) );
    // load the configs to get the databases paths
    ok = this->checkDatabases();

    #if !defined( Q_OS_MACOS )
        if ( ok ) { // mac .app contains it
            this->ui->progressBar_Uninstall->setValue( 10 );
            this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing the menu entry ..." ) );
            // remove the menu entry
            this->removeMenuEntry();

            if ( ok ) {
                this->ui->progressBar_Uninstall->setValue( 20 );
                this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing the icon ..." ) );
                // remove the icon
                this->removeIcon();
            }
        }
    #endif


    // remove data
    if ( ok ) {
        // remove the databases first, if required
        if ( ok && this->remove_databases ) {
            this->ui->progressBar_Uninstall->setValue( 35 );
            this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing databases ..." ) );
            this->removeDatabases();
        }
    }


    // databases eventually removed, can now delete configuration file
    if ( ok && this->remove_config_file ) {
        this->ui->progressBar_Uninstall->setValue( 50 );
        this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing configuration data ..." ) );
        this->removeConfigfile();
    }


    // now remove other data
    if ( ok ) {
        this->ui->progressBar_Uninstall->setValue( 65 );
        this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing application data ..." ) );
        // check the app data folder
        ok = this->removeAppdata();
    }


    // remove the executable
    if ( ok ) {
        this->ui->progressBar_Uninstall->setValue( 90 );
        this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing the executable ..." ) );
        // check the executable path
        ok = this->removeExecutable();
    }

    // remove the uninstaller
    #if defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
        if ( ok ) {
            this->ui->progressBar_Uninstall->setValue( 95 );
            // on linux/bsd check if the uninstaller has been removed already
            ok = this->removeSelf();
        }
    #endif


    // proocess finished
    if ( ok ) {
        this->ui->progressBar_Uninstall->setValue( 100 );
        this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Final steps ..." ) );
        // succesfully
        this->ui->label_Done_Status->setText( MainWindow::tr( "Successfully uninstalled" ) );
    } else {
        // with a failure
        this->ui->label_Done_Status->setText( MainWindow::tr( "Failed to uninstall" ) );
        this->ui->label_Done_Info->setText( MainWindow::tr( "An error occured while uninstalling LogDoctor" ) );
    }
    this->uninstalling = false;
}




///////////////////
//// FUNCTIONS ////
///////////////////
bool MainWindow::checkDatabases()
{
    bool ok = true;

    std::filesystem::path conf_file_path = this->conf_path / "logdoctor.conf";
    try {
        this->readConfigs( conf_file_path );
        // check the existence
        if ( this->db_data_path.string().size() > 0 ) {
            this->db_data_path = this->db_data_path.string() + "/collection.db";
            if ( std::filesystem::exists( this->db_data_path )
              && std::filesystem::is_regular_file( this->db_data_path ) ) {
                // db found
                this->db_data_found = true;
            }
        }
        if ( this->db_hashes_path.string().size() > 0 ) {
            this->db_hashes_path = this->db_hashes_path.string() + "/hashes.db";
            if ( std::filesystem::exists( this->db_hashes_path )
              && std::filesystem::is_regular_file( this->db_hashes_path ) ) {
                // db found
                this->db_hashes_found = true;
            }
        }
    } catch (...) {
        // something failed, doesn't matter what
        ;
    }

    this->ui->progressBar_Uninstall->setValue( 3 );
    if ( ! this->db_data_found ) {
        // paths not found in the configs (or configs file not found), check the default path
        this->db_data_path = this->data_path.string() + "/collection.db";
        if ( std::filesystem::exists( this->db_data_path )
          && std::filesystem::is_regular_file( this->db_data_path ) ) {
            // db found
            this->db_data_found = true;
        } else {
            // db not found
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Database not found" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "Unable to locate the logs database" ),
                    QString::fromStdString( this->db_data_path.string() ) ),
                MainWindow::tr( "Continue anyway?" ) );
            ok = dialog.exec();
        }
    }
    this->ui->progressBar_Uninstall->setValue( 6 );
    if ( ! this->db_hashes_found ) {
        // configs file not found, check the default path
        this->db_hashes_path = this->data_path.string() + "/hashes.db";
        if ( std::filesystem::exists( this->db_hashes_path )
          && std::filesystem::is_regular_file( this->db_hashes_path ) ) {
            // db found
            this->db_hashes_found = true;
        } else {
            // db not found
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Database not found" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "Unable to locate the hashes database" ),
                    QString::fromStdString( this->db_hashes_path.string() ) ),
                MainWindow::tr( "Continue anyway?" ) );
            ok = dialog.exec();
        }
    }
    return ok;
}


bool MainWindow::removeMenuEntry()
{
    bool ok = true;
    std::error_code err;

    bool remove = false;

    #if defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
        #if defined( Q_OS_LINUX )
            const std::filesystem::path p = "/usr/share/applications/LogDoctor.desktop";
        #elif defined( Q_OS_BSD4 )
            const std::filesystem::path p = "/usr/local/share/applications/LogDoctor.desktop";
        #endif
        if ( std::filesystem::exists( p ) ) {
            if ( std::filesystem::is_regular_file( p ) ) {
                // menu entry exists, plan remove it
                remove = true;
            } else {
                // not a file
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove the menu entry" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "The path doesn't point to a file" ),
                        QString::fromStdString( p.string() ) ),
                    MainWindow::tr( "Continue anyway?" ) );
                ok = dialog.exec();
            }
        }

    #elif defined( Q_OS_WINDOWS )
        const std::filesystem::path p = this->home_path.substr(0,2) + "/ProgramData/Microsoft/Windows/Start Menu/Programs/LogDoctor.exe";
        if ( std::filesystem::exists( p ) ) {
            if ( std::filesystem::is_symlink( p ) ) {
                // menu entry exists, plan remove it
                remove = true;
            } else {
                // not a symlink
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove the menu entry" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "The path doesn't point to a symlink" ),
                        QString::fromStdString( p.string() ) ),
                    MainWindow::tr( "Continue anyway?" ) );
                ok = dialog.exec();
            }
        }

    #else
        // macOS should not run this method
        throw( "LogDoctor: removeMenuEntry(): Unexpected OS" );
    #endif

    if ( remove ) {
        this->ui->progressBar_Uninstall->setValue( 15 );
        ok = std::filesystem::remove( p, err );
        if ( ! ok ) {
            // failed to remove
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Failed to remove the menu entry" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "An error occured while removing the entry" ),
                    QString::fromStdString( p.string() ) ),
                MainWindow::tr( "Continue anyway?" ),
                QString::fromStdString( err.message() ) );
            ok = dialog.exec();
        }
    }
    return ok;
}


bool MainWindow::removeIcon()
{
    bool ok = true;
    std::error_code err;

    #if defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
        const std::filesystem::path p = "/usr/share/LogDoctor/LogDoctor.svg";
    #elif defined( Q_OS_WINDOWS )
        const std::filesystem::path p = this->exec_path / "LogDoctor.svg";
    #else
        // macOS should not run this method
        throw( "LogDoctor: removeIcon(): Unexpected OS" );
    #endif

    if ( std::filesystem::exists( p ) ) {
        ok = std::filesystem::remove( p, err );
        if ( ! ok ) {
            // failed to remove
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Failed to remove the icon" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "An error occured while deleting the file" ),
                    QString::fromStdString( p.string() ) ),
                MainWindow::tr( "Continue anyway?" ),
                QString::fromStdString( err.message() ) );
            ok = dialog.exec();
        }
    }
    return ok;
}


bool MainWindow::removeDatabases()
{
    bool ok = true;
    std::error_code err;

    if ( this->db_data_found ) {
        this->ui->progressBar_Uninstall->setValue( 40 );
        ok = std::filesystem::remove( this->db_data_path, err );
        if ( !ok ) {
            // failed to remove the database
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Failed to remove database" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "An error occured while deleting the logs database" ),
                    QString::fromStdString( this->db_data_path.string() ) ),
                MainWindow::tr( "Continue anyway?" ),
                QString::fromStdString( err.message() ) );
            ok = dialog.exec();
        } else {
            // try removing the backups too
            std::string aux = this->db_data_path.string();
            aux = aux.substr( 0, aux.size()-14 );
            const std::filesystem::path path = aux+"/backups";
            if ( std::filesystem::exists( path )
              && std::filesystem::is_directory( path ) ) {
                // backups folder found
                std::ignore = std::filesystem::remove_all( path, err );
                ok = !std::filesystem::exists( path );
                if ( !ok ) {
                    // failed to remove backups
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Failed to remove database backups" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "An error occured while deleting the logs database's backups" ),
                            QString::fromStdString( path.string() ) ),
                        MainWindow::tr( "Continue anyway?" ),
                        QString::fromStdString( err.message() ) );
                    ok = dialog.exec();
                }
            }
        }
    }
    if ( ok && this->db_hashes_found ) {
        this->ui->progressBar_Uninstall->setValue( 45 );
        ok = std::filesystem::remove( this->db_hashes_path, err );
        if ( ! ok ) {
            // failed to remove
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Failed to remove database" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "An error occured while deleting the hashes database" ),
                    QString::fromStdString( this->db_hashes_path.string() ) ),
                MainWindow::tr( "Continue anyway?" ),
                QString::fromStdString( err.message() ) );
            ok = dialog.exec();
        }
    }
    return ok;
}


bool MainWindow::removeConfigfile()
{
    bool ok = true;
    std::error_code err;

    if ( ! std::filesystem::exists( this->conf_path ) ) {
        // conf dir not found
        DialogBool dialog = DialogBool(
            MainWindow::tr( "Failed to remove configuration data" ),
            QString("%1:\n%2").arg(
                MainWindow::tr( "The directory doesn't exist" ),
                QString::fromStdString( this->conf_path.string() ) ),
            MainWindow::tr( "Continue anyway?" ) );
        ok = dialog.exec();
    } else {
        // path exists, check if dir
        if ( ! std::filesystem::is_directory( this->conf_path ) ) {
            // conf dir not found
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Failed to remove configuration data" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "The path doesn't point to a directory" ),
                    QString::fromStdString( this->conf_path.string() ) ),
                MainWindow::tr( "Continue anyway?" ) );
            ok = dialog.exec();
        } else {
            this->ui->progressBar_Uninstall->setValue( 55 );
            // it's a directory, remove it
            bool delete_folder = false;
            if ( this->remove_databases ) {
                // no need to check for db presence, wipe it out
                delete_folder = true;
            } else {
                // choosed to keep databases, check the presence here
                bool db_found = false;
                {
                    const std::filesystem::path path = this->conf_path / "collection.db";
                    if ( this->db_data_found ) {
                        if ( this->db_data_path == path ) {
                            db_found = true;
                        }
                    } else {
                        if ( std::filesystem::exists( path )
                          && std::filesystem::is_regular_file( path ) ) {
                            // db found
                            db_found = true;
                        }
                    }
                }{
                    const std::filesystem::path path = this->conf_path / "hashes.db";
                    if ( this->db_data_found ) {
                        if ( this->db_data_path == path ) {
                            db_found = true;
                        }
                    } else {
                        if ( std::filesystem::exists( path )
                          && std::filesystem::is_regular_file( path ) ) {
                            // db found
                            db_found = true;
                        }
                    }
                }
                delete_folder = !db_found;
            }
            std::filesystem::path path;
            if ( delete_folder ) {
                path = this->conf_path.string();
                std::ignore = std::filesystem::remove_all( path, err );
                ok = !std::filesystem::exists( path );
            } else {
                path = this->conf_path / "logdoctor.conf";
                ok = std::filesystem::remove( path, err );
            }
            if ( ! ok ) {
                // failed to remove
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove configuration data" ),
                    QString("%1:\n%2").arg(
                        (delete_folder)
                            ? MainWindow::tr( "An error occured while deleting the folder" )
                            : MainWindow::tr( "An error occured while deleting the file" ),
                        QString::fromStdString( path.string() ) ),
                    MainWindow::tr( "Continue anyway?" ),
                    QString::fromStdString( err.message() ) );
                ok = dialog.exec();
            }
        }
    }
    return ok;
}


bool MainWindow::removeAppdata()
{
    bool ok = true;
    std::error_code err;

    bool remove = false;
    if ( ! std::filesystem::exists( this->data_path ) ) {
        // path does not exists
        DialogBool dialog = DialogBool(
            MainWindow::tr( "Failed to remove application's data" ),
            QString("%1:\n%2").arg(
                MainWindow::tr( "The directory doesn't exist" ),
                QString::fromStdString( this->data_path.string() ) ),
            MainWindow::tr( "Continue anyway?" ) );
        ok = dialog.exec();
    } else {
        // path exists
        if ( ! std::filesystem::is_directory( this->data_path ) ) {
            // not a directory
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Failed to remove application's data" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "The path doesn't point to a directory" ),
                    QString::fromStdString( this->data_path.string() ) ),
                MainWindow::tr( "Continue anyway?" ) );
            ok = dialog.exec();
        } else {
            // and it's a directory, keep going
            remove = true;
        }
    }

    if ( ok && remove ) {
        this->ui->progressBar_Uninstall->setValue( 70 );
        // remove data
        bool delete_folder = false;
        bool db_data_here = false;
        bool db_hashes_here = false;
        bool conf_file_here = false;
        if ( this->remove_databases ) {
            // doesn't matter if databases are here
            #if defined( Q_OS_WINDOWS )
                delete_folder = this->remove_config_file; // if choosed to remove the config file, there's no need to check for its presence in this folder, wipe it out
            #else
                delete_folder = true; // config file is in a different folder
            #endif
        } else {
            // choosed to keep databases, check the presence here
            bool found = false;
            {
                const std::filesystem::path path = this->data_path / "collection.db";
                if ( this->db_data_found ) {
                    if ( this->db_data_path == path ) {
                        found = true;
                        db_data_here = true;
                    }
                } else {
                    if ( std::filesystem::exists( path )
                      && std::filesystem::is_regular_file( path ) ) {
                        // db found
                        found = true;
                        db_data_here = true;
                    }
                }
            }
            {
                const std::filesystem::path path = this->data_path / "hashes.db";
                if ( this->db_data_found ) {
                    if ( this->db_data_path == path ) {
                        found = true;
                        db_hashes_here = true;
                    }
                } else {
                    if ( std::filesystem::exists( path )
                      && std::filesystem::is_regular_file( path ) ) {
                        // db found
                        found = true;
                        db_hashes_here = true;
                    }
                }
            }
            if ( !this->remove_config_file ) {
                // choosed to keep the config file, check for its presence here
                const std::filesystem::path path = this->data_path / "logdoctor.conf";
                if ( std::filesystem::exists( path )
                  && std::filesystem::is_regular_file( path ) ) {
                    // config file found
                    found = true;
                    conf_file_here = true;
                }
            }
            delete_folder = !found;
        }

        this->ui->progressBar_Uninstall->setValue( 75 );
        if ( delete_folder ) {
            // delete the whole folder at once
            std::ignore = std::filesystem::remove_all( this->data_path, err );
            ok = !std::filesystem::exists( this->data_path );
            if ( ! ok ) {
                // failed to remove
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove application's data" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "An error occured while deleting the folder" ),
                        QString::fromStdString( this->data_path.string() ) ),
                    MainWindow::tr( "Continue anyway?" ),
                    QString::fromStdString( err.message() ) );
                ok = dialog.exec();
            }

        } else {
            // delete LogDoctor's data only, but not the databases
            std::vector<std::filesystem::path> paths = {
                this->data_path.string() + "/help" };
            #if !defined( Q_OS_MACOS )
                // mac .app already contains it
                paths.push_back( this->data_path.string() + "/licenses" );
            #endif
            if ( this->remove_databases && db_data_here ) {
                paths.push_back( this->data_path.string() + "/collection.db" );
                paths.push_back( this->data_path.string() + "/backups" );
            }
            if ( this->remove_databases && db_hashes_here ) {
                paths.push_back( this->data_path.string() + "/hashes.db" );
            }
            if ( this->remove_config_file && conf_file_here ) {
                paths.push_back( this->data_path.string() + "/logdoctor.conf" );
            }
            for ( const auto& path : paths ) {
                if ( std::filesystem::exists( path )
                  && std::filesystem::is_directory( path ) ) {
                    // found, remove
                    ok = std::filesystem::remove_all( path, err );
                    if ( ! ok ) {
                        // failed to remove
                        DialogBool dialog = DialogBool(
                            MainWindow::tr( "Failed to remove application's data" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "An error occured while deleting the folder" ),
                                QString::fromStdString( path.string() ) ),
                            MainWindow::tr( "Continue anyway?" ),
                            QString::fromStdString( err.message() ) );
                        ok = dialog.exec();
                        if ( !ok ) {
                            break;
                        }
                    }
                }
            }
        }
    }
    return ok;
}


bool MainWindow::removeExecutable()
{
    bool ok = true;
    std::error_code err;

    if ( ! std::filesystem::exists( this->exec_path ) ) {
        // path does not exists
        DialogBool dialog = DialogBool(
            MainWindow::tr( "Failed to remove the executable" ),
            QString("%1:\n%2").arg(
                MainWindow::tr( "The executable's directory doesn't exist" ),
                QString::fromStdString( this->exec_path.string() ) ),
            MainWindow::tr( "Continue anyway?" ) );
        ok = dialog.exec();

    } else {
        // directory exists, check the executable file
        #if defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
            // just remove the file
            const std::filesystem::path path = this->exec_path / "logdoctor";
            if ( ! std::filesystem::exists( path ) ) {
                // executable does not exists
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove the executable" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "File not found" ),
                        QString::fromStdString( path.string() ) ),
                    MainWindow::tr( "Continue anyway?" ) );
                ok = dialog.exec();

            } else {
                // executable found, check it
                if ( ! std::filesystem::is_regular_file( path ) ) {
                    // not a file
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Failed to remove the executable" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "The path doesn't point to a file" ),
                            QString::fromStdString( this->exec_path.string() ) ),
                        MainWindow::tr( "Continue anyway?" ) );
                    ok = dialog.exec();

                } else {
                    // everything seems fine, remove the file
                    ok = std::filesystem::remove( path, err );
                    if ( ! ok ) {
                        // failed to remove
                        DialogBool dialog = DialogBool(
                            MainWindow::tr( "Failed to remove the executable" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "An error occured while deleting the file" ),
                                QString::fromStdString( this->exec_path.string() ) ),
                            MainWindow::tr( "Continue anyway?" ),
                            QString::fromStdString( err.message() ) );
                        ok = dialog.exec();
                    }
                }
            }

        #else
            // on windows/mac, remove the folder
            #if defined( Q_OS_WINDOWS )
                const std::filesystem::path path = this->exec_path / "LogDoctor";
            #elif defined( Q_OS_MACOS )
                const std::filesystem::path path = this->exec_path / "LogDoctor.app";
            #endif
            if ( ! std::filesystem::is_directory( path ) ) {
                // not a directory
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove the executable" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "The path doesn't point to a directory" ),
                        QString::fromStdString( path.string() ) ),
                    MainWindow::tr( "Continue anyway?" ) );
                ok = dialog.exec();

            } else {
                // everything seems fine, remove the file
                std::ignore = std::filesystem::remove_all( path, err );
                ok = !std::filesystem::exists( path );
                if ( ! ok ) {
                    // failed to remove
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Failed to remove the executable" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "An error occured while deleting the folder" ),
                            QString::fromStdString( path.string() ) ),
                        MainWindow::tr( "Continue anyway?" ),
                        QString::fromStdString( err.message() ) );
                    ok = dialog.exec();
                }
            }
        #endif
    }
    return ok;
}


bool MainWindow::removeSelf()
{
    bool ok = true;
    std::error_code err;

    if ( std::filesystem::exists( this->data_path ) ) { // may have been deleted already
        const std::filesystem::path path = this->data_path / "uninstall";
        if ( std::filesystem::exists( path ) ) {
            if ( std::filesystem::is_regular_file( path ) ) {
                // found, remove
                ok = std::filesystem::remove( path, err );
                if ( ! ok ) {
                    // failed to remove
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Failed to remove the uninstaller" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "An error occured while deleting the file" ),
                            QString::fromStdString( path.string() ) ),
                        MainWindow::tr( "Continue anyway?" ),
                        QString::fromStdString( err.message() ) );
                    ok = dialog.exec();
                }
            }
        }
    }
    return ok;
}
