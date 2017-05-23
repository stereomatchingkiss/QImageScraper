#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "core/utility.hpp"

#include <QDebug>
#include <QImage>
#include <QNetworkRequest>
#include <QSizeGrip>

#include "core/bing_image_search.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    default_max_size_(maximumSize()),
    downloader_(new qte::net::download_supervisor(this)),
    img_search_(nullptr)
{
    ui->setupUi(this);
    ui->comboBoxSearchBy->addItem("Bing");
    on_comboBoxSearchBy_activated("Bing");
    ui->actionDownload->setEnabled(false);
    ui->actionScroll->setEnabled(false);

    ui->labelProgress->setVisible(false);
    ui->progressBar->setVisible(false);

    ui->lineEditSaveAt->setText("/home/ramsus/Qt/img_pages");

    setMinimumSize(size());

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
        img_search_ = new bing_image_search(*ui->webView->page(), this);
        img_search_->go_to_first_page();
    }

    connect(img_search_, &image_search::found_image_link, this, &MainWindow::found_img_link);
    connect(img_search_, &image_search::go_to_first_page_done, this, &MainWindow::process_go_to_first_page);
    connect(img_search_, &image_search::go_to_second_page_done, this, &MainWindow::process_go_to_second_page);
}

void MainWindow::found_img_link(const QString &big_img_link, const QString &small_img_link)
{
    if(ui->comboBoxSearchBy->currentText() == "Bing"){
        QNetworkRequest const request = create_img_download_request(big_img_link,
                                                                    ui->comboBoxSearchBy->currentText());
        auto const unique_id = downloader_->append(request, ui->lineEditSaveAt->text());
        img_links_.emplace(unique_id, std::make_tuple(big_img_link, small_img_link, link_choice::big));
    }
}

void MainWindow::process_go_to_first_page()
{
   ui->actionScroll->setEnabled(false);
   ui->actionDownload->setEnabled(false);
}

void MainWindow::process_go_to_second_page()
{
   ui->actionScroll->setEnabled(true);
   ui->actionDownload->setEnabled(true);
}

void MainWindow::download_finished(std::shared_ptr<qte::net::download_supervisor::download_task> task)
{
    qDebug()<<__func__<<":"<<task->get_unique_id()<<":"<<task->get_network_error_code();
    qDebug()<<"save as:"<<task->get_save_as();
    auto it = img_links_.find(task->get_unique_id());
    if(task->get_network_error_code() == QNetworkReply::NoError && it != std::end(img_links_)){
        QImage img(task->get_save_as());
        if(!img.isNull()){
            qDebug()<<"can save image choice:"<<(int)std::get<2>(it->second);
        }else{
            qDebug()<<"cannot save image choice:"<<(int)std::get<2>(it->second);
            QFile::remove(task->get_save_as());
            if(std::get<0>(it->second) != std::get<1>(it->second) && std::get<2>(it->second) == link_choice::big){
                QNetworkRequest const request = create_img_download_request(std::get<1>(it->second),
                                                                            ui->comboBoxSearchBy->currentText());
                auto const uid = downloader_->append(request, ui->lineEditSaveAt->text(), true);
                img_links_.emplace(uid, std::make_tuple(std::get<0>(it->second),
                                                        std::get<1>(it->second), link_choice::small));
            }
        }
        img_links_.erase(it);
    }
}

void MainWindow::download_progress(size_t unique_id, qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug()<<__func__<<":"<<unique_id<<":"<<bytesReceived<<":"<<bytesTotal;
}

void MainWindow::on_actionScroll_triggered()
{
    img_search_->scroll_second_page(1000);
}

void MainWindow::on_actionDownload_triggered()
{
    ui->webView->page()->runJavaScript("function jscmd(){return document.getElementById(\"sb_form_q\").value} jscmd()",
                                       [this](QVariant const &contents)
    {
        setEnabled(false);
        img_search_->parse_imgs_link();
        setMaximumSize(size());
        qDebug()<<contents;
    });
}
