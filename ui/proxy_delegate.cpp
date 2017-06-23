#include "proxy_delegate.hpp"

#include "proxy_settings.hpp"

#include <QLineEdit>
#include <QPainter>

proxy_delegate::proxy_delegate(QObject *parent) :
    QItemDelegate(parent)
{

}

void proxy_delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem view_option(option);
    if(index.column() == static_cast<int>(proxy_settings::proxy_field::user_name) ||
            index.column() == static_cast<int>(proxy_settings::proxy_field::password)){
        if(index.data(Qt::DisplayRole).value<QString>().isEmpty()){
            painter->save();
            painter->setPen(QPen(QColor("#00e0fc")));
            painter->setFont(QFont("Arial", 8, QFont::Bold));
            painter->drawText(view_option.rect, Qt::AlignCenter, tr("Optional"));
            painter->restore();
        }else{
            painter->drawText(view_option.rect, "");
        }
    }

    QItemDelegate::paint(painter, view_option, index);
}

QWidget *proxy_delegate::createEditor(QWidget *parent, const QStyleOptionViewItem&, const QModelIndex &index) const
{
    if(index.column() == static_cast<int>(proxy_settings::proxy_field::password)){
        QLineEdit *line_edit = new QLineEdit(parent);
        line_edit->setEchoMode(QLineEdit::Password);
        return line_edit;
    }

    return parent;
}

void proxy_delegate::setModelData(QWidget *widget, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(index.column() == static_cast<int>(proxy_settings::proxy_field::password)){
        model->setData(index, QString(4, QChar('*')), Qt::DisplayRole);
        model->setData(index, qobject_cast<QLineEdit*>(widget)->text(), Qt::UserRole + 1);
        qDebug()<<__func__<<":display role="<<model->data(index, Qt::DisplayRole).value<QString>();
        qDebug()<<__func__<<":eidt role="<<model->data(index, Qt::EditRole).value<QString>();
        qDebug()<<__func__<<":user role="<<model->data(index, Qt::UserRole + 1).value<QString>();
    }
}
