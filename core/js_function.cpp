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

QString js_scroll_to_window_height(qreal)
{
    return doc_height() + QString("\n"
                "var dheight = doc_height();"
                "function scrollPage(){"
                "  var cur_height = window.innerHeight + window.pageYOffset;"
                "  if(Math.abs(window.pageYOffset - document.body.scrollHeight) < 1000){"
                "    return [false, cur_height, dheight];"
                "  }else{"
                "    window.scrollTo(0, window.pageYOffset + 1000);"
                "    return [true, cur_height, dheight];"
                "  }"
                "}"
                "scrollPage()");
}
