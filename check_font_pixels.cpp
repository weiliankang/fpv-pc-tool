// 使用 Qt 检查 font_bf_24.png 的像素内容
// 编译: qmake -project && qmake && mingw32-make
#include <QApplication>
#include <QImage>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QString path = "E:/000_openclaw/file/debug_tool_qt/fonts/font_bf_24.png";
    QImage img(path);
    if (img.isNull()) {
        qCritical() << "Failed to load" << path;
        return 1;
    }
    qDebug() << "Image:" << img.width() << "x" << img.height();
    int yNum = img.height() / 24;
    int xNum = img.width() / 24;
    qDebug() << "xNum:" << xNum << "yNum:" << yNum;
    
    // 用 SDK 算法 x * yNum + y
    qDebug() << "\n=== x*yNum+y (SDK formfcosd) ===";
    for (int idx : {0, 1, 65, 133}) {
        int x = idx / yNum;
        int y = idx % yNum;
        QImage crop = img.copy(x*24, y*24, 24, 24);
        int opaque = 0;
        for (int py = 0; py < 24; py++)
            for (int px = 0; px < 24; px++)
                if (qAlpha(crop.pixel(px, py)) > 0) opaque++;
        qDebug() << "  idx=" << idx << "x=" << x << "y=" << y 
                 << "QR(" << x*24 << "," << y*24 << ",24,24)" 
                 << "opaque=" << opaque;
        crop.save(QString("E:/000_openclaw/file/debug_tool_qt/temp_sdk_%1.png").arg(idx));
    }
    
    // 用 y * xNum + x (row-major)
    qDebug() << "\n=== y*xNum+x (row-major) ===";
    for (int idx : {0, 1, 65, 133}) {
        int y = idx / xNum;
        int x = idx % xNum;
        QImage crop = img.copy(x*24, y*24, 24, 24);
        int opaque = 0;
        for (int py = 0; py < 24; py++)
            for (int px = 0; px < 24; px++)
                if (qAlpha(crop.pixel(px, py)) > 0) opaque++;
        qDebug() << "  idx=" << idx << "y=" << y << "x=" << x 
                 << "QR(" << x*24 << "," << y*24 << ",24,24)" 
                 << "opaque=" << opaque;
        crop.save(QString("E:/000_openclaw/file/debug_tool_qt/temp_row_%1.png").arg(idx));
    }
    
    // 对比 osdChars/1.png
    QImage osd1("E:/000_openclaw/file/debug_tool_qt/osdChars/1.png");
    qDebug() << "\nosdChars/1.png:" << osd1.width() << "x" << osd1.height();
    int opaque1 = 0;
    for (int py = 0; py < osd1.height(); py++)
        for (int px = 0; px < osd1.width(); px++)
            if (qAlpha(osd1.pixel(px, py)) > 0) opaque1++;
    qDebug() << "  opaque=" << opaque1;
    
    return 0;
}
