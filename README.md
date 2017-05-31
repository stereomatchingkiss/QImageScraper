What is QImageScraper ?
------------

QImageScraper is an free app(no advertising, no in app purchase) for researching how to scrape images 
by Qt5 from different search engines like Google, Bing and Yahoo(Primitive motivation is create an app 
to help me collect image needed for my computer vision projects).

For more information and download prebuilt apps, see [Version Info](https://github.com/stereomatchingkiss/QImageScraper/blob/master/VERSION_INFO.md).

- ![google gallery](
https://s15.postimg.org/5sqfv402j/gallery_0.jpg
)
- ![bing gallery](
https://s15.postimg.org/6jj60w2fv/gallery_1.jpg
)
- ![yahoo gallery](
https://s15.postimg.org/5vabhy3q3/gallery_2.jpg
)

Why Qt5 ?
------------

If you asked on Stack Overflow which tool is nice for web scraping, none(?) of them will answer you Qt5.
Contrary with those answers, I think Qt5 is a great tool for this kind of task, Qt5 provide us 
almost everything we need for web scraping, it gots

- High level, intuitive, asynchronous network api like QNetworkAccessManager, QNetworkReply, QNetworkProxy etc
- Powerful regex class like QRegularExpression
- Decent web engine like QtWebEngine
- Robust, mature gui like QWidgets
- Most of the Qt5 api are well designed, signal and slot make writing asynchronous codes become much easier too
- Great unicode support
- Able to determine image type from the contents
- Feature rich file system library. Whether create, remove, rename or find standard path to save image is piece of cake in Qt5
- QsLog is a nice logging library which only depend on Qt5
- Asynchronous api of QNetworkAccessManager make it easy to spawn many download request at once
- Cross major desktop platforms, windows, mac os and linux, write once compiled anywhere, one code bases only.
- Easy to deploy on windows and mac(linux?maybe linuxdeployqt can save us tons of troubles)
- QSettings provide an easy way to provides persistent platform-independent application settings.
- and so on

With so many infrastructures, I think Qt5 is a nice tool to build an image scraper app.

Someone may say, "c++ is very low level, I do not want to manage memory by myself".
My answer are

- We have a python binding of Qt(PyQt)
- I seldom found a need to worry about any resource leak by using c++ after I learn RAII, parent and child relationship of Qt.If you always found the resource of c++ as hard to manage as c, that means you do not know how to use c++ effectively.

To know more details about image scraping with Qt5, see
- [Download Bing images by Qt5](http://qtandopencv.blogspot.my/2017/05/scrape-bing-images-by-qwebengine.html) 
- [Create a better images downloader(Google, Bing and Yahoo) by Qt5](http://qtandopencv.blogspot.my/2017/05/create-better-images-downloadergoogle.html)

Dependencies
------------

To build QImageScraper from source you will need
- [Qt5.6.2](https://www.qt.io/download-open-source/) (>= 5.6.2, recommended version is Qt5.6.x , x >= 2), and
- [QsLog](https://bitbucket.org/codeimproved/qslog) (Use git to clone it, do no need to build, include the .pri is enough).
- [qt_enhance](https://github.com/stereomatchingkiss/qt_enhance) (Do not need to compile, just put it at the same directory of QImageScraper)
- [icons](https://www.dropbox.com/s/lrtu3abv90d4j3f/icons.7z?dl=0) (Unzip it in the folder QImageScraper/icons)

Bug Reporting
-------------

Please open an issue on github(recommended) or send me an [email](thamngapwei@gmail.com)
