#include "proxy_password_delegate.hpp"

#include <QDebug>
#include <QLineEdit>
#include <QPainter>

proxy_password_delegate::proxy_password_delegate(QObject *parent) :
    QItemDelegate(parent)
{

}

void proxy_password_delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem view_option(option);
    if(index.data(Qt::UserRole + 1).value<QString>().isEmpty()){
        painter->save();
        painter->setPen(QPen(QColor("#00e0fc")));
        painter->setFont(QFont("Arial", 8, QFont::Bold));
        painter->drawText(view_option.rect, Qt::AlignCenter, tr("Optional"));
        painter->restore();
    }else{
        QItemDelegate::paint(painter, option, index);
    }
}

QWidget *proxy_password_delegate::createEditor(QWidget *parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    QLineEdit *line_edit = new QLineEdit(parent);
    line_edit->setEchoMode(QLineEdit::Password);
    return line_edit;
}

void proxy_password_delegate::setModelData(QWidget *widget, QAbstractItemModel *model, const QModelIndex &index) const
{
    model->setData(index, QString(4, QChar('*')), Qt::DisplayRole);
    model->setData(index, qobject_cast<QLineEdit*>(widget)->text(), Qt::UserRole + 1);
}
