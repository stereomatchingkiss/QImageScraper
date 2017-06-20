#include "tor_controller.hpp"

#include <QsLog.h>
#include <QByteArray>
#include <QTcpSocket>
#include <QTimer>

tor_controller::tor_controller(QObject *parent) :
    QObject(parent),
    socket_(new QTcpSocket(this)),
    state_(tor_state::authenticate),
    timer_(new QTimer(this))
{
    connect(socket_, &QTcpSocket::connected, [this]()
    {
        QLOG_INFO()<<"connect to tor";
        QString const contents(QString("AUTHENTICATE \"%1\"\r\n").arg(password_));
        socket_->write(contents.toLatin1());
    });
    connect(socket_, static_cast<void(QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this, &tor_controller::handle_error);
    connect(socket_, &QTcpSocket::readyRead, this, &tor_controller::handle_ready_read);
    connect(timer_, &QTimer::timeout, [this]()
    {
        emit error_happen("Tor server do not give any response");
        socket_->abort();
    });
}

void tor_controller::renew_ip(const QString &host, quint16 port, const QString &password)
{
    socket_->abort();
    password_ = password;
    socket_->connectToHost(host, port);
    timer_->start(5000);    
}

void tor_controller::handle_error(QAbstractSocket::SocketError error)
{
    timer_->stop();
    QLOG_INFO()<<"connect to tor error occur:"<<error;
    state_ = tor_state::authenticate;
    emit error_happen(socket_->errorString());
}

void tor_controller::handle_ready_read()
{    
    timer_->start();
    if(socket_->bytesAvailable() >= 3){
        if(socket_->readAll().startsWith("250")){
            if(state_ == tor_state::authenticate){
                emit authenticate_success();
                state_ = tor_state::renew_ip;
                socket_->write("SIGNAL NEWNYM\r\n");
            }else{
                timer_->stop();
                emit renew_ip_success();
            }
        }
    }
}
