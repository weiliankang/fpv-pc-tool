#include "crsfchannelwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <cstring>
#include <cmath>

CrsfChannelWidget::CrsfChannelWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(250);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_updateTimer.start();

    // Set default channel values to midpoint
    for (int i = 0; i < 16; ++i) {
        m_channels[i] = PWM_MID;
        m_prevChannels[i] = PWM_MID;
    }
}

void CrsfChannelWidget::updateChannels(const uint16_t channels[16])
{
    memcpy(m_prevChannels, m_channels, sizeof(m_channels));
    bool changed = false;

    for (int i = 0; i < 16; ++i) {
        m_channels[i] = channels[i];
        // Add to history
        if (m_history[i].size() >= m_historyDepth)
            m_history[i].removeFirst();
        m_history[i].append(channels[i]);
        if (m_channels[i] != m_prevChannels[i])
            changed = true;
    }

    if (changed) {
        m_lastUpdateUs = m_updateTimer.nsecsElapsed() / 1000;
        update();
    }
}

void CrsfChannelWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    QRect r = rect().adjusted(2, 2, -2, -2);
    drawBackground(p, r);

    int usableWidth = r.width() - LABEL_WIDTH - 10;
    int columnCount = 8; // 2 columns of 8 channels
    int columnWidth = usableWidth / columnCount;
    int chHeight = BAR_HEIGHT + BAR_GAP;

    for (int ch = 0; ch < 16; ++ch) {
        int col = ch / 8;        // 0 = left, 1 = right
        int row = ch % 8;        // 0-7 per column

        int x = r.x() + LABEL_WIDTH + col * columnWidth;
        int y = r.y() + row * chHeight;
        int w = columnWidth - 4;

        QRect barRect(x, y, w, BAR_HEIGHT);

        // Determine previous frame's value for this channel
        uint16_t prevVal = m_history[ch].size() >= 2
            ? m_history[ch][m_history[ch].size() - 2]
            : m_channels[ch];

        drawChannelBar(p, ch, m_channels[ch], prevVal, barRect, columnWidth);
    }

    // Draw header labels
    p.setPen(QColor(180, 180, 180));
    QFont hf = p.font();
    hf.setPointSize(7);
    hf.setBold(true);
    p.setFont(hf);
    QFontMetrics hfm(hf);
    int headerY = r.y() - 1;
    int labelH = hfm.height();
    for (int col = 0; col < 2; ++col) {
        int hx = r.x() + LABEL_WIDTH + col * columnWidth;
        int hw = columnWidth - 4;
        // Label at left, center value marker at right
        QString hdrText = QString("CH1-%1").arg(col == 0 ? "8" : "16");
        int hdrW = hfm.width(hdrText) + 6;
        p.fillRect(hx + 2, headerY - labelH, hdrW, labelH, QColor(50, 55, 60));
        p.drawText(hx + 4, headerY - 2, hdrText);

        // Center marker
        QString cntrText = "◄1000    1500    2012►";
        int cntrW = hfm.width(cntrText);
        p.setPen(QColor(100, 100, 100));
        p.drawText(hx + hw - cntrW - 4, headerY - 2, cntrText);
        p.setPen(QColor(180, 180, 180));
    }
}

void CrsfChannelWidget::drawBackground(QPainter &p, const QRect &rect)
{
    p.fillRect(rect, QColor(32, 32, 32));

    // Channel label area background
    p.fillRect(rect.x(), rect.y(), LABEL_WIDTH - 1, rect.height(), QColor(40, 40, 40));
}

