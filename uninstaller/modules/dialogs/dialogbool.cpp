
#include "dialogbool.h"
#include "ui_dialogbool.h"

#include <QIcon>
#include <QFont>
#include <QFontDatabase>


DialogBool::DialogBool( const QString& title, const QString& text, const QString& quest, const QString& err, QWidget *parent ) :
    QDialog(parent),
    ui(new Ui::DialogBool)
{
    ui->setupUi(this);

    // insert the given title and text
    this->ui->label_Title->setText( title );
    this->ui->label_Message->setText( text );

    // question, may be included in the text message
    if ( quest.size() == 0 ) {
        this->ui->label_Question->setVisible( false );
    } else {
        this->ui->label_Question->setText( quest );
    }

    // additional info, hide by default
    this->ui->text_Additional->setVisible( false );
    if ( err.size() == 0 ) {
        this->ui->button_ShowAdditional->setEnabled( false );
        this->ui->button_ShowAdditional->setVisible( false );
    } else {
        this->ui->text_Additional->setText( err );
        this->ui->text_Additional->setFont(
            QFont(
                QFontDatabase::applicationFontFamilies(
                        QFontDatabase::addApplicationFont(":/fonts/3270")).at(0),
                11 ));
    }

    // adjust the initial size
    this->adjustSize();
}

DialogBool::~DialogBool()
{
    delete ui;
}


void DialogBool::on_button_ShowAdditional_clicked()
{
    this->additional_shown = ! this->additional_shown;
    // set additional info visibility
    this->ui->text_Additional->setVisible( this->additional_shown );
    // set the icon
    QIcon icon;
    if ( this->additional_shown ) {
        icon = QIcon(":/icons/up");
        // resize
        this->initial_height = this->height();
        if ( this->additional_height > 0 ) {
            this->resize( this->width(), this->additional_height );
        } else {
            this->resize( this->width(), this->height()+100 );
        }
    } else {
        icon = QIcon(":/icons/down");
        this->additional_height = this->height();
        this->resize( this->width(), this->initial_height );
    }
    this->ui->button_ShowAdditional->setIcon( icon );
}


void DialogBool::on_button_NO_clicked()
{
    this->done( 0 );
}

void DialogBool::on_button_YES_clicked()
{
    this->done( 1 );
}
