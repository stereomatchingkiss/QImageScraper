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
    if(index.data(Qt::DisplayRole).value<QString>().isEmpty()){
        painter->save();
        painter->setPen(QPen(QColor("#00e0fc")));
        painter->setFont(QFont("Arial", 8, QFont::Bold));
        painter->drawText(view_option.rect, Qt::AlignCenter, tr("Optional"));
        painter->restore();
    }else{
        QItemDelegate::paint(painter, option, index);
    }
}
