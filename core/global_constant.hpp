#ifndef GLOBAL_CONSTANT_HPP
#define GLOBAL_CONSTANT_HPP

#include <QString>

class global_constant
{
public:
    static QString adobe_stock_name();
    static QString deposit_photo_name();
    static QString dream_time_name();
    static QString bing_search_name();
    static QString google_search_name();
    static QString pexels_search_name();
    static constexpr int network_reply_timeout()
    {
        return 1000 * 60;
    }
    static constexpr size_t download_retry_limit()
    {
        return 2;
    }    
    static constexpr size_t timeout_retry_limit()
    {
        return 5;
    }

    static QString shutter_stock_name();

    static QString yahoo_search_name();
};

#endif // GLOBAL_CONSTANT_HPP
