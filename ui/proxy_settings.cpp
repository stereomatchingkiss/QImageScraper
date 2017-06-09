#include "proxy_settings.hpp"
#include "ui_proxy_settings.h"

#include <QsLog.h>

#include "proxy_delegate.hpp"

#include <QComboBox>
#include <QSettings>
#include <QSpinBox>

#include <set>

proxy_settings::proxy_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::proxy_settings)
{
    ui->setupUi(this);
    ui->tableWidgetPoxyTable->setItemDelegate(new proxy_delegate(this));
    ui->tableWidgetPoxyTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QSettings settings;
    if(settings.contains("proxy_settings/manual_proxy")){
        if(settings.value("proxy_settings/manual_proxy").toBool()){
            ui->radioButtonManualProxy->setChecked(true);
        }else{
            ui->radioButtonNoProxy->setChecked(true);
            ui->tableWidgetPoxyTable->setEnabled(false);
        }
    }else{
        settings.setValue("proxy_settings/manual_proxy", false);
        ui->radioButtonNoProxy->setChecked(true);
        ui->tableWidgetPoxyTable->setEnabled(false);
    }

    if(settings.contains("proxy_settings/no_proxy")){
        if(settings.value("proxy_settings/no_proxy").toBool()){
            ui->radioButtonNoProxy->setChecked(true);
            ui->tableWidgetPoxyTable->setEnabled(false);
        }
    }else{
        settings.setValue("proxy_settings/no_proxy", true);
        ui->radioButtonNoProxy->setChecked(true);
        ui->tableWidgetPoxyTable->setEnabled(false);
    }
}

proxy_settings::~proxy_settings()
{
    delete ui;
}

void proxy_settings::accept_settings()
{
    QSettings settings;
    settings.setValue("proxy_settings/no_proxy", ui->radioButtonNoProxy->isChecked());
    settings.setValue("proxy_settings/manual_proxy", ui->radioButtonManualProxy->isChecked());
}

std::vector<QNetworkProxy> proxy_settings::get_proxies() const
{
    std::vector<QNetworkProxy> proxies;
    for(int i = 0; i != ui->tableWidgetPoxyTable->rowCount(); ++i){
        auto const host = get_table_data(i, proxy_field::host).value<QString>();
        auto const port = get_table_data(i, proxy_field::port).value<qint16>();
        auto const type = get_table_data(i, proxy_field::type).value<int>();
        auto const user_name = get_table_data(i, proxy_field::user_name).value<QString>();
        auto const password = get_table_data(i, proxy_field::password).value<QString>();
        proxies.emplace_back(static_cast<QNetworkProxy::ProxyType>(type), host,
                             port, user_name, password);
    }

    return proxies;
}

void proxy_settings::reject_settings()
{
   QSettings settings;
   ui->radioButtonNoProxy->setChecked(settings.value("proxy_settings/no_proxy").toBool());
   ui->radioButtonManualProxy->setChecked(settings.value("proxy_settings/manual_proxy").toBool());
   ui->tableWidgetPoxyTable->setEnabled(ui->radioButtonManualProxy->isChecked());
}

void proxy_settings::on_radioButtonNoProxy_clicked()
{
    ui->tableWidgetPoxyTable->setEnabled(false);
}

void proxy_settings::on_radioButtonManualProxy_clicked()
{
    ui->tableWidgetPoxyTable->setEnabled(true);
}

void proxy_settings::on_pushButtonHelp_clicked()
{

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

QVariant proxy_settings::get_table_data(int row, proxy_field col) const
{
    return ui->tableWidgetPoxyTable->item(row, static_cast<int>(col))->data(Qt::DisplayRole);
}
