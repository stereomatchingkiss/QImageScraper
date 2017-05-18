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

void image_search::set_color_filter(image_search::color_option) noexcept
{

}

void image_search::set_color_filter(const QColor&) noexcept
{

}

void image_search::set_layout_filter(image_search::layout) noexcept
{

}

void image_search::set_maximum_size_filter(const QSize&) noexcept
{

}

void image_search::set_minimum_size_filter(const QSize&) noexcept
{

}

void image_search::set_people_filter(image_search::people) noexcept
{

}

void image_search::set_safe_search(image_search::safe_search) noexcept
{

}

void image_search::set_image_type_filter(image_search::image_type) noexcept
{

}

void image_search::set_suffix_filter(const QStringList&)
{

}

QWebEnginePage& image_search::get_web_page()
{
    return web_page_;
}
