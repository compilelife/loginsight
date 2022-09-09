#include "textcodec.h"
#include <QDebug>

TextCodec::TextCodec()
    :QObject()
{
    mCodec = QTextCodec::codecForLocale();
}

QByteArray TextCodec::toLog(QString text)
{
    return mCodec->fromUnicode(text);
}

QString TextCodec::toVisual(QByteArray arr)
{
    return mCodec->toUnicode(arr);
}

QString TextCodec::toLogByte(QString text)
{
    return mCodec->fromUnicode(text).toBase64();
}

QString TextCodec::toVisualByte(QString text)
{
    return mCodec->toUnicode(QByteArray::fromBase64(text.toLocal8Bit()));
}

int TextCodec::toLogOffset(QString str, int offset)
{
    return mCodec->fromUnicode(str.mid(0, offset)).length();
}

void TextCodec::setName(QString name)
{
    mCodec = QTextCodec::codecForName(name.toLocal8Bit());
}

QString TextCodec::getName()
{
    return mCodec->name();
}

