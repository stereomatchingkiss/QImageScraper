#include "proxy_delegate.hpp"

#include "proxy_settings.hpp"

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
