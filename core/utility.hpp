#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <QString>

class QNetworkRequest;

QString convert_url_to_filename(QString const &url, QString const &save_at);

QNetworkRequest create_img_download_request(QString const &url, QString const &engine);

#endif // UTILITY_HPP
