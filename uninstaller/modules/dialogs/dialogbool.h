#ifndef DIALOGBOOL_H
#define DIALOGBOOL_H

#include <QDialog>


namespace Ui {
    class DialogBool;
}

class DialogBool : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBool( const QString& title, const QString& text, const QString& quest="", const QString& err="", QWidget *parent=nullptr );
    ~DialogBool();

private slots:
    void on_button_NO_clicked();

    void on_button_YES_clicked();

    void on_button_ShowAdditional_clicked();

private:
    Ui::DialogBool *ui;

    bool additional_shown = false;

    int initial_height    = 0;
    int additional_height = 0;
};

#endif // DIALOGBOOL_H
