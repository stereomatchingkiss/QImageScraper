#include "proxy_settings.hpp"
#include "ui_proxy_settings.h"

#include <QsLog.h>

#include "proxy_delegate.hpp"
#include "proxy_password_delegate.hpp"

#include <QComboBox>
#include <QDataStream>
#include <QDesktopServices>
#include <QFile>
#include <QLineEdit>
#include <QMessageBox>
#include <QSaveFile>
#include <QSettings>
#include <QSpinBox>
#include <QStandardPaths>

#include <set>
#include <map>

namespace{

struct settings_key
{
    static QString const no_proxy;
    static QString const manual_proxy;
};

QString const settings_key::no_proxy("proxy_settings/no_proxy");
QString const settings_key::manual_proxy = "proxy_settings/manual_proxy";

QString const proxy_setting_path(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                                 "/qimage_scraper_proxy_settings.dat");

}

proxy_settings::proxy_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::proxy_settings)
{
    ui->setupUi(this);
    ui->tableWidgetPoxyTable->setItemDelegateForColumn(static_cast<int>(proxy_field::user_name), new proxy_delegate(this));
    ui->tableWidgetPoxyTable->setItemDelegateForColumn(static_cast<int>(proxy_field::password), new proxy_password_delegate(this));
    ui->tableWidgetPoxyTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    init_settings();
}

proxy_settings::~proxy_settings()
{
    delete ui;
}

void proxy_settings::accept_settings()
{
    QSettings settings;
    settings.setValue(settings_key::no_proxy, ui->radioButtonNoProxy->isChecked());
    settings.setValue(settings_key::manual_proxy, ui->radioButtonManualProxy->isChecked());
    write_proxy_data();
}

std::vector<QNetworkProxy> proxy_settings::get_proxies() const
{
    std::vector<QNetworkProxy> proxies;
    std::map<QString, QNetworkProxy::ProxyType> mapper
    {
        {"DefaultProxy", QNetworkProxy::DefaultProxy},
        {"Socks5Proxy", QNetworkProxy::Socks5Proxy},
        {"HttpProxy", QNetworkProxy::HttpProxy},
        {"HttpCachingProxy", QNetworkProxy::HttpCachingProxy},
        {"FtpCachingProxy", QNetworkProxy::FtpCachingProxy}
    };
    for(int i = 0; i != ui->tableWidgetPoxyTable->rowCount(); ++i){
        auto *combo_box = qobject_cast<QComboBox*>(ui->tableWidgetPoxyTable->cellWidget(i, static_cast<int>(proxy_field::type)));
        auto *spin_box = qobject_cast<QSpinBox*>(ui->tableWidgetPoxyTable->cellWidget(i, static_cast<int>(proxy_field::port)));
        auto const host = get_table_data(i, proxy_field::host).value<QString>();
        auto const user_name = get_table_data(i, proxy_field::user_name).value<QString>();
        auto const password = get_table_data(i, proxy_field::password).value<QString>();
        proxies.emplace_back(mapper[combo_box->currentText()], host,
                static_cast<quint16>(spin_box->value()),
                user_name, password);
    }

    return proxies;
}

proxy_settings::proxy_state proxy_settings::state() const
{
    if(ui->radioButtonNoProxy->isChecked()){
        return proxy_state::no_proxy;
    }else{
        return proxy_state::manual_proxy;
    }
}

void proxy_settings::init_settings()
{
    QSettings settings;
    if(settings.contains(settings_key::manual_proxy)){
        if(settings.value(settings_key::manual_proxy).toBool()){
            ui->radioButtonManualProxy->setChecked(true);
            read_proxy_data();
            on_radioButtonManualProxy_clicked();
        }
    }else{
        settings.setValue(settings_key::manual_proxy, false);
    }

    if(settings.contains(settings_key::no_proxy)){
        if(settings.value(settings_key::no_proxy).toBool()){
            ui->radioButtonNoProxy->setChecked(true);
            on_radioButtonNoProxy_clicked();
        }
    }else{
        settings.setValue(settings_key::no_proxy, true);
        ui->radioButtonNoProxy->setChecked(true);
        on_radioButtonNoProxy_clicked();
    }
}

void proxy_settings::reject_settings()
{
    QSettings settings;
    ui->radioButtonNoProxy->setChecked(settings.value(settings_key::no_proxy).toBool());
    ui->radioButtonManualProxy->setChecked(settings.value(settings_key::manual_proxy).toBool());
    ui->groupBoxManualProxy->setVisible(ui->radioButtonManualProxy->isChecked());
    read_proxy_data();
}

void proxy_settings::on_radioButtonNoProxy_clicked()
{
    ui->groupBoxManualProxy->setVisible(false);
}

void proxy_settings::on_radioButtonManualProxy_clicked()
{    
    ui->groupBoxManualProxy->setVisible(true);
}

void proxy_settings::on_pushButtonHelp_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/stereomatchingkiss/QImageScraper/blob/master/proxy_tutorial.md"));
}

