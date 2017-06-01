#include "image_search.hpp"
#include <qt_enhance/utility/qte_utility.hpp>

#include <QsLog.h>

#include <QClipboard>
#include <QColor>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QSize>
#include <QThread>
#include <QWebEnginePage>

image_search::image_search(QWebEnginePage &page, QObject *parent) :
    QObject(parent),
    web_page_(page)
{
    connect(&web_page_, &QWebEnginePage::loadFinished, this, &image_search::load_web_page_finished);
}

void image_search::load(const QUrl &url)
{
    web_page_.load(url);
}

bool image_search::save_web_view_image(QString const &save_at)
{
    QLOG_INFO()<<__func__<<":trigger copy action";
    web_page_.triggerAction(QWebEnginePage::WebAction::Copy);
    QLOG_INFO()<<__func__<<":access clipboard";
    QClipboard *clipboard = QGuiApplication::clipboard();    
    //Wait a moment before image can copy to clipboard
    //Should give save function a try after update to Qt5.9
    QThread::sleep(2);
    QLOG_INFO()<<__func__<<":get image from clipboard";
    QImage const img = clipboard->image(QClipboard::Clipboard);
    if(!img.isNull()){
        QLOG_INFO()<<__func__<<":image is not null";
        QString const url = web_page_.url().toString();        
        QString const file_name = qte::utils::unique_file_name(save_at, QFileInfo(url).fileName());
        QLOG_INFO()<<__func__<<":save image as:"<<file_name;
        return img.save(save_at + "/" + file_name);
    }else{
        QLOG_INFO()<<__func__<<"image is null";
        return false;
    }
}

QWebEnginePage& image_search::get_web_page()
{
    return web_page_;
}
