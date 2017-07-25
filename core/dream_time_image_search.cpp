#include "dream_time_image_search.hpp"

#include <QsLog.h>

#include <QSize>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>
#include <QWebEnginePage>

namespace{

constexpr int show_more_duration = 2000;

}

dream_time_image_search::dream_time_image_search(QWebEnginePage &page, QObject *parent) :
    image_search{page, parent},
    max_search_size_{0},
    state_{state::to_search_page},
    stop_show_more_image_{false}
{
}

void dream_time_image_search::get_search_image_size(std::function<void (size_t)> callback)
{
    callback(big_img_links_.size());
}

void dream_time_image_search::go_to_search_page()
{
    state_ = state::to_search_page;
    QLOG_INFO()<<__func__<<": before load";
    get_web_page().load(QUrl("https://www.dreamstime.com/"));
    QLOG_INFO()<<__func__<<": after load";
}

void dream_time_image_search::load(const QUrl &url)
{
    state_ = state::load_url;
    image_search::load(url);
}

void dream_time_image_search::reload()
{
    state_ = state::reload_url;
    get_web_page().load(get_web_page().url());
}

void dream_time_image_search::show_more_images(size_t max_search_size)
{
    big_img_links_.clear();
    small_img_links_.clear();
    max_search_size_ = max_search_size;
    state_ = state::show_more_images;
    //we need to setup timer because web view may not able to update in time,
    //this may cause the page stop loading next page too early
    stop_show_more_image_ = false;
    QTimer::singleShot(show_more_duration, [this](){show_more_page();});
}

void dream_time_image_search::stop_show_more_images()
{
    stop_show_more_image_ = true;
}

size_t dream_time_image_search::get_current_page_num()
{    
    QRegularExpression const reg("&pg=(\\d+)|photos-images/p(\\d+)/");
    auto const match = reg.match(get_web_page().url().toString());
    if(match.hasMatch()){
        if(!match.captured(1).isEmpty()){
            return match.captured(1).toInt();
        }else{
            return match.captured(2).toInt();
        }
    }

    return 1;
}

void dream_time_image_search::load_web_page_finished(bool ok)
{
    QLOG_INFO()<<"load web page finished:"<<ok<<", url:"<<get_web_page().url().toString();
    QLOG_INFO()<<"default state";
    bool const is_gallery_page = get_web_page().url().toString().contains("https://www.dreamstime.com/photos-images") ||
            get_web_page().url().toString().contains("https://www.dreamstime.com/search.php");
    if(state_ != state::show_more_images && is_gallery_page){
        QLOG_INFO()<<__func__<<":go to gallery page done";
        state_ = state::to_gallery_page;
        emit go_to_gallery_page_done();
        return;
    }
    if(ok){
        switch(state_){
        case state::load_url:{
            QLOG_INFO()<<"state load_url";
            emit load_url_done();
            break;
        }
        case state::reload_url:{
            QLOG_INFO()<<"state reload_url";
            emit reload_url_done();
            break;
        }
        case state::to_search_page:{
            QLOG_INFO()<<"state to_search_page";
            emit go_to_search_page_done();
            break;
        }
        case state::to_gallery_page:{
            QLOG_INFO()<<"state to_gallery_page";
            emit go_to_gallery_page_done();
            break;
        }
        case state::show_more_images:{
            QLOG_INFO()<<"state show_more_images";
            QTimer::singleShot(show_more_duration, [this](){show_more_page();});
            break;
        }
        case state::get_img_link_from_gallery_page:{
            QLOG_INFO()<<"state get_img_link_from_gallery_page";
            return;
        }
        default:
            break;
        }
    }else{
        emit search_error(image_search_error::error::load_page_error);
    }
}

void dream_time_image_search::parse_img_link(std::function<void(QString const&)> callback)
{
    get_web_page().toHtml([this, callback](QString const &contents)
    {
        QRegularExpression const reg("(https://static.dreamstime.com/t[^\"]*)");
        auto iter = reg.globalMatch(contents);
        while(iter.hasNext()){
            QRegularExpressionMatch const match = iter.next();
            small_img_links_.push_back(match.captured(1));
            big_img_links_.push_back(match.captured(1).replace("https://static.dreamstime.com/t",
                                                               "https://static.dreamstime.com/z"));
        }

        callback(contents);
    });
}

void dream_time_image_search::get_imgs_link_from_gallery_page(std::function<void(const QStringList &,
                                                                                 const QStringList &)> callback)
{
    state_ = state::get_img_link_from_gallery_page;
    if(big_img_links_.empty()){
        parse_img_link([this, callback](QString const &)
        {
            callback(big_img_links_, small_img_links_);
        });
    }else{
        big_img_links_.removeDuplicates();
        small_img_links_.removeDuplicates();
        callback(big_img_links_, small_img_links_);
    }
}

void dream_time_image_search::get_search_target(std::function<void (const QString &)> callback)
{
    get_web_page().runJavaScript("function get_target(){"
                                 "  return document.getElementById(\"srh_field\").value"
                                 "} get_target()", [callback](QVariant const &value)
    {
        callback(value.toString());
    });
}

void dream_time_image_search::go_to_gallery_page(const QString &target)
{
    state_ = state::to_gallery_page;
    if(!target.isEmpty()){
        QString const link = QString("https://www.dreamstime.com/photos-images/%1.html").arg(target);
        get_web_page().load(link);
    }
}

void dream_time_image_search::to_next_page(size_t max_page)
{
    auto const cur_page = get_current_page_num();
    if(cur_page < max_page){
        QLOG_INFO()<<__func__<<":found Next Page:"<<cur_page<<","<<max_page;
        get_web_page().runJavaScript("function get_next_page(){"
                                     "  return document.getElementsByClassName(\"dt-next\")[0].childNodes[0].href;"
                                     "} get_next_page()", [this](QVariant const &val)
            {
                QLOG_INFO()<<"dream line next page url:"<<val;
                get_web_page().load(val.toString());
            });
    }else{
        QLOG_INFO()<<"Reach the end of page";
        emit show_more_images_done();
    }
}

void dream_time_image_search::show_more_page()
{
    if(state_ != state::show_more_images){
        return;
    }

    if(stop_show_more_image_){
        stop_show_more_image_ = false;
        emit show_more_images_done();
        return;
    }

    if(static_cast<size_t>(big_img_links_.size()) >= max_search_size_){
        emit show_more_images_done();
        return;
    }

    parse_img_link([this](QString const &contents)
    {
        QRegularExpression const reg("<li class=\"dt-label\"><span>of ([^ ]*) pages</span></li>");
        auto const match = reg.match(contents);
        if(match.hasMatch()){
            auto const max_page = static_cast<size_t>(match.captured(1).replace(",", "").toInt());
            to_next_page(max_page);
        }else{
            QLOG_INFO()<<"cannot find max page";
            emit show_more_images_done();
        }
    });
}