void proxy_settings::on_pushButtonAddProxy_clicked()
{
    int const row = ui->tableWidgetPoxyTable->rowCount();
    ui->tableWidgetPoxyTable->insertRow(row);

    QComboBox *box = new QComboBox;
    ui->tableWidgetPoxyTable->setCellWidget(row, static_cast<int>(proxy_field::type), box);
    box->addItems({tr("DefaultProxy"), tr("Socks5Proxy"), tr("HttpProxy"),
                   tr("HttpCachingProxy"), tr("FtpCachingProxy")});

    QSpinBox *spin_box = new QSpinBox;
    ui->tableWidgetPoxyTable->setCellWidget(row, static_cast<int>(proxy_field::port), spin_box);
    spin_box->setRange(0, 65535);
}

void proxy_settings::on_pushButtonDeleteProxy_clicked()
{
    auto const list = ui->tableWidgetPoxyTable->selectionModel()->selectedIndexes();
    std::set<int> rows;
    for(auto const &index : list){
        rows.insert(index.row());
    }
    QLOG_INFO()<<__func__<<": selected item size:"<<list.count();
    int offset = 0;
    for(auto const row : rows){
        ui->tableWidgetPoxyTable->removeRow(row - offset);
        ++offset;
    }
}

void proxy_settings::add_proxy(const QString &type, const QString &host, quint16 port,
                               const QString &user_name, const QString &password)
{
    on_pushButtonAddProxy_clicked();

    auto const row = ui->tableWidgetPoxyTable->rowCount() - 1;

    auto *combo_box = qobject_cast<QComboBox*>(ui->tableWidgetPoxyTable->cellWidget(row, static_cast<int>(proxy_field::type)));
    combo_box->setCurrentText(type);

    auto *spin_box = qobject_cast<QSpinBox*>(ui->tableWidgetPoxyTable->cellWidget(row, static_cast<int>(proxy_field::port)));
    spin_box->setValue(port);

    auto *model = ui->tableWidgetPoxyTable->model();
    model->setData(model->index(row, static_cast<int>(proxy_field::host)), host, Qt::DisplayRole);
    model->setData(model->index(row, static_cast<int>(proxy_field::user_name)), user_name, Qt::DisplayRole);
    model->setData(model->index(row, static_cast<int>(proxy_field::password)), "****", Qt::DisplayRole);
    model->setData(model->index(row, static_cast<int>(proxy_field::password)), password, Qt::UserRole + 1);
}

QVariant proxy_settings::get_table_data(int row, proxy_field col, int role) const
{   
    auto const index = ui->tableWidgetPoxyTable->model()->index(row, static_cast<int>(col));
    return ui->tableWidgetPoxyTable->model()->data(index, role);
}

void proxy_settings::read_proxy_data()
{
    if(ui->radioButtonManualProxy->isChecked()){
        QFile file(proxy_setting_path);
        if(file.open(QIODevice::ReadOnly)){
            ui->tableWidgetPoxyTable->model()->removeRows(0, ui->tableWidgetPoxyTable->rowCount());
            QDataStream stream(&file);
            while(!stream.atEnd()){
                QString type;
                QString host;
                quint16 port;
                QString user_name;
                QString password;
                stream>>type>>host>>port>>user_name>>password;
                add_proxy(type, host, port, user_name, password);
            }
        }else{
            if(QFile::exists("proxy_settgins.dat")){
                QMessageBox::warning(nullptr, tr("QImageScraper"), tr("Cannot read proxy settings"));
            }
        }
    }
}

void proxy_settings::write_proxy_data()
{
    QSaveFile file(proxy_setting_path);
    auto const cannot_write = []()
    {
        QMessageBox::warning(nullptr, tr("Error"), tr("Cannot write proxy data into file, your settings "
                                                      "of the proxies cannot be saved"));
    };

    if(!file.open(QIODevice::WriteOnly)){
        cannot_write();
    }else{
        QDataStream stream(&file);
        for(int i = 0; i != ui->tableWidgetPoxyTable->rowCount(); ++i){
            auto *combo_box = qobject_cast<QComboBox*>(ui->tableWidgetPoxyTable->cellWidget(i, static_cast<int>(proxy_field::type)));
            if(!combo_box){
                QLOG_INFO()<<__func__<<":combo box is empty";
            }
            QString const type = combo_box->currentText();
            QString const host = get_table_data(i, proxy_field::host).toString();
            auto *spin_box = qobject_cast<QSpinBox*>(ui->tableWidgetPoxyTable->cellWidget(i, static_cast<int>(proxy_field::port)));
            if(!spin_box){
                QLOG_INFO()<<__func__<<":spinbox box is empty";
            }
            quint16 const port = static_cast<quint16>(spin_box->value());
            QString const user_name = get_table_data(i, proxy_field::user_name).toString();
            QString const password = get_table_data(i, proxy_field::password, Qt::UserRole + 1).toString();
            QLOG_INFO()<<__func__<<":password of user role:"<<password;
            stream<<type<<host<<port<<user_name<<password;
        }

        if(!file.commit()){
            cannot_write();
        }
    }
}
