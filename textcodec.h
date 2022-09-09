#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <QTextCodec>
#include <QObject>

class TextCodec : public QObject
{
    Q_OBJECT

public:
    TextCodec();
    QByteArray toLog(QString text);
    QString toVisual(QByteArray arr);

public:
    Q_PROPERTY(QString name READ getName WRITE setName)
    //将界面上的文本，特指搜索、过滤用的关键字（要用于日志比对）
    //转为对应编码下的字节（否则QString是Unicode存储的），顺带base64避免json传输乱码
    Q_INVOKABLE QString toLogByte(QString text);
    //将日志里的文本，按指定编码，转为界面上可读的文本（顺带解码base64）
    Q_INVOKABLE QString toVisualByte(QString encodeStr);
    Q_INVOKABLE int toLogOffset(QString str, int offset);
public:
    void setName(QString name);
    QString getName();
private:
    QTextCodec* mCodec;
};

#endif // TEXTCODEC_H
