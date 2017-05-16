#include "bing_image_search.hpp"

#include <QSize>
#include <QWebEnginePage>

bing_image_search::bing_image_search(QWebEnginePage &page, QObject *parent) :
    image_search(page, parent),
    color_option_(color_option::all),
    image_type_(image_type::all),
    layout_(layout::all),
    max_search_size_(0),
    people_(people::all),
    safe_search_(safe_search::moderate),
    suffix_({"jpg", "jpeg", "png"})
{

}

void bing_image_search::find_image_links(const QString &target, size_t max_search_size)
{
    max_search_size_ = max_search_size;
    get_web_page().load("https://www.bing.com/images/search?q=" + target);
}

void bing_image_search::set_color_filter(image_search::color_option option) noexcept
{
    color_option_ = option;
}

void bing_image_search::set_color_filter(const QColor &option) noexcept
{
    color_ = option;
}

void bing_image_search::set_layout_filter(image_search::layout option) noexcept
{
    layout_ = option;
}

void bing_image_search::set_maximum_size_filter(const QSize &size) noexcept
{
    max_size_ = size;
}

void bing_image_search::set_minimum_size_filter(const QSize &size) noexcept
{
    min_size_ = size;
}

void bing_image_search::set_people_filter(image_search::people option) noexcept
{
    people_ = option;
}

void bing_image_search::set_safe_search(image_search::safe_search option) noexcept
{
    safe_search_ = option;
}

void bing_image_search::set_image_type_filter(image_search::image_type option) noexcept
{
    image_type_ = option;
}

void bing_image_search::set_suffix_filter(const QStringList &type)
{
    suffix_ = type;
}

void bing_image_search::load_web_page_finished(bool ok)
{

}
