#include "filltextedit.h"

FillTextEdit::FillTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    // 允许在布局中自由拉伸
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(0, 0); // 消除基于内容的最小尺寸约束
}
