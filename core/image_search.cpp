#include "image_search.hpp"
#include "global_constant.hpp"

#include <qt_enhance/utility/qte_utility.hpp>

#include <QsLog.h>

#include <QClipboard>
#include <QColor>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QSize>
#include <QTimer>
#include <QWebEnginePage>

image_search::image_search(QWebEnginePage &page, QObject *parent) :
    QObject(parent),
    reload_time_(0),
    timer_(new QTimer(parent)),
    web_page_(page)
{
    connect(&web_page_, &QWebEnginePage::loadFinished, this, &image_search::load_web_page_finished);
    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &image_search::clipboard_data_changed);
    connect(&web_page_, &QWebEnginePage::loadProgress, [this](int progress)
    {
        timer_->start(global_constant::network_reply_timeout());
        emit web_page_load_prgress(progress);
    });
    connect(timer_, &QTimer::timeout, [this]()
    {
        if(reload_time_ < 2){
            ++reload_time_;
            web_page_.load(web_page_.url());
            timer_->start(global_constant::network_reply_timeout());
        }else{
            get_web_view_img_callback_(QImage());
        }
    });
}

void image_search::load(const QUrl &url)
{
    reload_time_ = 0;
    web_page_.load(url);
    timer_->start(global_constant::network_reply_timeout());
}

void image_search::get_web_view_image(std::function<void(QImage)> callback)
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
    timer_->stop();
    get_web_view_img_callback_(QGuiApplication::clipboard()->image(QClipboard::Clipboard));
}
