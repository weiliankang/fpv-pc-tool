#ifndef FILLTEXTEDIT_H
#define FILLTEXTEDIT_H

#include <QTextEdit>

// 可随布局自由拉伸的 QTextEdit：
// QTextEdit 默认 hasHeightForWidth()==true，导致其在 QVBoxLayout 中
// 高度被内容锁死（框拉到一定高度就不再变高）。
// 此子类关闭 heightForWidth，使控件完全由布局（如 splitter）分配高度，
// 实现"框与 splitter 分隔条同步拉伸"。
class FillTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit FillTextEdit(QWidget *parent = nullptr);

    bool hasHeightForWidth() const override { return false; }
};

#endif // FILLTEXTEDIT_H
