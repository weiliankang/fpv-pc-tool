#ifndef CRSFCHANNELWIDGET_H
#define CRSFCHANNELWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPainter>
#include <QElapsedTimer>

class CrsfChannelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CrsfChannelWidget(QWidget *parent = nullptr);

    // Update all 16 channels (values in PWM microseconds, 988-2012)
    void updateChannels(const uint16_t channels[16]);

    // Get current channel values
    const uint16_t* channels() const { return m_channels; }

    // History depth for ghost overlay
    void setHistoryDepth(int depth) { m_historyDepth = qBound(1, depth, 20); }
    int historyDepth() const { return m_historyDepth; }

    // Toggle ghost overlay
    void setGhostEnabled(bool enabled) { m_showGhost = enabled; update(); }
    bool ghostEnabled() const { return m_showGhost; }

signals:
    void channelChanged(int channel, uint16_t valuePwm);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize minimumSizeHint() const override { return QSize(400, 250); }
    QSize sizeHint() const override { return QSize(600, 320); }

private:
    void drawBackground(QPainter &p, const QRect &rect);
    void drawChannelBar(QPainter &p, int ch, uint16_t value, uint16_t prevValue,
                        const QRect &barRect, int chWidth);

    uint16_t m_channels[16] = {0};
    uint16_t m_prevChannels[16] = {0};
    QVector<uint16_t> m_history[16]; // circular history for each channel

    bool m_showGhost = true;
    int m_historyDepth = 5;

    // Layout constants
    static constexpr int LABEL_WIDTH = 55;
    static constexpr int BAR_HEIGHT = 14;
    static constexpr int BAR_GAP = 2;
    static constexpr int PWM_MIN = 988;
    static constexpr int PWM_MID = 1500;
    static constexpr int PWM_MAX = 2012;
    static constexpr int PWM_RANGE = PWM_MAX - PWM_MIN; // 1024

    QElapsedTimer m_updateTimer;
    qint64 m_lastUpdateUs = 0;
};

#endif // CRSFCHANNELWIDGET_H
