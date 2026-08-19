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
    // 允许随布局等比例缩放铺满（QLabel 网格在缩放模式下会被隐藏，改由 QPainter 统一绘制）
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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

    if (m_scaleToFit) {
        // 缩放铺满：不锁死最小尺寸，允许自由伸缩
        setMinimumSize(0, 0);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    } else {
        setMinimumSize(FCOSD_MAX_WIDTH * m_cellW, FCOSD_MAX_HEIGHT * m_cellH);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
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
    // SDK 字体实际字符高度: 720p=36, 1080p=54 (sysenv.cpp default_osdfont)
    int fontH = use1080 ? 54 : 36;

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
    // 外边距背景（纯黑）——保证黑框 53:20 等比居中，四周留黑边不被拉伸
    p.fillRect(rect(), QColor(0x00, 0x00, 0x00));

    // ---- 缩放铺满模式：黑框保持 53:20 纵横比，等比例填满可用空间 ----
    // 此模式下隐藏 QLabel 网格，统一用 QPainter 等比例缩放绘制字符位图，
    // 数据流(m_charMap/updateOSDGrid)不变，只改渲染路径。
    if (m_scaleToFit) {
        // 隐藏 QLabel 网格（避免它们以固定尺寸叠加在缩放绘制之上）
        for (int y = 0; y < FCOSD_MAX_HEIGHT; y++)
            for (int x = 0; x < FCOSD_MAX_WIDTH; x++)
                m_pCharLabel[y][x]->hide();
        paintScaled(p);
        return;
    }

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
    // 缩放铺满模式下 QLabel 被隐藏，需显式触发 QPainter 重绘（否则要切界面才刷新）
    if (m_scaleToFit) update();
}

