#ifndef OSDGRIDWIDGET_H
#define OSDGRIDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QStringList>
#include <QDebug>

#define FCOSD_MAX_HEIGHT 20
#define FCOSD_MAX_WIDTH  53
#define OSD_MAX_CHARS    1024

class OsdGridWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OsdGridWidget(QWidget *parent = nullptr);

    // 设置 character_map（unsigned short 索引值，高字节=属性，低字节=字符索引）
    void setCharacterMap(const unsigned short map[FCOSD_MAX_HEIGHT][FCOSD_MAX_WIDTH]);
    void clear();

    // 加载 OSD 字体图（PNG，与 SDK 相同格式：每格一个字符）
    bool loadFontImage(const QString &imgPath, int fontW, int fontH);
    bool loadFontByIndex(int fontIdx, bool use1080 = false);  // 0=BF, 1=INAV, 2=Ardu, 3=FTTC, 4=KISS

    // 从 osdChar 目录加载单个字符 PNG（如 osdChar1080/133.png）
    // 这是 SDK 预切好的字符图片，索引 0~numChars-1
    bool loadCharImages(const QString &dirPath, int numChars = 256);
    void unloadFont();

    // 查询可用字体列表
    static QStringList availableFonts();

    bool hasFont() const { return m_hasFont; }
    int cellWidth() const { return m_cellW; }
    int cellHeight() const { return m_cellH; }

    QSize minimumSizeHint() const override { return QSize(FCOSD_MAX_WIDTH * m_cellW, FCOSD_MAX_HEIGHT * m_cellH); }
    QSize sizeHint() const override { return QSize(FCOSD_MAX_WIDTH * m_cellW, FCOSD_MAX_HEIGHT * m_cellH); }

signals:
    void cellHovered(int row, int col, unsigned short value);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void updateOSDGrid();

    unsigned short m_charMap[FCOSD_MAX_HEIGHT][FCOSD_MAX_WIDTH];
    unsigned short m_oldCharMap[FCOSD_MAX_HEIGHT][FCOSD_MAX_WIDTH];

    // QLabel 网格（与 SDK formfcosd 完全相同的方式）
    QLabel *m_pCharLabel[FCOSD_MAX_HEIGHT][FCOSD_MAX_WIDTH];
    QPixmap m_bmpChar[OSD_MAX_CHARS]; // 切割好的字符位图
    int m_cellW;
    int m_cellH;
    int m_numChars;
    bool m_hasFont;
};

#endif // OSDGRIDWIDGET_H
