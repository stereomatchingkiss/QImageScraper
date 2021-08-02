#include "info_dialog.hpp"
#include "ui_info_dialog.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QTextBrowser>

info_dialog::info_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::info_dialog)
{
    ui->setupUi(this);
}

info_dialog::~info_dialog()
{
    delete ui;
}

void info_dialog::on_pushButtonQt_clicked()
{
    QMessageBox::aboutQt(this);
}

void info_dialog::create_text_browser(QString const &contents)
{
    QDialog html_dialog(this);

    QTextBrowser* text_help = new QTextBrowser(&html_dialog);
    text_help->setOpenExternalLinks(true);
    text_help->setHtml(contents);

    QPushButton* ok_button = new QPushButton( "OK", &html_dialog );

    QHBoxLayout* button_layout = new QHBoxLayout;
    button_layout->addStretch();
    button_layout->addWidget( ok_button );

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(text_help);
    main_layout->addLayout(button_layout);
    main_layout->setStretchFactor( text_help, 2);
    html_dialog.setLayout(main_layout);

    connect(ok_button, &QPushButton::clicked, &html_dialog, &QDialog::accept);
    html_dialog.resize(size() * 2 / 3);
    html_dialog.exec();
}

void info_dialog::on_pushButtonFatCow_clicked()
{
    create_text_browser("All of the icons are come from "
                        "<a href = \"http://www.fatcow.com/free-icons\">FatCow</a>");
}

void info_dialog::on_pushButtonAuthor_clicked()
{
    create_text_browser("This software is created by <font color=\"green\">Tham</font>, </br>"
                        "if you have any trouble, please contact me by </br>"
                        "the email <font color=\"blue\">thamngapwei@gmail.com</font> or "
                        "go to the project <a href=\"https://github.com/stereomatchingkiss/QImageScraper\">website</a>");
}

void info_dialog::on_pushButtonLinkCouldFound_clicked()
{
    emit check_link_could_found();
}