void OsdGridWidget::setScaleToFit(bool on)
{
    if (m_scaleToFit == on) return;
    m_scaleToFit = on;
    if (on) {
        setMinimumSize(0, 0);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    } else {
        setMinimumSize(FCOSD_MAX_WIDTH * m_cellW, FCOSD_MAX_HEIGHT * m_cellH);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    updateGeometry();
    update();
}

void OsdGridWidget::setAttitudeBarEnabled(bool on)
{
    m_attitudeBarEnabled = on;
    update();
}

void OsdGridWidget::setAttitudeBar(const int left[4], const int right[4])
{
    for (int i = 0; i < 4; i++) m_attLeft[i]  = left[i];
    for (int i = 0; i < 4; i++) m_attRight[i] = right[i];
    update();
}

void OsdGridWidget::clearAttitudeBar()
{
    for (int i = 0; i < 4; i++) { m_attLeft[i] = -5; m_attRight[i] = -5; }
    update();
}

// ============================================================
// 缩放铺满绘制：黑框保持 53:20 纵横比，等比例填满可用空间。
// 无论有无字体，都统一用 QPainter 绘制：
//   - 有字体：把 m_bmpChar[索引] 位图缩放到当前格子大小
//   - 无字体：回退绘制文字字符
// 数据流(m_charMap)不变，只改渲染路径。始终保留深色黑底。
// ============================================================
void OsdGridWidget::paintScaled(QPainter &p)
{
    const int availW = width();
    const int availH = height();

    // 网格逻辑尺寸（基于单元格像素）
    const double gridW = (double)FCOSD_MAX_WIDTH * m_cellW;
    const double gridH = (double)FCOSD_MAX_HEIGHT * m_cellH;

    // 等比例缩放：取宽高比更紧的那个，锁 53:20 纵横比
    double scale = qMin(availW / gridW, availH / gridH);
    if (scale <= 0) scale = 1.0;

    // 实际绘制区域（居中，保持 53:20 纵横比）
    int drawW = qRound(gridW * scale);
    int drawH = qRound(gridH * scale);
    int offX = (availW - drawW) / 2;
    int offY = (availH - drawH) / 2;

    // OSD 屏幕区底色（黑框内部）——区别于外边距纯黑
    p.fillRect(offX, offY, drawW, drawH, QColor(0x0A, 0x0A, 0x0A));

    // 统一的单元格缩放尺寸
    const int cellW = qMax(1, qRound(m_cellW * scale));
    const int cellH = qMax(1, qRound(m_cellH * scale));

    // 有字体：逐格缩放绘制位图
    if (m_hasFont && m_numChars > 0) {
        for (int row = 0; row < FCOSD_MAX_HEIGHT; ++row) {
            for (int col = 0; col < FCOSD_MAX_WIDTH; ++col) {
                unsigned short c = m_charMap[row][col];
                // 与 updateOSDGrid 完全一致的索引边界处理
                if (c >= m_numChars)
                    c = m_numChars > 0 ? m_numChars - 1 : 0;
                if (c == 0 || c >= OSD_MAX_CHARS) continue;
                const QPixmap &bmp = m_bmpChar[c];
                if (bmp.isNull()) continue;
                int x = offX + col * cellW;
                int y = offY + row * cellH;
                p.drawPixmap(QRect(x, y, cellW, cellH), bmp, bmp.rect());
            }
        }
    } else {
    QFont font("Consolas", -1, QFont::Normal);
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPixelSize(qMax(6, qRound(14 * scale)));
    p.setFont(font);
    QFontMetrics fm(font);
    int textCharW = fm.horizontalAdvance('W');
    int textCharH = fm.height();

    for (int row = 0; row < FCOSD_MAX_HEIGHT; ++row) {
        for (int col = 0; col < FCOSD_MAX_WIDTH; ++col) {
            unsigned short val = m_charMap[row][col];
            if (val == 0) continue;
            int x = offX + col * cellW;
            int y = offY + row * cellH;
            int cx = x + (cellW - textCharW) / 2;
            int cy = y + (cellH + textCharH) / 2 - fm.descent();
            unsigned short lo = val & 0xFF;
            if (lo >= 0x20 && lo <= 0x7E) {
                if (lo >= '0' && lo <= '9') p.setPen(QColor(0x00, 0xFF, 0x88));
                else if (lo >= 'A' && lo <= 'Z') p.setPen(QColor(0xFF, 0xFF, 0xFF));
                else if (lo >= 'a' && lo <= 'z') p.setPen(QColor(0xAA, 0xCC, 0xFF));
                else p.setPen(QColor(0xCC, 0xCC, 0xCC));
                p.drawText(cx, cy, QChar(lo));
            } else if (lo > 0x7E) {
                p.setPen(QColor(0x88, 0xFF, 0x88));
                p.drawText(cx, cy, QString("0x%1").arg(lo, 2, 16, QChar('0')));
            } else {
                p.setPen(QColor(0x88, 0x88, 0x88));
                p.drawText(cx, cy, QChar(lo));
            }
        }
    }
    } // end else (无字体回退)

    // ---- 姿态条图形化叠加层（F0 读取页 R10 姿态区可视化）----
    if (m_attitudeBarEnabled) {
        const int row = 10; // R10 姿态行
        int baseY = offY + row * cellH + cellH / 2;
        const int leftStartX  = offX + 21 * cellW + cellW / 2;
        const int rightStartX = offX + 28 * cellW + cellW / 2;

        p.setPen(QPen(QColor(0xFF, 0xD7, 0x00), qMax(1, qRound(2 * scale))));
        for (int i = 0; i < 4; i++) {
            if (m_attLeft[i] < -4 || m_attLeft[i] > 4) continue;
            int x = leftStartX + i * cellW;
            int y = baseY - m_attLeft[i] * (cellH / 5);
            p.drawLine(x - cellW / 4, y, x + cellW / 4, y);
        }
        for (int i = 0; i < 4; i++) {
            if (m_attRight[i] < -4 || m_attRight[i] > 4) continue;
            int x = rightStartX + i * cellW;
            int y = baseY - m_attRight[i] * (cellH / 5);
            p.drawLine(x - cellW / 4, y, x + cellW / 4, y);
        }
        p.setPen(QPen(QColor(0x44, 0x88, 0xFF), 1, Qt::DotLine));
        p.drawLine(leftStartX - cellW / 2, baseY, rightStartX + 4 * cellW - cellW / 2, baseY);
    }

    // 黑框边框——体现 53:20 等比边界（随缩放，始终保持比例）
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(QColor(0x28, 0x28, 0x28), qMax(1, qRound(2 * scale))));
    p.drawRect(offX, offY, drawW, drawH);
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
