#ifndef PROXY_SETTINGS_HPP
#define PROXY_SETTINGS_HPP

#include <QDialog>
#include <QNetworkProxy>

#include <vector>

namespace Ui {
class proxy_settings;
}

class QTableWidgetItem;

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

    enum class proxy_state
    {
        no_proxy,
        manual_proxy
    };

    explicit proxy_settings(QWidget *parent = 0);
    ~proxy_settings();

    void accept_settings();
    std::vector<QNetworkProxy> get_proxies() const;
    void reject_settings();
    proxy_state state() const;    

private slots:
    void on_radioButtonNoProxy_clicked();

    void on_radioButtonManualProxy_clicked();

    void on_pushButtonHelp_clicked();

    void on_pushButtonAddProxy_clicked();

    void on_pushButtonDeleteProxy_clicked();    

private:
    void add_proxy(QString const &type, QString const &host,
                   quint16 port, QString const &user_name,
                   QString const &password);
    QVariant get_table_data(int row, proxy_field col, int role = Qt::DisplayRole) const;
    void read_proxy_data();
    void init_settings();
    void write_proxy_data();

    Ui::proxy_settings *ui;    
};

#endif // PROXY_SETTINGS_HPP
