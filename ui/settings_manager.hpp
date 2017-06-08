#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QDialog>

namespace Ui {
class settings;
}

class general_settings;
class proxy_settings;

class QAbstractButton;

class settings_manager : public QDialog
{
    Q_OBJECT

public:
    explicit settings_manager(QWidget *parent = nullptr);
    ~settings_manager();

    general_settings const& get_general_settings() const noexcept;
    proxy_settings const& get_proxy_settings() const noexcept;

signals:
    void ok_clicked();

private slots:
    void on_settings_accepted();

    void on_settings_rejected();

private:
    general_settings& get_pri_general_settings() noexcept;
    proxy_settings& get_pri_proxy_settings() noexcept;

    Ui::settings *ui;    
};

#endif // SETTINGS_HPP
