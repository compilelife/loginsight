#include <QtGlobal>
#ifdef Q_OS_LINUX
#define _GNU_SOURCE //strcasestr in linux
#endif

#include <string.h>

#ifdef Q_OS_WIN
extern "C" const char *strcasestr(const char *haystack, const char *needle);
#endif

#include "log.h"
#include <QDebug>
#include <QByteArray>
#include <QTime>
#include <regex>
#include <QTextCodec>

using namespace std;

Log::~Log(){

}

QString Log::getLine(int num) {
    return getLine(num, num);
}

SearchResult Log::search(const QString &text, QTextDocument::FindFlags flag, int fromLine, int fromPos, LongtimeOperation& op)
{
    QTime time;
    time.start();

    //因为文本里可能有汉字，要定位关键字出现在行里的位置，用strstr定位受编码影响，会定位偏。
    //最简单的做法是直接用QString的indexOf查找好位置返回出去
    auto caseSensitivity = Qt::CaseInsensitive;
    if (flag.testFlag(QTextDocument::FindCaseSensitively)) {
        caseSensitivity = Qt::CaseSensitive;
    }

    SearchResult ret{0,0};

    if (flag.testFlag(QTextDocument::FindBackward)) {
        if (fromPos > 0) {
            fromPos -= 1;//避免连续搜索的时候搜索到自己
        }
        op.from = fromLine;
        op.to = 1;
        while (fromLine >= 1 && !op.terminate) {
            op.cur = fromLine;
            ret.line = fromLine;
            auto cur = getLine(fromLine);
            ret.pos = cur.lastIndexOf(text, fromPos, caseSensitivity);
            if (ret.pos >= 0) {
                return ret;
            }
            fromPos = -1;
            --fromLine;
        }
    } else {
        op.from = fromLine;
        op.to = lineCount();
        while(fromLine <= lineCount() && !op.terminate) {
            op.cur = fromLine;
            ret.line = fromLine;
            auto cur = getLine(fromLine);
            ret.pos = cur.indexOf(text, fromPos, caseSensitivity);
            if (ret.pos >= 0) {
                ret.pos += text.length();
                qDebug()<<"search cost: "<<time.elapsed();
                return ret;
            }
            fromPos = 0;
            ++fromLine;
        }
    }

    return ret;
}

SubLog *Log::createSubLog(const QString &text, bool caseSensitive, LongtimeOperation& op)
{
    SubLog* sub = new SubLog(this);

    op.from = 1;
    op.to = lineCount();
    for (op.cur = op.from; op.cur <= op.to; op.cur++) {
        if (op.terminate)
            break;

        auto s = getLine(op.cur);
        if (s.indexOf(text, 0, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive) >= 0) {
            sub->addLine(op.cur);
        }
    }

    return sub;
}

FileLog::FileLog() {

}

FileLog::~FileLog() {
    if (mFile) {
        delete mFile;
    }
}

bool FileLog::open(const QString &path, LongtimeOperation& op) {
    if (mFile) {
        close();
    }

    mFile = new QFile(path);
    if (!mFile->open(QIODevice::ReadOnly)) {
        qDebug()<<"failed to open file as read only";
        delete mFile;
        mFile = nullptr;
        return false;
    }

    mMem = (const char*)(mFile->map(0, mFile->size()));
    mSize = mFile->size();

    QTime time;
    time.restart();
    mEnters.push_back(-1);//插入“第0行”方便处理。假设第0行的行结束标记在-1字节

    op.cur = 0;
    op.from = 1;
    op.to = mSize/100;//按一行100个字符估计总行数

    //分割为一行一行；MacBook 2005 2.4G耗时3.2s左右
    const char* ptr = mMem;
    const char* pEnd = mMem + mSize;

    int enterCharOffset = 2;
    auto firstNewLine = strchr(mMem, '\n');
    if (firstNewLine != nullptr) {
        if (firstNewLine == mMem || *(firstNewLine - 1) != '\r') {
            enterCharOffset = 1;
        }

        --enterCharOffset;
        while (ptr < pEnd && (ptr=strchr(ptr, '\n')) != nullptr) {
            mEnters.push_back(ptr - mMem - enterCharOffset);//定位在\r（如有）或\n上
            ++op.cur;
            ++ptr;
        }
    }

    if (ptr < pEnd) {//没有\n的最后一行
        qDebug()<<"add last line:"<<(pEnd - ptr);
        mEnters.push_back(mSize);
    }

    qDebug()<<"create enters cost "<<time.elapsed();//2G+release: 5s
    //TODO mEnters记录到工程文件中，下次秒开

    mLineCnt = mEnters.size()-1;

    detectTextCodec();

    return true;
}

