#include "image_search.hpp"
#include <qt_enhance/utility/qte_utility.hpp>

#include <QClipboard>
#include <QColor>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QSize>
#include <QWebEnginePage>

image_search::image_search(QWebEnginePage &page, QObject *parent) :
    QObject(parent),
    web_page_(page)
{
    connect(&web_page_, &QWebEnginePage::loadFinished, this, &image_search::load_web_page_finished);
}

bool image_search::save_web_view_image(QString const &save_at)
{
    web_page_.triggerAction(QWebEnginePage::WebAction::Copy);
    QClipboard *clipboard = QGuiApplication::clipboard();
    QImage const img = clipboard->image(QClipboard::Clipboard);
    if(!img.isNull()){
        QString const url = web_page_.url().toString();
        QString const file_name = qte::utils::unique_file_name(save_at, QFileInfo(url).fileName());
        return img.save(file_name);
    }else{
        return false;
    }
}

QWebEnginePage& image_search::get_web_page()
{
    return web_page_;
}
