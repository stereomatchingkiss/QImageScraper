#include "utility.hpp"

#include <QFileInfo>
#include <QNetworkInterface>
#include <QNetworkRequest>

QString convert_url_to_filename(QString const &url, QString const &save_at)
{
    QFileInfo file_info(url);
    QString file_name = file_info.fileName();
    if(QFile::exists(save_at + "/" + file_name)){
        QString const base_name = file_info.baseName();
        QString complete_suffix = file_info.completeSuffix();
        if(complete_suffix.isEmpty()){
            complete_suffix = "txt";
        }
        QString new_file_name = base_name + "(0)." + complete_suffix;
        for(size_t i = 1; QFile::exists(save_at + "/" + new_file_name); ++i){
            new_file_name = base_name + "(" + QString::number(i) + ")." + complete_suffix;
        }

        return new_file_name;
    }

    return file_name;
}

QNetworkRequest create_img_download_request(const QString &url, const QString &engine)
{
    if(engine == "Bing"){
        QNetworkRequest request(url);
        QString const header = "msnbot/2.0b (+http://search.msn.com/msnbot.htm)";
        //QString const header = "msnbot-media/1.1 (+http://search.msn.com/msnbot.htm)";
        //QString const header = "Mozilla/5.0 (compatible; bingbot/2.0; +http://www.bing.com/bingbot.htm)";
        request.setHeader(QNetworkRequest::UserAgentHeader, header);

        return request;
    }

    return QNetworkRequest(url);
}

bool is_connected_to_network()
{
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    bool result = false;

    for (int i = 0; i < ifaces.count(); ++i){
        if(result){
            break;
        }
        QNetworkInterface iface = ifaces.at(i);
        if(iface.flags().testFlag(QNetworkInterface::IsUp)
             && !iface.flags().testFlag(QNetworkInterface::IsLoopBack)){

            // we have an interface that is up, and has an ip address
            // therefore the link is present
            // we will only enable this check on first positive,
            // all later results are incorrect
            if(!iface.addressEntries().empty()){
                if(!result){
                    result = true;
                }
            }
        }
    }

    return result;
}
