#include "fillplaintextedit.h"
#include <QDebug>

FillPlainTextEdit::FillPlainTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(0, 0); // 消除基于内容的最小尺寸约束，允许完全由布局分配
}

void FillPlainTextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    qDebug() << "[FillPlainTextEdit] resize:" << e->size();
}
