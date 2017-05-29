What is QImageScraper ?
------------

QImageScraper is an app for researching how to scrape images by Qt5 from different search engines like 
Google, Bing and Yahoo(Primitive motivation is create an app to help me collect image needed 
for my computer vision projects).

For more information and download prebuilt apps, see [Version Info](https://github.com/stereomatchingkiss/QImageScraper/blob/master/VERSION_INFO.md).

Why Qt5 ?
------------

If you asked on Stack Overflow which tool is nice for web scraping, none of them will answer you Qt5.
Contraty with those I answer, I think Qt5 is a great tool for this kind of task, Qt5 provide us 
almost everything we need for web scraping, it gots

- High level, intuitive, asynchronous network api like QNetworkAccessManager, QNetworkReply,
QNetworkProxy etc
- Powerful regex class like QRegularExpression
- Decent web engine like QtWebEngine
- Robust, mature gui like QWidgets
- Qt5 api, signal and slot make writing asynchronous codes become much easy
- Cross major desktop platforms, windows, mac os and linux, write once compiled anywhere,
one code bases only.

With so many infrastructures, I think Qt5 is a nice tool to build an image scraper.

To know more details about image scraping with Qt5, see
[Download Bing images by Qt5](http://qtandopencv.blogspot.my/2017/05/scrape-bing-images-by-qwebengine.html) 

Dependencies
------------

To build QImageScraper from source you will need
- [Qt5.6.2](https://www.qt.io/download-open-source/) (>= 5.6.2, recommended version is Qt5.6.x , x >= 2), and
- [QsLog](https://bitbucket.org/codeimproved/qslog) (Use git to clone it).
- [icons](https://www.dropbox.com/s/lrtu3abv90d4j3f/icons.7z?dl=0) (Unzip it in the folder QImageScraper/icons)

Bug Reporting
-------------

Please open an issue in github(recommended) or send me an [email](thamngapwei@gmail.com)
