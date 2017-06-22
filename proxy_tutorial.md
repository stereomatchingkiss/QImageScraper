Why proxy ?
------------

Because proxy may help us bypass blocking of some websites, this could help us
increase the success rate to download the images.

How to setup proxy?
------------

1. Click the Settings button.![pic_00](
https://s22.postimg.org/r4eyxe775/setting_button.png
)
1. Select Proxy![pic_01](
https://s16.postimg.org/amzdag0lx/proxy_00.png
)
1. Select the modes(No proxy, Manual proxy)




No proxy
------------

This mode do not use any proxy and is the default mode

Manual proxy
------------

This mode allow you to setup multiple proxies at once, when the QImageScraper failed to download
full size image, it will instead download the images in the proxy list with random order.

### Setup

1. Select manual proxy
1. Press add button![pic_02](https://s12.postimg.org/ms1gpekkt/proxy_01.png)
1. Enter host, port, user name(optional), password(optional) and type of your proxies

### Type of proxy

- DefaultProxy : Proxy is determined based on the application proxy
- Socks5Proxy : Socks5 proxying is used
- HttpProxy : HTTP transparent proxying is used
- HttpCachingProxy : Proxying for HTTP requests only
- FtpCachingProxy : Proxying for FTP requests only
