#ifndef INFO_DIALOG_HPP
#define INFO_DIALOG_HPP

#include <QDialog>

namespace Ui {
class info_dialog;
}

class info_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit info_dialog(QWidget *parent = nullptr);
    ~info_dialog();

signals:
    void check_link_could_found();

private slots:
    void on_pushButtonQt_clicked();

    void on_pushButtonFatCow_clicked();

    void on_pushButtonAuthor_clicked();

    void on_pushButtonLinkCouldFound_clicked();

private:
    void create_text_browser(QString const &contents);    

    Ui::info_dialog *ui;    
};

#endif // INFO_DIALOG_HPP
