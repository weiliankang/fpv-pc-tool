#include "osdgridwidget.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QToolTip>
#include <QFileInfo>
#include <QDir>
#include <QImage>
#include <QtMath>

// ============================================================
// OsdGridWidget - 完全对齐 SDK formfcosd 的渲染方式
// 使用 QLabel 网格 + 字体位图切割，逐字符显示
// 字符索引为 unsigned short (16-bit)，低字节为字符码，高字节为属性
// ============================================================

// SDK 字体路径（720p 版本）
#define SDK_IMAGES_720  "X:/prj3_FPV60X/FPV40X_60X_SDK/gnd-ars31-fpv2-sdk-0.16.02-15-release-20260126-cx485/base/arprj/apps/fpv40x/ar_ldy_gui/images"
#define SDK_IMAGES_1080 "X:/prj3_FPV60X/FPV40X_60X_SDK/gnd-ars31-fpv2-sdk-0.16.02-15-release-20260126-cx485/base/arprj/apps/fpv40x/ar_ldy_gui/images_1080"

static const char *FONT_NAMES_720[] = {
    "font_bf_24.png",
    "font_inav_24.png",
    "font_ardu_24.png",
    "font_FTTC24.png",
    "font_kiss_24.png",
};

static const int FONT_SIZES_720[] = {24, 24, 24, 24, 24};

// 字体名称（用于显示）
static const char *FONT_DISPLAY_NAMES[] = {
    "Betaflight",
    "INAV",
    "ArduPilot",
    "Fettec",
    "KISS",
};

OsdGridWidget::OsdGridWidget(QWidget *parent)
    : QWidget(parent)
    , m_cellW(24)
    , m_cellH(24)
    , m_numChars(0)
    , m_hasFont(false)
{
    memset(m_charMap, 0, sizeof(m_charMap));
    memset(m_oldCharMap, 0, sizeof(m_oldCharMap));

    // 创建 QLabel 网格（与 SDK 完全相同的方式）
    for (int y = 0; y < FCOSD_MAX_HEIGHT; y++) {
        for (int x = 0; x < FCOSD_MAX_WIDTH; x++) {
            m_pCharLabel[y][x] = new QLabel(this);
            m_pCharLabel[y][x]->setAttribute(Qt::WA_TranslucentBackground, true);
            m_pCharLabel[y][x]->hide();
        }
    }

    setMouseTracking(true);
    setMinimumSize(FCOSD_MAX_WIDTH * m_cellW, FCOSD_MAX_HEIGHT * m_cellH);
}

void OsdGridWidget::setCharacterMap(const unsigned short map[FCOSD_MAX_HEIGHT][FCOSD_MAX_WIDTH])
{
    memcpy(m_charMap, map, sizeof(m_charMap));
    updateOSDGrid();
}

void OsdGridWidget::clear()
{
    memset(m_charMap, 0, sizeof(m_charMap));
    for (int y = 0; y < FCOSD_MAX_HEIGHT; y++) {
        for (int x = 0; x < FCOSD_MAX_WIDTH; x++) {
            m_oldCharMap[y][x] = 0;
            m_pCharLabel[y][x]->hide();
        }
    }
    update();
}

bool OsdGridWidget::loadFontImage(const QString &imgPath, int fontW, int fontH)
{
    QPixmap fontImg(imgPath);
    if (fontImg.isNull()) {
        qWarning() << "[OSD] 无法加载字体图:" << imgPath;
        return false;
    }

    m_cellW = fontW;
    m_cellH = fontH;

    // 计算字符网格列数（与 SDK 相同算法：x_num = width / font_width）
    int xNum = fontImg.width() / fontW;
    int yNum = fontImg.height() / fontH;
    m_numChars = xNum * yNum;
    if (m_numChars > OSD_MAX_CHARS) m_numChars = OSD_MAX_CHARS;

    qDebug() << "[OSD] 字体图:" << QFileInfo(imgPath).fileName()
             << "图片尺寸:" << fontImg.width() << "x" << fontImg.height()
             << "字符尺寸:" << fontW << "x" << fontH
             << "总字符:" << m_numChars
             << "列数:" << xNum;

    // 从字体图中切割每个字符（与 SDK 完全一致）
    m_hasFont = true;
    for (int x = 0; x < xNum; x++) {
        for (int y = 0; y < yNum && (x * yNum + y) < OSD_MAX_CHARS; y++) {
            QRect srcRect(x * fontW, y * fontH, fontW, fontH);
            m_bmpChar[x * yNum + y] = fontImg.copy(srcRect);
        }
    }

    // 更新 QLabel 位置
    for (int y = 0; y < FCOSD_MAX_HEIGHT; y++) {
        for (int x = 0; x < FCOSD_MAX_WIDTH; x++) {
            m_pCharLabel[y][x]->setGeometry(x * fontW, y * fontH, fontW, fontH);
        }
    }

    setMinimumSize(FCOSD_MAX_WIDTH * fontW, FCOSD_MAX_HEIGHT * fontH);
    updateGeometry();

    // 重置 oldCharMap，强制重新设置所有 QLabel 的 pixmap
    memset(m_oldCharMap, 0xFF, sizeof(m_oldCharMap));

    updateOSDGrid();
    return true;
}

