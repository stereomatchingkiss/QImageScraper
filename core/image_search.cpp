#include "image_search.hpp"

#include <QColor>
#include <QSize>
#include <QWebEnginePage>

image_search::image_search(QWebEnginePage &page, QObject *parent) :
    QObject(parent),
    web_page_(page)
{
    connect(&web_page_, &QWebEnginePage::loadFinished, this, &image_search::load_web_page_finished);    
}

QWebEnginePage& image_search::get_web_page()
{
    return web_page_;
}
