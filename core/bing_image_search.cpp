#include "bing_image_search.hpp"
#include "js_function.hpp"

#include <QsLog.h>

#include <QSize>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>
#include <QWebEnginePage>

namespace{

constexpr int scroll_page_duration = 1500;

}

bing_image_search::bing_image_search(QWebEnginePage &page, QObject *parent) :
    image_search{page, parent},    
    scroll_count_{0},
    scroll_limit_{2},
    state_{state::to_search_page},
    stop_scroll_page_{false}
{    
}

void bing_image_search::get_search_image_size(std::function<void (size_t)> callback)
{
    get_web_page().toHtml([this, callback](QString const &contents)
    {        
        callback(parse_page_link(contents).size());
    });
}

void bing_image_search::go_to_search_page()
{
    state_ = state::to_search_page;    
    QLOG_INFO()<<__func__<<": before load";
    get_web_page().load(QUrl("https://www.bing.com/?scope=images&nr=1&FORM=NOFORM"));
    QLOG_INFO()<<__func__<<": after load";
}

void bing_image_search::load(const QUrl &url)
{
    state_ = state::load_url;
    image_search::load(url);
}

void bing_image_search::reload()
{
    state_ = state::reload_url;
    get_web_page().load(get_web_page().url());
}

void bing_image_search::show_more_images(size_t max_search_size)
{    
    scroll_limit_ = (max_search_size) / 35 + 1;
    state_ = state::show_more_images;
    //we need to setup timer because web view may not able to update in time,
    //this may cause the page stop scrolling too early
    scroll_count_ = 0;
    stop_scroll_page_ = false;
    QTimer::singleShot(scroll_page_duration, [this](){scroll_web_page();});
}

void bing_image_search::stop_show_more_images()
{
    stop_scroll_page_ = true;
}

void bing_image_search::load_web_page_finished(bool ok)
{
    QLOG_INFO()<<"load web page finished:"<<ok<<", url:"<<get_web_page().url().toString();
    if(ok){
        if(get_web_page().url().toString().contains("https://www.bing.com/images/search?q=")){
            state_ = state::to_gallery_page;
            emit go_to_gallery_page_done();
            return;
        }                

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
            break;
        }        
        case state::get_img_link_from_gallery_page:{
            QLOG_INFO()<<"state get_img_link_from_gallery_page";
            return;
        }
        default:
            QLOG_INFO()<<"default state";
            break;
        }
    }else{
        emit search_error(image_search_error::error::load_page_error);
    }
}

void bing_image_search::get_imgs_link_from_gallery_page(std::function<void(const QStringList &, const QStringList &)> callback)
{
    state_ = state::get_img_link_from_gallery_page;
    get_web_page().toHtml([this, callback](QString const &contents)
    {
        //<a class="iusc" style="height:180px;width:239px"
        //m="{&quot;cid&quot;:&quot;S0tIsNfh&quot;,&quot;purl&quot;:&quot;
        //https://whiteglovechimney.com/causing-smoke/&quot;,&quot;murl&quot;:&quot;
        //https://whiteglovechimney.com/wp-content/uploads/2014/01/What-is-Causing-the-Smoke-Chico-CA-Flue-Season.jpg
        //&quot;,&quot;turl&quot;:&quot;https://tse4.mm.bing.net/th?
        //QRegularExpression const reg("m=\"{[^,]*([^;]*;){7}([^&]*)[^\"]*\" "
        //                             "src=\"{([^;]*;){3}([^;]*;)");
        QRegularExpression const reg("<a class=\"iusc\" style=\"[^\"]*\" "
                                     "m=\"{.+?(?=http)http(.+?(?=http)(http[s]?[^&]*)"
                                     ".+?(?=src=)src=\"([^\"]*))");
        auto iter = reg.globalMatch(contents);
        QStringList big_img, small_img;
        while(iter.hasNext()){
            QRegularExpressionMatch const match = iter.next();
            QLOG_INFO()<<"parse_imgs_link_from_second_page:"<<match.captured(2)<<","<<match.captured(3);
            big_img.push_back(match.captured(2).replace("&amp;", "&"));
            small_img.push_back(match.captured(3).replace("&amp;", "&"));
        }
        QLOG_INFO()<<__func__<<" big img size = "<<big_img.size()<<", small img url = "<<small_img.size();
        callback(big_img, small_img);
    });
}

void bing_image_search::get_search_target(std::function<void (const QString &)> callback)
{
    get_web_page().runJavaScript("function jscmd(){return document.getElementById(\"sb_form_q\").value} jscmd()",
                                 [this, callback](QVariant const &contents)
    {
        QLOG_INFO()<<"gallery page target:"<<contents.toString();        
        callback(contents.toString());
    });
}

void bing_image_search::go_to_gallery_page(const QString &target)
{
    state_ = state::to_gallery_page;
    if(!target.isEmpty()){
        get_web_page().load("https://www.bing.com/images/search?q=" + target);
    }
}

void bing_image_search::scroll_web_page()
{    
    QLOG_INFO()<<__func__<<":scroll_count:"<<scroll_count_;
    if(state_ != state::show_more_images){
        return;
    }

    if(stop_scroll_page_){
        stop_scroll_page_ = false;
        emit show_more_images_done();
        return;
    }

    if(scroll_count_ == scroll_limit_){
        emit show_more_images_done();
        return;
    }

    get_web_page().toPlainText([this](QString const &contents)
    {
        if(contents.contains("See more images")){
            QLOG_INFO()<<"found See more images";
            //should not click See more images button if it is not shown on
            //the page, click it blindly may cause the scrolling process become
            //extremely slow(this maybe a trap setup by the search engine, to
            //prevent robot)
            get_web_page().runJavaScript("document.getElementsByClassName(\"btn_seemore\")[0].click();"
                                         "window.scrollTo(0, document.body.scrollHeight);");
        }else{
            QLOG_INFO()<<"cannot found See more images";
            get_web_page().runJavaScript("window.scrollTo(0, document.body.scrollHeight)");
        }
        ++scroll_count_;
        emit second_page_scrolled();
        QTimer::singleShot(scroll_page_duration, [this](){scroll_web_page();});
    });
}

QStringList bing_image_search::parse_page_link(const QString &contents)
{
    QRegularExpression const reg("(search\\?view=detailV2[^\"]*)");
    auto iter = reg.globalMatch(contents);
    QStringList links;
    while(iter.hasNext()){
        QRegularExpressionMatch const match = iter.next();
        if(match.captured(1).right(20) != "ipm=vs#enterinsights"){
            QString url = QUrl("https://www.bing.com/images/" + match.captured(1)).toString();
            url.replace("&amp;", "&");
            links.push_back(url);
        }
    }
    links.removeDuplicates();
    QLOG_INFO()<<"total match link:"<<links.size();

    return links;
}
