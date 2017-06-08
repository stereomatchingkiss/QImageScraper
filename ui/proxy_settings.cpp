#include "proxy_settings.hpp"
#include "ui_proxy_settings.h"

#include <QsLog.h>

#include "proxy_delegate.hpp"

#include <QComboBox>
#include <QSpinBox>

#include <set>

proxy_settings::proxy_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::proxy_settings)
{
    ui->setupUi(this);
    ui->tableWidgetPoxyTable->setItemDelegate(new proxy_delegate(this));
    ui->tableWidgetPoxyTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

proxy_settings::~proxy_settings()
{
    delete ui;
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