void CrsfChannelWidget::drawChannelBar(QPainter &p, int ch, uint16_t value, uint16_t prevValue,
                                        const QRect &barRect, int chWidth)
{
    Q_UNUSED(chWidth);
    int barLeft = barRect.x();
    int barWidth = barRect.width();

    // Normalized position: 0.0 (988us) to 1.0 (2012us)
    double norm = static_cast<double>(value - PWM_MIN) / PWM_RANGE;
    double prevNorm = static_cast<double>(prevValue - PWM_MIN) / PWM_RANGE;

    int pixelPos = barLeft + static_cast<int>(norm * barWidth);
    int prevPixelPos = barLeft + static_cast<int>(prevNorm * barWidth);

    // Clip to widget
    pixelPos = qBound(barLeft, pixelPos, barLeft + barWidth);
    prevPixelPos = qBound(barLeft, prevPixelPos, barLeft + barWidth);

    // --- Draw ghost (previous value line) ---
    if (m_showGhost) {
        p.setPen(QPen(QColor(120, 120, 120, 80), 2));
        p.drawLine(prevPixelPos, barRect.y(), prevPixelPos, barRect.y() + BAR_HEIGHT);
    }

    // --- Draw filled bar ---
    // Color gradient: center bar is green, left/right are orange/red
    QColor barColor;
    if (norm < 0.45) {
        // Low end: red-ish
        double t = norm / 0.45;
        barColor = QColor(
            static_cast<int>(120 + 135 * t),
            static_cast<int>(30 + 170 * t),
            40
        );
    } else if (norm <= 0.55) {
        // Center range: green
        barColor = QColor(50, 200, 50);
    } else {
        // High end: red
        double t = (norm - 0.55) / 0.45;
        if (t > 1.0) t = 1.0;
        barColor = QColor(
            static_cast<int>(50 + 200 * t),
            static_cast<int>(200 - 170 * t),
            50
        );
    }

    // Bar from left edge to current position
    int paddedTop = barRect.y() + 1;
    int paddedHeight = BAR_HEIGHT - 2;
    int barEndX = qMax(barLeft + 2, pixelPos);
    QRect fillRect(barLeft + 1, paddedTop, barEndX - barLeft - 1, paddedHeight);
    p.fillRect(fillRect, barColor);

    // --- Draw center line inside each bar (1500us) ---
    int ctrX = barLeft + static_cast<int>(0.5 * barWidth);
    p.setPen(QPen(QColor(255, 255, 255, 40), 1));
    p.drawLine(ctrX, paddedTop, ctrX, paddedTop + paddedHeight);

    // --- Draw border of the bar ---
    p.setPen(QPen(QColor(80, 80, 80), 1));
    p.drawRect(barLeft, paddedTop, barWidth, paddedHeight);

    // --- Draw channel label + value ---
    QFont f = p.font();
    f.setPointSize(8);
    p.setFont(f);
    QFontMetrics fm(f);

    // Channel number on the left label area
    p.setPen(Qt::white);
    QRect labelRect(barRect.x() - LABEL_WIDTH + 2, barRect.y(), LABEL_WIDTH - 6, BAR_HEIGHT);
    p.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter,
               QString("CH%1").arg(ch + 1, 2));

    // Value text on the bar (right-aligned)
    QString valStr = QString::number(value);
    int tw = fm.horizontalAdvance(valStr) + 4;
    QRect valRect(qMax(barLeft + 2, pixelPos - tw), paddedTop, tw, paddedHeight);
    if (norm < 0.5)
        p.setPen(Qt::white);
    else
        p.setPen(QColor(32, 32, 32));
    p.drawText(valRect, Qt::AlignRight | Qt::AlignVCenter, valStr);

    // --- Draw offset from center ---
    int offset = static_cast<int>(value) - PWM_MID;
    if (qAbs(offset) >= 1) {
        QString offsetStr;
        if (offset > 0) {
            offsetStr = QString("+%1").arg(offset);
            p.setPen(QColor(255, 200, 100));
        } else {
            offsetStr = QString::number(offset);
            p.setPen(QColor(100, 200, 255));
        }
        int ow = fm.horizontalAdvance(offsetStr) + 2;
        QRect offsetRect(barLeft + barWidth - ow - 1, paddedTop, ow, paddedHeight);

        // Determine if text would overlap too much with value
        if (valRect.right() < offsetRect.left() - 10) {
            // Semi-transparent background
            p.fillRect(offsetRect, QColor(0, 0, 0, 100));
            p.drawText(offsetRect, Qt::AlignCenter, offsetStr);
        }
    }
}
