#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QNetworkReply>

#include <map>
#include <memory>

#include <qt_enhance/network/download_supervisor.hpp>

class image_search;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:    
    void on_comboBoxSearchBy_activated(const QString &arg1);

    void on_actionGo_triggered();

private:
    enum class link_choice
    {
        big,
        small
    };

    void found_img_link(QString const &big_img_link, QString const &small_img_link);
    void download_finished(std::shared_ptr<qte::net::download_supervisor::download_task> task);
    void download_image();
    void download_progress(size_t unique_id, qint64 bytesReceived, qint64 bytesTotal);

    Ui::MainWindow *ui;

    QSize default_max_size_;
    qte::net::download_supervisor *downloader_;
    std::map<size_t, std::tuple<QString, QString, link_choice>> img_links_;
    image_search *img_search_;
};

#endif // MAINWINDOW_HPP
