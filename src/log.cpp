#include "log.h"
#include <QDebug>
#include <QByteArray>
#include <QTime>
#include <regex>

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
    op.from = 0;
    op.to = mSize/100;//按一行100个字符估计总行数
    for (qint64 i = 0; i < mSize; ) {//TODO:进一步优化
        if (mMem[i] == '\r') {
            mEnters.push_back(i);
            i+=2;
            if (op.terminate) {
                return false;
            }
            ++op.cur;
        } else if (mMem[i] == '\n') {
            mEnters.push_back(i);
            ++i;
            if (op.terminate) {
                return false;
            }
            ++op.cur;
        } else {
            ++i;
        }
    }
    qDebug()<<"create enters cost "<<time.elapsed();//2G+release: 5s
    //TODO mEnters记录到工程文件中，下次秒开

    mLineCnt = mEnters.size()-1;

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

QString FileLog::getLine(int from, int to) {
    if (from <= 0 || from > mLineCnt) {
        return "";
    }

    if (to > mLineCnt) {
        to = mLineCnt;
    }

    auto start = getLineStart(from);
    return QByteArray::fromRawData(mMem + start, (int)(mEnters[to] - start));
}

SubLog *FileLog::createSubLog(const QString &text, bool /*caseSensitive*/, LongtimeOperation& op)
{
    //Log的默认实现太慢了，直接操作内存块会快很多。
    //2G文件在2015 Macbook上耗时只有默认实现的1/7 (8787 ms vs 58255 ms)
    //release可降到4s左右

    //TODO:记录到工程文件中，下次秒载。记录过去2次结果

    QTime time;
    time.start();

    auto s = text.toStdString();
    auto ps = s.c_str();
    SubLog* sub = new SubLog(this);

    op.from = 1;
    op.to = lineCount();
    for (op.cur = op.from; op.cur <= op.to; op.cur++) {
        if (op.terminate)
            break;

        if (strnstr(mMem + mEnters[op.cur-1]+1, ps, mEnters[op.cur]-mEnters[op.cur-1])) {
            sub->addLine(op.cur);
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
