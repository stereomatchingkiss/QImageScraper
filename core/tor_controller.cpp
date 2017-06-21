#include "tor_controller.hpp"

#include <QsLog.h>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QTcpSocket>
#include <QTimer>

tor_controller::tor_controller(QObject *parent) :
    QObject(parent),
    control_port_(9151),
    network_manager_(new QNetworkAccessManager(this)),
    port_(9150),
    reply_(nullptr),
    socket_(new QTcpSocket(this)),
    state_(tor_state::renew_ip),
    timer_(new QTimer(this))
{
    timer_->setSingleShot(true);
    connect(socket_, &QTcpSocket::connected, [this]()
    {
        QLOG_INFO()<<"connect to tor";
        QString const contents(QString("AUTHENTICATE \"%1\"\r\nSIGNAL NEWNYM").arg(password_));
        socket_->write(contents.toLatin1());
    });
    connect(socket_, static_cast<void(QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this, &tor_controller::handle_error);
    connect(socket_, &QTcpSocket::readyRead, this, &tor_controller::handle_ready_read);
    connect(timer_, &QTimer::timeout, [this]()
    {
        socket_->abort();
        if(state_ == tor_state::renew_ip){
            emit error_happen(tr("Tor server do not give any response"));
        }else if(state_ == tor_state::check_tor_validation){
            if(reply_){
                reply_->abort();
            }else{
                emit error_happen(tr("Please check configuration of your tor browser/network environment"));
            }
        }
    });
}

void tor_controller::check_tor_validation(const QString &host, quint16 port, quint16 control_port, const QString &password)
{
    host_ = host;
    port_ = port;
    control_port_ = control_port;
    password_ = password;
    state_ = tor_state::check_tor_validation;
    timer_->setInterval(15000);
    renew_ip_impl(host, control_port, password);
}

void tor_controller::renew_ip(QString const &host, quint16 control_port, QString const &password)
{
    state_ = tor_state::renew_ip;
    timer_->setInterval(5000);
    renew_ip_impl(host, control_port, password);
}

void tor_controller::access_network_by_tor(const QString &link)
{        
    QNetworkProxy proxy(QNetworkProxy::Socks5Proxy, host_, port_);
    network_manager_->setProxy(proxy);

    reply_ = network_manager_->get(QNetworkRequest(link));
    connect(reply_, &QNetworkReply::finished, this, &tor_controller::handle_network_finished);
}

void tor_controller::handle_error(QAbstractSocket::SocketError error)
{
    timer_->stop();
    QLOG_INFO()<<"connect to tor error occur:"<<error;
    //emit error_happen(socket_->errorString());
}

void tor_controller::handle_ready_read()
{    
    QLOG_INFO()<<__func__;
    timer_->start();
    if(socket_->bytesAvailable() >= 3){
        if(socket_->readAll().startsWith("250")){
            timer_->stop();
            QLOG_INFO()<<__func__<<":renew tor ip success";
            emit renew_ip_success();
            if(state_ == tor_state::check_tor_validation){
                timer_->start();
                QLOG_INFO()<<__func__<<":access network by tor";
                access_network_by_tor("https://google.com/");
            }
        }else{
            timer_->stop();
            QLOG_INFO()<<__func__<<": emit error string";
            emit error_happen(socket_->errorString());
        }
    }
}

void tor_controller::handle_network_finished()
{
    QLOG_INFO()<<__func__;
    reply_ = qobject_cast<QNetworkReply*>(sender());
    if(reply_){
        if(reply_->error() == QNetworkReply::NoError){
            timer_->stop();
            emit validate_tor_success();
        }else{
            emit validate_tor_fail(reply_->errorString());
        }
        reply_->deleteLater();
        reply_ = nullptr;
    }else{
        QLOG_ERROR()<<__func__<<":Cannot get QNetworkReply";
        emit validate_tor_fail(tr("Cannot access network by tor"));
    }
}

void tor_controller::renew_ip_impl(const QString &host, quint16 control_port, const QString &password)
{
    socket_->abort();
    password_ = password;
    socket_->connectToHost(host, control_port);
    timer_->start();
}
