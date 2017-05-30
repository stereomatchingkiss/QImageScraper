#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <QString>

class QNetworkRequest;

QNetworkRequest create_img_download_request(QString const &url);

#endif // UTILITY_HPP
