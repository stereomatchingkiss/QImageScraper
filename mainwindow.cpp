#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSizeGrip>

#include <qt_enhance/network/download_supervisor.hpp>

#include "core/bing_image_search.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    downloader_(new qte::net::download_supervisor(this)),
    img_search_(nullptr)
{
    ui->setupUi(this);
    ui->comboBoxSearchBy->addItem("Bing");
    on_comboBoxSearchBy_activated("Bing");

    ui->labelProgress->setVisible(false);
    ui->progressBar->setVisible(false);

    using namespace qte::net;

    connect(downloader_, &download_supervisor::download_finished, this, &MainWindow::download_finished);
    connect(downloader_, &download_supervisor::download_progress, this, &MainWindow::download_progress);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_comboBoxSearchBy_activated(const QString &arg1)
{
    if(img_search_){
        img_search_->deleteLater();
    }

    if(arg1 == "Bing"){
        ui->webView->load(QUrl("https://www.bing.com/?scope=images&nr=1&FORM=NOFORM"));
        img_search_ = new bing_image_search(*ui->webView->page(), this);
    }

    connect(img_search_, &bing_image_search::found_image_link, this, &MainWindow::found_img_link);
    connect(img_search_, &bing_image_search::parse_all_image_link, this, &MainWindow::download_image);
}

void MainWindow::found_img_link(const QString &big_img_link, const QString &small_img_link)
{
    img_links_.emplace_back(big_img_link, small_img_link);
}

void MainWindow::download_finished(size_t unique_id, QNetworkReply::NetworkError code, QByteArray data)
{
    qDebug()<<__func__<<":"<<unique_id<<":"<<code;
}

void MainWindow::download_image()
{
    if(!img_links_.empty()){
        //downloader_->append()
    }
}

void MainWindow::download_progress(size_t unique_id, qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug()<<__func__<<":"<<unique_id<<":"<<bytesReceived<<":"<<bytesTotal;
}

void MainWindow::on_actionGo_triggered()
{
    if(img_search_){
        ui->webView->page()->runJavaScript("function jscmd(){return document.getElementById(\"sb_form_q\").value} jscmd()",
                                           [this](QVariant const &contents)
        {
            setEnabled(false);
            img_search_->find_image_links(contents.toString(), 1000);
            qDebug()<<contents;
        });
    }
}
