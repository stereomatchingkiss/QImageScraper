#include "js_function.hpp"

namespace{

QString doc_height()
{
    return QString(
                "function doc_height(){"
                "  return Math.max("
                "    document.body.scrollHeight, document.documentElement.scrollHeight,"
                "    document.body.offsetHeight, document.documentElement.offsetHeight,"
                "    document.body.clientHeight, document.documentElement.clientHeight);"
                "}"
                );
}

}

QString js_scroll_to_window_height(qreal threshold)
{
    return doc_height() + QString("\n"
                "var dheight = doc_height();"
                "function scrollPage(){"
                "  var cur_height = window.innerHeight + window.pageYOffset;"
                "  if(Math.abs(window.pageYOffset - dheight) < %1){"
                "    return [false, cur_height, dheight];"
                "  }else{"
                "    window.scrollTo(0, window.pageYOffset + %1);"
                "    return [true, cur_height, dheight];"
                "  }"
                "}"
                "scrollPage()").arg(threshold);
}

QString js_scroll_to_window_height_2(qreal threshold)
{
    return doc_height() + QString("\n"
                "var dheight = doc_height();"
                "function scrollPage(){"
                "  var cur_height = window.innerHeight + window.pageYOffset;"
                "  if(dheight <= cur_height || Math.abs(dheight - window.pageYOffset) < %1){"
                "    return [false, cur_height, dheight];"
                "  }else{"
                "    window.scrollTo(0, window.pageYOffset + Math.abs((dheight - window.pageYOffset)/2.0));"
                "    return [true, cur_height, dheight];"
                "  }"
                "}"
                "scrollPage()").arg(threshold);
}
