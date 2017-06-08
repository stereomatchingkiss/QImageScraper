#ifndef PROXY_SETTINGS_HPP
#define PROXY_SETTINGS_HPP

#include <QDialog>

namespace Ui {
class proxy_settings;
}

class proxy_settings : public QDialog
{
    Q_OBJECT

public:
    enum class proxy_field
    {
        host,
        port,
        user_name,
        password,
        type
    };

    explicit proxy_settings(QWidget *parent = 0);
    ~proxy_settings();

private slots:
    void on_radioButtonNoProxy_clicked();

    void on_radioButtonManualProxy_clicked();

    void on_pushButtonHelp_clicked();

    void on_pushButtonAddProxy_clicked();

    void on_pushButtonDeleteProxy_clicked();

private:
    Ui::proxy_settings *ui;
};

#endif // PROXY_SETTINGS_HPP