bool OsdGridWidget::loadCharImages(const QString &dirPath, int numChars)
{
    // 判断是否是 Qt 资源路径
    bool isResource = dirPath.startsWith(":/") || dirPath.startsWith("qrc:");

    m_numChars = qMin(numChars, OSD_MAX_CHARS);
    int loadedCount = 0;
    int firstW = 0, firstH = 0;

    for (int i = 0; i < m_numChars; i++) {
        QString imgPath;
        if (isResource) {
            // Qt 资源路径：qv1-.../osdChars/0.png 或 :/osdchars/0.png
            if (dirPath.startsWith("qrc:"))
                imgPath = dirPath.mid(3) + "/" + QString::number(i) + ".png";
            else
                imgPath = dirPath + "/" + QString::number(i) + ".png";
        } else {
            imgPath = dirPath + "/" + QString::number(i) + ".png";
            if (!QFile::exists(imgPath)) {
                m_bmpChar[i] = QPixmap();
                continue;
            }
        }
        QPixmap pix(imgPath);
        if (pix.isNull()) continue;

        m_bmpChar[i] = pix;
        if (loadedCount == 0) {
            firstW = pix.width();
            firstH = pix.height();
            m_cellW = firstW;
            m_cellH = firstH;
        }
        loadedCount++;
    }

    if (loadedCount == 0) {
        qWarning() << "[OSD] osdChar 没有加载到任何字符:" << dirPath;
        return false;
    }

    qDebug() << "[OSD] osdChar 加载成功:" << dirPath
             << "字符尺寸:" << m_cellW << "x" << m_cellH
             << "加载数:" << loadedCount
             << "总数:" << m_numChars;

    // 检查图片尺寸
    for (int i = 1; i < m_numChars; i++) {
        QPixmap &p = m_bmpChar[i];
        if (!p.isNull() && (p.width() != m_cellW || p.height() != m_cellH)) {
            p = p.scaled(m_cellW, m_cellH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
    }

    m_hasFont = true;

    // 更新 QLabel 位置
    for (int y = 0; y < FCOSD_MAX_HEIGHT; y++) {
        for (int x = 0; x < FCOSD_MAX_WIDTH; x++) {
            m_pCharLabel[y][x]->setGeometry(x * m_cellW, y * m_cellH, m_cellW, m_cellH);
        }
    }

    setMinimumSize(FCOSD_MAX_WIDTH * m_cellW, FCOSD_MAX_HEIGHT * m_cellH);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    updateGeometry();
    memset(m_oldCharMap, 0xFF, sizeof(m_oldCharMap));
    updateOSDGrid();
    return true;
}

bool OsdGridWidget::loadFontByIndex(int fontIdx, bool use1080)
{
    if (fontIdx < 0 || fontIdx >= 5) {
        qWarning() << "[OSD] 字体索引越界:" << fontIdx;
        return false;
    }

    QString imgDir = use1080 ? SDK_IMAGES_1080 : SDK_IMAGES_720;
    QString imgPath = imgDir + "/" + FONT_NAMES_720[fontIdx]; // 文件名相同，尺寸不同
    int fontW = use1080 ? 36 : 24;
    int fontH = fontW;

    return loadFontImage(imgPath, fontW, fontH);
}

void OsdGridWidget::unloadFont()
{
    m_hasFont = false;
    for (int i = 0; i < OSD_MAX_CHARS; i++) {
        m_bmpChar[i] = QPixmap();
    }
    for (int y = 0; y < FCOSD_MAX_HEIGHT; y++) {
        for (int x = 0; x < FCOSD_MAX_WIDTH; x++) {
            m_pCharLabel[y][x]->hide();
        }
    }
    m_cellW = 18;
    m_cellH = 18;
    setMinimumSize(FCOSD_MAX_WIDTH * m_cellW, FCOSD_MAX_HEIGHT * m_cellH);
    updateGeometry();
    update();
}

QStringList OsdGridWidget::availableFonts()
{
    QStringList names;
    for (int i = 0; i < 5; i++) {
        names << FONT_DISPLAY_NAMES[i];
    }
    return names;
}

void OsdGridWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    // 深色背景（模拟 OSD 屏幕底色）
    p.fillRect(rect(), QColor(0x0A, 0x0A, 0x0A));

    if (!m_hasFont) {
        // ============ 无字体时的回退模式：QPainter 文字渲染 ============
        int fontSize = 14;
        QFont font("Consolas", -1, QFont::Normal);
        font.setPixelSize(fontSize);
        font.setStyleHint(QFont::Monospace);
        font.setFixedPitch(true);
        p.setFont(font);

        QFontMetrics fm(font);
        int actualCharW = fm.horizontalAdvance('W');
        int actualCharH = fm.height();

        for (int row = 0; row < FCOSD_MAX_HEIGHT; ++row) {
            for (int col = 0; col < FCOSD_MAX_WIDTH; ++col) {
                unsigned short val = m_charMap[row][col];
                if (val == 0) continue;

                int cx = col * m_cellW + (m_cellW - actualCharW) / 2;
                int cy = row * m_cellH + (m_cellH + actualCharH) / 2 - fm.descent();

                unsigned short lo = val & 0xFF;
                if (lo >= 0x20 && lo <= 0x7E) {
                    if (lo >= '0' && lo <= '9') p.setPen(QColor(0x00, 0xFF, 0x88));
                    else if (lo >= 'A' && lo <= 'Z') p.setPen(QColor(0xFF, 0xFF, 0xFF));
                    else if (lo >= 'a' && lo <= 'z') p.setPen(QColor(0xAA, 0xCC, 0xFF));
                    else p.setPen(QColor(0xCC, 0xCC, 0xCC));
                    p.drawText(cx, cy, QChar(lo));
                } else if (lo > 0x7E) {
                    // 扩展字符（图标等）用索引号显示
                    p.setPen(QColor(0x88, 0xFF, 0x88));
                    p.drawText(cx, cy, QString("0x%1").arg(lo, 2, 16, QChar('0')));
                } else {
                    p.setPen(QColor(0x88, 0x88, 0x88));
                    p.drawText(cx, cy, QChar(lo));
                }
            }
        }

        // 淡网格线
        p.setPen(QPen(QColor(0x22, 0x22, 0x22), 1));
        for (int y = 1; y < FCOSD_MAX_HEIGHT; ++y)
            p.drawLine(0, y * m_cellH, FCOSD_MAX_WIDTH * m_cellW, y * m_cellH);
        for (int x = 1; x < FCOSD_MAX_WIDTH; ++x)
            p.drawLine(x * m_cellW, 0, x * m_cellW, FCOSD_MAX_HEIGHT * m_cellH);
    }
}

void OsdGridWidget::updateOSDGrid()
{
    if (!m_hasFont) {
        update(); // 无字体时触发 paintEvent
        return;
    }

    // 与 SDK formfcosd::slot_osdbuf_drawframe() 完全一致：
    // ---
    // c = character_map[y][x];      ← 直接取16位值，不 &0xFF
    // c = c >= num_chars ? num_chars - 1 : c;  ← 边界检查
    // if (c != old_charmap[y][x]) {
    //     old_charmap[y][x] = c;
    //     if (c < OSD_MAX_CHARS && c > 0) {
    //         setPixmap(bmpchar[c]);
    //         show();
    //     } else {
    //         hide();
    //     }
    // }
    // ---
    for (int y = 0; y < FCOSD_MAX_HEIGHT; y++) {
        for (int x = 0; x < FCOSD_MAX_WIDTH; x++) {
            unsigned short c = m_charMap[y][x];
            // 边界检查（SDK 式：超限时 cap 到最大索引）
            if (c >= m_numChars)
                c = m_numChars > 0 ? m_numChars - 1 : 0;
            if (c != m_oldCharMap[y][x]) {
                m_oldCharMap[y][x] = c;
                if (c > 0 && c < OSD_MAX_CHARS && !m_bmpChar[c].isNull()) {
                    m_pCharLabel[y][x]->setPixmap(m_bmpChar[c]);
                    m_pCharLabel[y][x]->show();
                } else {
                    m_pCharLabel[y][x]->hide();
                }
            }
        }
    }
}

void OsdGridWidget::mouseMoveEvent(QMouseEvent *event)
{
    int col = event->pos().x() / m_cellW;
    int row = event->pos().y() / m_cellH;

    if (row >= 0 && row < FCOSD_MAX_HEIGHT && col >= 0 && col < FCOSD_MAX_WIDTH) {
        unsigned short val = m_charMap[row][col];
        emit cellHovered(row, col, val);
        QString ch;
        if (val >= 0x20 && val <= 0x7E) {
            ch = QString(" '%1'").arg(QChar(val));
        } else if (val != 0) {
            ch = QString(" U+%1").arg(val, 4, 16, QChar('0'));
        }
        setToolTip(QString("行:%1 列:%2  索引:%3 (0x%4)%5")
                      .arg(row).arg(col).arg(val).arg(val, 4, 16, QChar('0'))
                      .arg(ch));
    }
    QWidget::mouseMoveEvent(event);
}
