#include "image_search.hpp"
#include "global_constant.hpp"

#include <qt_enhance/utility/qte_utility.hpp>

#include <QsLog.h>

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

QWebEnginePage& image_search::get_web_page()
{
    return web_page_;
}