void FileLog::close() {
    if (mFile) {
        mEnters.clear();
        mFile->unmap((uchar*)mMem);
        mFile->close();
        delete mFile;
        mFile = nullptr;
    }
}

qint64 FileLog::getLineStart(int num) {
    if (num == 1) {
        return 0;
    }

    auto lastLineEndPos = mEnters[num - 1];
    auto p = mMem + lastLineEndPos;

    if (*p == '\r')
        return lastLineEndPos+2;

    return lastLineEndPos+1;
}

void FileLog::detectTextCodec()
{
    //先只判断两种类型：UTF-8/GBK

    QTextCodec::ConverterState state;
    auto utf8 = QTextCodec::codecForName("UTF-8");
    utf8->toUnicode(mMem, 4096, &state);//取4k来判断
    if (state.invalidChars > 0) {
        setCodec(QTextCodec::codecForName("GBK"));
    } else {
        setCodec(utf8);
    }
}

QString FileLog::getLine(int from, int to) {
    if (from <= 0 || from > mLineCnt) {
        return "";
    }

    if (to > mLineCnt) {
        to = mLineCnt;
    }

    auto start = getLineStart(from);
    auto ba = QByteArray::fromRawData(mMem + start, (int)(mEnters[to] - start));
    return mCodec->toUnicode(ba);
}

SubLog *FileLog::createSubLog(const QString &text, bool caseSensitive, LongtimeOperation& op)
{
    //Log的默认实现太慢了，直接操作内存块会快很多。
    //2G文件（已cache）在2015 Macbook上耗时只有默认实现的1/11 (3500~5000 ms vs 58255 ms)
    //release会更低

    //TODO:记录到工程文件中，下次秒载。记录过去2次结果

    QTime time;
    time.start();

    auto ps = mCodec->fromUnicode(text).toStdString().c_str();
    SubLog* sub = new SubLog(this);

    op.from = 1;
    op.to = lineCount();
    op.cur = op.from;

#ifdef Q_OS_MACOS
    typedef char*(*StrstrFunc)(const char*,const char*);
#else
    typedef const char*(*StrstrFunc)(const char*,const char*);
#endif
    StrstrFunc strstrFunc = (StrstrFunc)strstr;
    if (!caseSensitive) {
        strstrFunc = (StrstrFunc)strcasestr;
    }

    const char* ptr = mMem;
    while ((ptr = strstrFunc(ptr, ps)) != nullptr) {
        auto pos = ptr - mMem;
        while (op.cur <= op.to) { // pos => line，可能还能再优化？
            if (mEnters[op.cur] >= pos) {
                sub->addLine(op.cur);
                if (op.cur != op.to)
                    ptr = mMem + mEnters[op.cur+1];//直接到下一行
                else
                    ptr = nullptr;
                break;
            }
            ++op.cur;
        }
    }

    qDebug()<<"create sub cost "<<time.elapsed();

    return sub;
}


SubLog::SubLog(Log *parent)
    :mParent(parent) {

}

void SubLog::addLine(int lineNum) {
    mLines.append(lineNum);
}

QString SubLog::getLine(int from, int to) {
    if (from == to) {
        return mParent->getLine(sourceLine(from));
    }

    QStringList list;
    while (from <= to) {
        list.append(mParent->getLine(sourceLine(from)));
        ++from;
    }

    return list.join("\n");
}

int SubLog::sourceLine(int lineNum)
{
    return mLines[lineNum - 1];
}
