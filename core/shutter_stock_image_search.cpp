#include "shutter_stock_image_search.hpp"

#include <QsLog.h>

#include <QSize>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>
#include <QWebEnginePage>

namespace{

constexpr int show_more_duration = 2000;

}

shutter_stock_image_search::shutter_stock_image_search(QWebEnginePage &page, QObject *parent) :
    image_search{page, parent},
    page_num_{1},
    show_more_count_{0},
    show_more_limit_{2},
    state_{state::to_search_page},
    stop_show_more_image_{false}
{
}

void shutter_stock_image_search::get_search_image_size(std::function<void (size_t)> callback)
{
    callback(big_img_links_.size());
}

void shutter_stock_image_search::go_to_search_page()
{
    state_ = state::to_search_page;
    QLOG_INFO()<<__func__<<": before load";
    get_web_page().load(QUrl("https://www.shutterstock.com/home"));
    QLOG_INFO()<<__func__<<": after load";
}

void shutter_stock_image_search::load(const QUrl &url)
{
    state_ = state::load_url;
    image_search::load(url);
}

void shutter_stock_image_search::reload()
{
    state_ = state::reload_url;
    get_web_page().load(get_web_page().url());
}

void shutter_stock_image_search::show_more_images(size_t max_search_size)
{
    big_img_links_.clear();
    page_num_ = get_current_page_num();
    small_img_links_.clear();
    show_more_limit_ = (max_search_size) / 103 + 1;
    state_ = state::show_more_images;
    //we need to setup timer because web view may not able to update in time,
    //this may cause the page stop loading next page too early
    show_more_count_ = 0;
    stop_show_more_image_ = false;
    QTimer::singleShot(show_more_duration, [this](){show_more_page();});
}

void shutter_stock_image_search::stop_show_more_images()
{
    stop_show_more_image_ = true;
}

size_t shutter_stock_image_search::get_current_page_num()
{
    QUrl const url = get_web_page().url();
    QRegularExpression const reg("page=(\\d+)");
    auto const match = reg.match(url.toString());
    if(match.hasMatch()){
        return match.captured(1).toInt();
    }

    return 1;
}

void shutter_stock_image_search::load_web_page_finished(bool ok)
{
    QLOG_INFO()<<"load web page finished:"<<ok<<", url:"<<get_web_page().url().toString();
    QLOG_INFO()<<"default state";
    if(state_ != state::show_more_images &&
            get_web_page().url().toString().contains("https://www.shutterstock.com/search?")){
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

void shutter_stock_image_search::parse_img_link(std::function<void(QString const&)> callback)
{
    get_web_page().toHtml([this, callback](QString const &contents)
    {
        QRegularExpression const reg("img src=\"([^\"]*)\"");
        auto iter = reg.globalMatch(contents);
        while(iter.hasNext()){
            QRegularExpressionMatch const match = iter.next();
            QStringList const slist = match.captured(1).split("/");
            big_img_links_.push_back("https://image.shutterstock.com/z/" + slist.back());
            small_img_links_.push_back("https:" + match.captured(1));
        }

        callback(contents);
    });
}

void shutter_stock_image_search::get_imgs_link_from_gallery_page(std::function<void(const QStringList &,
                                                                                    const QStringList &)> callback)
{
    state_ = state::get_img_link_from_gallery_page;
    if(page_num_== 1){
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

void shutter_stock_image_search::get_search_target(std::function<void (const QString &)> callback)
{
    get_web_page().toHtml([callback](QString const &contents)
    {
        QRegularExpression reg("<a class=\"dropdown-item\" href=\"#\" role=\"option\">([^\\<]*)</a>");
        auto match = reg.match(contents);
        callback(match.captured(1));
    });
}

void shutter_stock_image_search::go_to_gallery_page(const QString &target)
{
    state_ = state::to_gallery_page;
    if(!target.isEmpty()){
        QString temp = target;
        temp = temp.replace(QChar(' '), QChar('+'));
        QString const link = QString("https://www.shutterstock.com/search?search_source=base_landing_page&"
                                     "language=en&searchterm=%1&image_type=all").arg(temp);
        get_web_page().load(link);
    }
}

void shutter_stock_image_search::show_more_page()
{
    QLOG_INFO()<<__func__<<":show more count:"<<show_more_count_;
    if(state_ != state::show_more_images){
        return;
    }

    if(stop_show_more_image_){
        stop_show_more_image_ = false;
        emit show_more_images_done();
        return;
    }

    if(show_more_count_ == show_more_limit_){
        emit show_more_images_done();
        return;
    }

    parse_img_link([this](QString const &contents)
    {
        if(contents.contains("Next Page")){
            QLOG_INFO()<<__func__<<":found Next Page";
            QString temp = get_web_page().url().toString();
            QUrl url;
            if(temp.contains("page=")){
                url = QUrl(temp.replace(QRegularExpression("page=\\d+"), QString("page=%1").arg(++page_num_)));
            }else{
                url = temp + QString("&page=%1").arg(++page_num_);
            }
            QLOG_INFO()<<__func__<<":next page url:"<<url;
            get_web_page().load(url);
        }else{
            QLOG_INFO()<<"Reach the end of page";
            stop_show_more_image_ = true;
        }
        ++show_more_count_;
    });
}


