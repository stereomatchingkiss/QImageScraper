#ifndef IMAGE_SEARCH_ERROR_HPP
#define IMAGE_SEARCH_ERROR_HPP

class image_search_error
{
public:
    enum class error
    {
        load_page_error,
        invalid_search_target
    };
};

#endif // IMAGE_SEARCH_ERROR_HPP
