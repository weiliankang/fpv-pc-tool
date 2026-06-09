#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QString>
#include <QMap>

class AppTranslator : public QObject
{
    Q_OBJECT
public:
    explicit AppTranslator(QObject *parent = nullptr);

    // 设置语言
    bool setLanguage(const QString &langCode);

    // tr(key) — 返回翻译文本，找不到返回 key
    QString tr(const QString &key) const;

    QString currentLang() const { return m_currentLang; }

signals:
    void languageChanged(const QString &langCode);

private:
    void initZh();
    void initEn();
    QMap<QString, QString> m_zh;
    QMap<QString, QString> m_en;
    QString m_currentLang;
    bool m_initialized = false;
};

#endif
