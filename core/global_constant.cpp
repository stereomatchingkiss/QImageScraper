#include "global_constant.hpp"

#include <QObject>

QString global_constant::adobe_stock_name()
{
    return QObject::tr("AdobeStock");
}

QString global_constant::bing_search_name()
{
    return QObject::tr("Bing");
}

QString global_constant::google_search_name()
{
    return QObject::tr("Google");
}

QString global_constant::shutter_stock_name()
{
    return QObject::tr("ShutterStock");
}

QString global_constant::yahoo_search_name()
{
    return QObject::tr("Yahoo");
}
