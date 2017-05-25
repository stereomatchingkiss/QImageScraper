#ifndef GENERAL_SETTINGS_HPP
#define GENERAL_SETTINGS_HPP

#include <QDialog>

namespace Ui {
class general_settings;
}

class general_settings : public QDialog
{
    Q_OBJECT

public:
    explicit general_settings(QWidget *parent = nullptr);
    ~general_settings();

    int get_max_download_img() const;
    QString get_save_at() const;
    QString get_search_by() const;

signals:
    void cannot_create_save_dir(QString const &dir, QString const &write_able_path);
    void ok_clicked();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_pushButtonSaveAt_clicked();

private:
    void closeEvent(QCloseEvent *event) override;

    void restore_value();

    Ui::general_settings *ui;

    QString const write_able_path_;
};

#endif // GENERAL_SETTINGS_HPP
