#ifndef FILLPLAINTEXTEDIT_H
#define FILLPLAINTEXTEDIT_H

#include <QPlainTextEdit>

// 可随布局自由拉伸的 QPlainTextEdit：
// 关闭 heightForWidth，使控件高度完全由布局（如 splitter）分配，
// 实现"框与分隔条同步拉伸"。
class FillPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit FillPlainTextEdit(QWidget *parent = nullptr);

    bool hasHeightForWidth() const override { return false; }

    // 兼容 QTextEdit 风格的调用（便于从 QTextEdit 平滑迁移）
    void setText(const QString &text) { setPlainText(text); }
    void append(const QString &text) { appendPlainText(text); }

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // FILLPLAINTEXTEDIT_H
