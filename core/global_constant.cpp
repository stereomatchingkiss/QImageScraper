#include "global_constant.hpp"

#include <QObject>

QString global_constant::bing_search_name()
{
    return QObject::tr("Bing");
}

QString global_constant::google_search_name()
{
    return QObject::tr("Google");
}

int global_constant::network_reply_timeout()
{
    return 1000 * 60;
}

QString global_constant::yahoo_search_name()
{
    return QObject::tr("Yahoo");
}
