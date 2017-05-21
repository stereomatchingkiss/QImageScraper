#ifndef JS_FUNCTION_HPP
#define JS_FUNCTION_HPP

#include <QString>

/**
 * @brief keep scrolling the window until
 * Math.abs(window.pageYOffset - document height) < threshold
 * @param threshold self explained
 * @return JS command, for debug only
 */
QString js_scroll_to_window_height(qreal threshold);

/**
 * @brief keep scrolling the window until
 * document height < cur_height || Math.abs(document height - window.pageYOffset) < threshold
 * @param threshold self explained
 * @return JS command, for debug only
 * @todo give a better name
 */
QString js_scroll_to_window_height_2(qreal threshold);

#endif // JS_FUNCTION_HPP
