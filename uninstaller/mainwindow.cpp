
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

void MainWindow::readConfigs( std::filesystem::path& file_path, std::filesystem::path& db_data_path, std::filesystem::path& db_hashes_path )
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
                    db_data_path = aux_ + "/collection.db";
                }

            } else if ( var == "DatabaseHashesPath" ) {
                std::string aux_ = std::filesystem::canonical( val ).string();
                this->rstrip( aux_, "/" );
                if ( aux_.size() > 0 ) {
                    h_found = true;
                    db_hashes_path = aux_ + "/hashes.db";
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
    std::error_code err;

    if ( this->OS != 3 ) { // mac .app contains it
        this->ui->progressBar_Uninstall->setValue( 0 );
        this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing the menu entry ..." ) );
        // remove the menu entry
        bool remove = false;
        std::filesystem::path p;
        switch ( this->OS ) {
            case 1:
                // unix
                p = this->home_path+"/.local/share/applications/LogDoctor.desktop";
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
                        const bool response = dialog.exec();
                        if ( ! response ) {
                            ok = false;
                        }
                    }
                }
                break;
            case 2:
                p = this->home_path.substr(0,2) + "/ProgramData/Microsoft/Windows/Start Menu/Programs/LogDoctor.exe";
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
                        const bool response = dialog.exec();
                        if ( ! response ) {
                            ok = false;
                        }
                    }
                }
                break;
            default:
                throw( "Unexpected OS: "[this->OS] );
        }
        if ( remove ) {
            this->ui->progressBar_Uninstall->setValue( 5 );
            ok = std::filesystem::remove( p, err );
            if ( ! ok ) {
                // failed to remove
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove the menu entry" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "An error occured while deleting the entry" ),
                        QString::fromStdString( p.string() ) ),
                    MainWindow::tr( "Continue anyway?" ),
                    QString::fromStdString( err.message() ) );
                const bool response = dialog.exec();
                if ( ! response ) {
                    ok = false;
                }
            }
        }
    }


    if ( ok && this->OS != 3 ) { // mac .app contains it
        this->ui->progressBar_Uninstall->setValue( 10 );
        this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing the icon ..." ) );
        // remove the icon
        bool remove = false;
        std::filesystem::path p;
        switch ( this->OS ) {
            case 1:
                // unix
                p = "/usr/share/icons/logdoctor.svg";
                if ( std::filesystem::exists( p ) ) { // !!! remove later
                    // menu entry exists, plan remove it
                    remove = true;
                }
                break;
            case 2:
                p = this->exec_path.string() + "/LogDoctor.svg";
                if ( std::filesystem::exists( p ) ) { // !!! remove later
                    // menu entry exists, plan remove it
                    remove = true;
                }
                break;
            default:
                throw( "Unexpected OS: "[this->OS] );
        }
        /*if ( std::filesystem::exists( p ) ) {*/ // !!! restore after completing mac part
        if ( remove ) {
            ok = std::filesystem::remove( p, err );
            if ( ! ok ) {
                // failed to remove
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove the icon" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "An error occured while deleting the entry" ),
                        QString::fromStdString( p.string() ) ),
                    MainWindow::tr( "Continue anyway?" ),
                    QString::fromStdString( err.message() ) );
                const bool response = dialog.exec();
                if ( ! response ) {
                    ok = false;
                }
            }
        }
    }


    // remove data
    if ( ok ) {
        this->ui->progressBar_Uninstall->setValue( 20 );
        this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Checking databases ..." ) );
        // load the configs to get the databases paths
        bool db_data_found = false;
        bool db_hashes_found = false;
        std::filesystem::path db_data_path;
        std::filesystem::path db_hashes_path;
        std::filesystem::path conf_file_path = this->conf_path.string() + "/logdoctor.conf";
        try {
            this->readConfigs( conf_file_path, db_data_path, db_hashes_path );
            // check the existence
            if ( db_data_path.string().size() > 0 ) {
                if ( std::filesystem::exists( db_data_path )
                  && std::filesystem::is_regular_file( db_data_path ) ) {
                    // db found
                    db_data_found = true;
                }
            }
            if ( db_hashes_path.string().size() > 0 ) {
                if ( std::filesystem::exists( db_hashes_path )
                  && std::filesystem::is_regular_file( db_hashes_path ) ) {
                    // db found
                    db_hashes_found = true;
                }
            }
        } catch (...) {
            // something failed, doesn't matter what
            ;
        }

        this->ui->progressBar_Uninstall->setValue( 25 );
        if ( ! db_data_found ) {
            // paths not found in the configs (or configs file not found), check the default path
            db_data_path = this->data_path.string() + "/collection.db";
            if ( std::filesystem::exists( db_data_path )
              && std::filesystem::is_regular_file( db_data_path ) ) {
                // db found
                db_data_found = true;
            } else {
                // db not found
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove database" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "Unable to locate the logs database" ),
                        QString::fromStdString( db_data_path.string() ) ),
                    MainWindow::tr( "Continue anyway?" ) );
                const bool response = dialog.exec();
                if ( ! response ) {
                    ok = false;
                }
            }
        }
        this->ui->progressBar_Uninstall->setValue( 30 );
        if ( ! db_hashes_found ) {
            // configs file not found, check the default path
            db_hashes_path = this->data_path.string() + "/hashes.db";
            if ( std::filesystem::exists( db_hashes_path )
              && std::filesystem::is_regular_file( db_hashes_path ) ) {
                // db found
                db_data_found = true;
            } else {
                // db not found
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove database" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "Unable to locate the hashes database" ),
                        QString::fromStdString( db_hashes_path.string() ) ),
                    MainWindow::tr( "Continue anyway?" ) );
                const bool response = dialog.exec();
                if ( ! response ) {
                    ok = false;
                }
            }
        }

        // databases paths has been established, in a way or another

        // remove the databases first, if required
        if ( ok && this->remove_databases ) {
            this->ui->progressBar_Uninstall->setValue( 35 );
            this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing databases ..." ) );
            if ( db_data_found ) {
                this->ui->progressBar_Uninstall->setValue( 40 );
                ok = std::filesystem::remove( db_data_path, err );
                if ( ! ok ) {
                    // failed to remove
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Failed to remove database" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "An error occured while deleting the logs database" ),
                            QString::fromStdString( db_data_path.string() ) ),
                        MainWindow::tr( "Continue anyway?" ),
                        QString::fromStdString( err.message() ) );
                    const bool response = dialog.exec();
                    if ( ! response ) {
                        ok = false;
                    }
                }
            }
            if ( ok && db_hashes_found ) {
                this->ui->progressBar_Uninstall->setValue( 45 );
                ok = std::filesystem::remove( db_hashes_path, err );
                if ( ! ok ) {
                    // failed to remove
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Failed to remove database" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "An error occured while deleting the hashes database" ),
                            QString::fromStdString( this->exec_path.string() ) ),
                        MainWindow::tr( "Continue anyway?" ),
                        QString::fromStdString( err.message() ) );
                    const bool response = dialog.exec();
                    if ( ! response ) {
                        ok = false;
                    }
                }
            }
        }
    }


    // databases eventually removed, can now delete configuration file
    if ( ok && this->remove_config_file ) {
        this->ui->progressBar_Uninstall->setValue( 50 );
        this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing configuration data ..." ) );
        if ( ! std::filesystem::exists( this->conf_path ) ) {
            // conf dir not found
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Failed to remove configuration data" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "The directory doesn't exist" ),
                    QString::fromStdString( this->conf_path.string() ) ),
                MainWindow::tr( "Continue anyway?" ) );
            const bool response = dialog.exec();
            if ( ! response ) {
                ok = false;
            }
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
                const bool response = dialog.exec();
                if ( ! response ) {
                    ok = false;
                }
            } else {
                this->ui->progressBar_Uninstall->setValue( 55 );
                // it's a directory, remove it
                std::ignore = std::filesystem::remove_all( this->conf_path, err );
                ok = !std::filesystem::exists( this->conf_path );
                if ( ! ok ) {
                    // failed to remove
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Failed to remove configuration data" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "An error occured while deleting the folder" ),
                            QString::fromStdString( this->conf_path.string() ) ),
                        MainWindow::tr( "Continue anyway?" ),
                        QString::fromStdString( err.message() ) );
                    const bool response = dialog.exec();
                    if ( ! response ) {
                        ok = false;
                    }
                }
            }
        }
    }


    // now remove other data
    if ( ok ) {
        this->ui->progressBar_Uninstall->setValue( 65 );
        this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing application data ..." ) );
        bool remove = false;
        // check the app data folder
        if ( ! std::filesystem::exists( this->data_path ) ) {
            // path does not exists
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Failed to remove application's data" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "The directory doesn't exist" ),
                    QString::fromStdString( this->data_path.string() ) ),
                MainWindow::tr( "Continue anyway?" ) );
            const bool response = dialog.exec();
            if ( ! response ) {
                ok = false;
            }
        } else {
            // path exists
            if ( ! std::filesystem::is_directory( this->data_path ) ) {
                // not a file
                DialogBool dialog = DialogBool(
                    MainWindow::tr( "Failed to remove application's data" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "The path doesn't point to a directory" ),
                        QString::fromStdString( this->data_path.string() ) ),
                    MainWindow::tr( "Continue anyway?" ) );
                const bool response = dialog.exec();
                if ( ! response ) {
                    ok = false;
                }
            } else {
                // and it's a directory, keep going
                remove = true;
            }
        }

        if ( ok && remove ) {
            this->ui->progressBar_Uninstall->setValue( 70 );
            // remove data
            bool delete_folder = false;
            if ( this->remove_databases ) {
                // no need to check for db presence, wipe it out
                delete_folder = true;
            } else {
                // choosed to keep databases, check the presence here
                bool db_found = false;
                for ( const auto& name : std::vector<std::string>({"/collection.db","/hashes.db"}) ) {
                    const std::filesystem::path path = this->data_path.string() + name;
                    if ( std::filesystem::exists( path )
                      && std::filesystem::is_regular_file( path ) ) {
                        // db found
                        db_found = true;
                        break;
                    }
                }
                delete_folder = !db_found;
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
                    const bool response = dialog.exec();
                    if ( ! response ) {
                        ok = false;
                    }
                }

            } else {
                // delete LogDoctor's data only, but not the databases
                std::vector<std::filesystem::path> paths = {
                    this->data_path.string() + "/help" };
                if ( this->OS != 3 ) { // mac .app already contains it
                    paths.push_back( this->data_path.string() + "/licenses" );
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
                                    MainWindow::tr( "An error occured while deleting the entry" ),
                                    QString::fromStdString( path.string() ) ),
                                MainWindow::tr( "Continue anyway?" ),
                                QString::fromStdString( err.message() ) );
                            const bool response = dialog.exec();
                            if ( ! response ) {
                                ok = false;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }


    // remove the executable
    if ( ok ) {
        this->ui->progressBar_Uninstall->setValue( 90 );
        this->ui->label_Uninstall_Info->setText( MainWindow::tr( "Removing the executable ..." ) );
        // check the executable path
        if ( ! std::filesystem::exists( this->exec_path ) ) {
            // path does not exists
            DialogBool dialog = DialogBool(
                MainWindow::tr( "Failed to remove the executable" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "The executable's directory doesn't exist" ),
                    QString::fromStdString( this->exec_path.string() ) ),
                MainWindow::tr( "Continue anyway?" ) );
            const bool response = dialog.exec();
            if ( ! response ) {
                ok = false;
            }

        } else {
            // directory exists, check the executable file
            if ( this->OS == 1 ) {
                // on linux/bsd, just remove the file
                const std::filesystem::path path = this->exec_path.string() + "/logdoctor";
                if ( ! std::filesystem::exists( path ) ) {
                    // executable does not exists
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Failed to remove the executable" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "File not found" ),
                            QString::fromStdString( path.string() ) ),
                        MainWindow::tr( "Continue anyway?" ) );
                    const bool response = dialog.exec();
                    if ( ! response ) {
                        ok = false;
                    }

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
                        const bool response = dialog.exec();
                        if ( ! response ) {
                            ok = false;
                        }

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
                            const bool response = dialog.exec();
                            if ( ! response ) {
                                ok = false;
                            }
                        }
                    }
                }

            } else {
                // on windows/mac, remove the whole folder
                std::filesystem::path path;
                switch ( this->OS ) {
                    case 2:
                        path = this->exec_path.string() + "/LogDoctor.app";
                        break;
                    case 3:
                        path = this->exec_path.string() + "/LogDoctor";
                        break;
                    default:
                        throw( "Unexpected OS: "[this->OS] );
                }
                if ( ! std::filesystem::is_directory( path ) ) {
                    // not a file
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Failed to remove the executable" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "The path doesn't point to a directory" ),
                            QString::fromStdString( path.string() ) ),
                        MainWindow::tr( "Continue anyway?" ) );
                    const bool response = dialog.exec();
                    if ( ! response ) {
                        ok = false;
                    }

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
                        const bool response = dialog.exec();
                        if ( ! response ) {
                            ok = false;
                        }
                    }
                }
            }
        }
    }

    if ( ok && this->OS == 1 ) {
        this->ui->progressBar_Uninstall->setValue( 95 );
        // on linux/bsd check if the uninstaller has been removed already
        const std::filesystem::path path = this->data_path.string() + "/uninstall";
        if ( std::filesystem::exists( path ) ) {
            if ( std::filesystem::is_regular_file( path ) ) {
                // found, remove
                ok = std::filesystem::remove( path, err );
                if ( ! ok ) {
                    // failed to remove
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Failed to remove the uninstaller" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "An error occured while deleting the entry" ),
                            QString::fromStdString( path.string() ) ),
                        MainWindow::tr( "Continue anyway?" ),
                        QString::fromStdString( err.message() ) );
                    const bool response = dialog.exec();
                    if ( ! response ) {
                        ok = false;
                    }
                }
            }
        }
    }


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



void MainWindow::on_button_Close_clicked()
{
    this->close();
}
