#include "image_search.hpp"
#include <qt_enhance/utility/qte_utility.hpp>

#include <QsLog.h>

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
    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &image_search::clipboard_data_changed);

}

void image_search::load(const QUrl &url)
{
    web_page_.load(url);
}

void image_search::get_web_view_image(std::function<void(QImage const&)> callback)
{    
    QLOG_INFO()<<__func__<<":trigger copy action";
    get_web_view_img_callback_ = callback;
    web_page_.triggerAction(QWebEnginePage::WebAction::Copy);
}

QWebEnginePage& image_search::get_web_page()
{
    return web_page_;
}

void image_search::clipboard_data_changed()
{
    QLOG_INFO()<<__func__<<":get image from clipboard";
    get_web_view_img_callback_(QGuiApplication::clipboard()->image(QClipboard::Clipboard));
}
