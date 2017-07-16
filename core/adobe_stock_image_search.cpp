#include "adobe_stock_image_search.hpp"

#include <QsLog.h>

#include <QSize>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>
#include <QWebEnginePage>

namespace{

constexpr int show_more_duration = 2000;

}

adobe_stock_image_search::adobe_stock_image_search(QWebEnginePage &page, QObject *parent) :
    image_search{page, parent},
    max_search_size_{0},
    page_num_{1},
    show_more_count_{0},
    show_more_limit_{0},
    state_{state::to_search_page},
    stop_show_more_image_{false}
{
}

void adobe_stock_image_search::get_search_image_size(std::function<void (size_t)> callback)
{
    callback(big_img_links_.size());
}

void adobe_stock_image_search::go_to_search_page()
{
    state_ = state::to_search_page;
    QLOG_INFO()<<__func__<<": before load";
    get_web_page().load(QUrl("https://stock.adobe.com/"));
    QLOG_INFO()<<__func__<<": after load";
}

void adobe_stock_image_search::load(const QUrl &url)
{
    state_ = state::load_url;
    image_search::load(url);
}

void adobe_stock_image_search::reload()
{
    state_ = state::reload_url;
    get_web_page().load(get_web_page().url());
}

void adobe_stock_image_search::show_more_images(size_t max_search_size)
{
    big_img_links_.clear();
    small_img_links_.clear();
    max_search_size_ = max_search_size;
    page_num_ = get_current_page_num();
    small_img_links_.clear();
    state_ = state::show_more_images;
    show_more_limit_ = (max_search_size) / 101 + 1;
    show_more_count_ = 0;
    //we need to setup timer because web view may not able to update in time,
    //this may cause the page stop loading next page too early
    stop_show_more_image_ = false;
    QTimer::singleShot(show_more_duration, [this](){show_more_page();});
}

void adobe_stock_image_search::stop_show_more_images()
{
    stop_show_more_image_ = true;
}

size_t adobe_stock_image_search::get_current_page_num()
{
    QUrl const url = get_web_page().url();
    QRegularExpression const reg("search_page=(\\d+)");
    auto const match = reg.match(url.toString());
    if(match.hasMatch()){
        return match.captured(1).toInt();
    }

    return 1;
}

void adobe_stock_image_search::load_web_page_finished(bool ok)
{
    QLOG_INFO()<<"load web page finished:"<<ok<<", url:"<<get_web_page().url().toString();
    if(state_ != state::show_more_images &&
            get_web_page().url().toString().contains("https://stock.adobe.com/search?")){
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

void adobe_stock_image_search::parse_img_link(std::function<void(QString const&)> callback)
{
    get_web_page().toHtml([this, callback](QString const &contents)
    {
        auto global_parse = [](QString const &contents, QString const &regex_cmd, QStringList &result)
        {
            QRegularExpression const reg(regex_cmd);
            auto iter = reg.globalMatch(contents);
            while(iter.hasNext()){
                QRegularExpressionMatch match = iter.next();
                result.push_back(match.captured(1));
            }
        };

        global_parse(contents, "data-comp-url=\"([^\"]*)\"", big_img_links_);
        global_parse(contents, "div class=\"thumb-frame[^<]*<[^<]*<img src=\"([^\"]*)", small_img_links_);

        callback(contents);
    });
}

void adobe_stock_image_search::get_imgs_link_from_gallery_page(std::function<void(const QStringList &,
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

void adobe_stock_image_search::get_search_target(std::function<void (const QString &)> callback)
{
    if(get_web_page().url().toString().contains("https://stock.adobe.com/search?")){
        get_web_page().toHtml([callback](QString const &contents)
        {
            QRegularExpression const reg("\"usertyped_keyword\":\"([^\"]*)\"");
            callback(reg.match(contents).captured(1));
        });
    }else{
        get_web_page().runJavaScript("function get_target(){"
                                     "  return document.getElementsByClassName(\"js-search-text-input input--silent input--full light input--xlarge\")[0].value"
                                     "} get_target()", [callback](QVariant const &value)
        {
            callback(value.toString());
        });
    }
}

void adobe_stock_image_search::go_to_gallery_page(const QString &target)
{
    state_ = state::to_gallery_page;
    if(!target.isEmpty()){
        QString const link = QString("https://stock.adobe.com/search?load_type="
                                     "&native_visual_search=&similar_content_id=&k=%1").arg(target);
        get_web_page().load(link);
    }
}

void adobe_stock_image_search::show_more_page()
{
    if(state_ != state::show_more_images){
        return;
    }

    if(stop_show_more_image_){
        stop_show_more_image_ = false;
        state_ = state::load_url;
        emit show_more_images_done();
        return;
    }

    if(show_more_count_ >= show_more_limit_){
        QLOG_INFO()<<"show_more_count_ >= show_more_limit_";
        state_ = state::load_url;
        emit show_more_images_done();
    }

    QLOG_INFO()<<"found "<<big_img_links_.size()<<" image";

    parse_img_link([this](QString const &contents)
    {
        if(contents.contains("Next")){
            QLOG_INFO()<<__func__<<":found Next Page";
            QString temp = get_web_page().url().toString();
            QUrl url;
            if(temp.contains("search_page=")){
                url = QUrl(temp.replace(QRegularExpression("search_page=\\d+"), QString("search_page=%1").arg(++page_num_)));
            }else{
                url = temp + QString("&search_page=%1").arg(++page_num_);
            }
            QLOG_INFO()<<__func__<<":next page url:"<<url;
            get_web_page().load(url);
        }
        ++show_more_count_;
    });
}


