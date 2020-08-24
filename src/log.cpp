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
#include <future>
#include "toast.h"

using namespace std;

Log::~Log(){

}

QString Log::getLine(int num) {
    return getLine(num, num);
}

SearchResult Log::search(const QString &text, QTextDocument::FindFlags flag, int fromLine, int fromPos, LongtimeOperation& op)
{
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
                return ret;
            }
            fromPos = 0;
            ++fromLine;
        }
    }

    return {0,0};
}

SubLog *Log::createSubLog(const QString &text, bool caseSensitive, LongtimeOperation& op)
{
    SubLog* sub = new SubLog(this);

    op.from = 1;
    op.to = lineCount();
    for (op.cur = op.from; op.cur <= op.to; op.cur++) {
        if (op.terminate) {
            delete sub;
            return nullptr;
        }

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

    mSize = mFile->size();
    if (mSize <= 0) {
        qDebug()<<"file is empty";
        mFile->close();
        delete mFile;
        mFile = nullptr;
        return false;
    }

    auto mem = mFile->map(0, mSize, QFileDevice::MapPrivateOption);
    //由于map出的内存是不带\0的，所以可能会导致越界访问
    //一个临时的修复方法是把最后一个字节改为\0
    mem[mSize-1]='\0';
    mMem = (char*)(mem);

    QTime time;
    time.restart();
    mEnters.push_back(-1);//插入“第0行”方便处理。假设第0行的行结束标记在-1字节


    //分割为一行一行；MacBook 2005 2.4G（已被）耗时3.2s左右
    mEnterCharOffset = 2;
    auto firstNewLine = strchr(mMem, '\n');
    if (firstNewLine != nullptr) {
        if (firstNewLine == mMem || *(firstNewLine - 1) != '\r') {
            mEnterCharOffset = 1;
        }

        --mEnterCharOffset;
        splitLines(op);
    }

    if (op.terminate)
        return false;

    if (mEnters.last() < mSize) {//没有\n的最后一行
        qDebug()<<"add last line";
        if (mSize >=2 && mMem[mSize-2]=='\r') {//\r\0
            mEnters.push_back(mSize - 2);
        } else {
            mEnters.push_back(mSize - 1);
        }
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

    return mEnters[num - 1] + 1 + mEnterCharOffset;
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

int FileLog::lineFromPos(qint64 pos, int from)
{
    while (from <= mLineCnt) { // pos => line，可能还能再优化？
        if (mEnters[from] >= pos) {
            return from;
        }
        ++from;
    }

    return -1;
}

//这里我们尝试以500M为块，作多线程加速
void FileLog::splitLines(LongtimeOperation& op)
{
    const qint64 blockSize = 524288000; //500M
    int extraParts = mSize / blockSize;

    if (mSize - extraParts * blockSize == 0) {//临界情况，刚好是500M的整数倍
        extraParts -= 1;
    }

    qDebug()<<"open using extra thread count: "<<extraParts;

    op.cur = 0;
    op.from = 1;
    op.to = mSize/100;//按一行100个字符估计总行数

    auto* extraEnters = new QVector<qint64>[extraParts];
    auto* extraRets = new future<void>[extraParts];
    auto* chBackup = new char[extraParts];
    for (int i = 0; i < extraParts; i++) {
        qint64 pos = blockSize * (i + 1);
        QVector<qint64>* enters = &extraEnters[i];
        enters->clear();

        //临时把mMem[pos]修改为0，方便分段处理
        chBackup[i] = mMem[pos];
        if (chBackup[i] == '\n') {
            enters->push_back(i - mEnterCharOffset);
        }
        mMem[pos] = 0;

        extraRets[i] = async(launch::async, [&, enters]{
            splitLine(op, enters, mMem + pos + 1, false);
        });
    }

    splitLine(op, &mEnters, mMem, true);//FIXME:extraParts>0的时候进度统计的有问题

    for (int i = 0; i < extraParts; i++) {
        extraRets[i].wait();

        //恢复mMem[pos]
        qint64 pos = blockSize * (i + 1);
        mMem[pos] = chBackup[i];

        //合并结果
        mEnters.append(extraEnters[i]);
        QVector<qint64>().swap(extraEnters[i]);//提前释放内存
    }

    delete[] extraEnters;
    delete[] extraRets;
    delete[] chBackup;
}

void FileLog::splitLine(LongtimeOperation& op, QVector<qint64>* enters, char *ptr, bool progress)
{
    while (!op.terminate && (ptr=strchr(ptr, '\n')) != nullptr) {
        enters->push_back(ptr - mMem - mEnterCharOffset);//定位在\r（如有）或\n上
        if (progress)
            ++op.cur;
        ++ptr;
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

    auto textInRaw = mCodec->fromUnicode(text).toStdString();
    SubLog* sub = new SubLog(this);

#if defined (Q_OS_MACOS) || defined (__MINGW32__)
    typedef char*(*StrstrFunc)(const char*,const char*);
#else
    typedef const char*(*StrstrFunc)(const char*,const char*);
#endif
    StrstrFunc strstrFunc = (StrstrFunc)strstr;
    if (!caseSensitive) {
        strstrFunc = (StrstrFunc)strcasestr;
    }

    const char* ptr = mMem;
    op.from = 1;
    op.to = mLineCnt;
    op.cur = 1;
    while (!op.terminate && (ptr = strstrFunc(ptr, textInRaw.c_str())) != nullptr) {
        op.cur = lineFromPos(ptr - mMem, op.cur);
        sub->addLine(op.cur);
        if (op.cur < op.to) {
            ptr = mMem + mEnters[op.cur];//直接到下一行
            op.cur++;
        }
        else {
            break;
        }
    }

    if (op.terminate) {
        delete sub;
        return nullptr;
    }

    qDebug()<<"create sub cost "<<time.elapsed();

    return sub;
}

SearchResult FileLog::search(const QString &text, QTextDocument::FindFlags flag, int fromLine, int fromPos, LongtimeOperation &op)
{
    if (flag.testFlag(QTextDocument::FindBackward)) {//对于向后查找，我们需要str[r]casestr来作优化
        return Log::search(text, flag, fromLine, fromPos, op);
    }

    const char* ptr = mMem + getLineStart(fromLine) + fromPos;

#if defined (Q_OS_MACOS) || defined (__MINGW32__)
    typedef char*(*StrstrFunc)(const char*,const char*);
#else
    typedef const char*(*StrstrFunc)(const char*,const char*);
#endif
    StrstrFunc strstrFunc = (StrstrFunc)strstr;
    if (!flag.testFlag(QTextDocument::FindCaseSensitively)) {
        strstrFunc = (StrstrFunc)strcasestr;
    }

    auto textInRaw = mCodec->fromUnicode(text).toStdString();
    op.from = fromLine;
    op.to = mLineCnt;
    op.cur = fromLine;
    ptr = strstrFunc(ptr, textInRaw.c_str());//坏处是没法中途停下来……
    SearchResult ret{0,0};
    if (ptr == nullptr) {
        return ret;
    }

    auto offset = ptr - mMem;
    ret.line = lineFromPos(offset);
    ret.pos = offset - getLineStart(ret.line) + text.length();
    op.cur = op.to;

    return ret;
}


SubLog::SubLog(Log *parent)
    :mParent(parent) {

}

void SubLog::addLine(int lineNum) {
    mLines.append(lineNum);
}

QString SubLog::getLine(int from, int to) {
    if (from == to) {
        return mParent->getLine(toParentLine(from));
    }

    QStringList list;
    while (from <= to) {
        list.append(mParent->getLine(toParentLine(from)));
        ++from;
    }

    return list.join("\n");
}

int SubLog::toParentLine(int lineNum)
{
    return mLines[lineNum - 1];
}

int SubLog::fromParentLine(int lineNum)
{
    auto it = lower_bound(mLines.begin(), mLines.end(), lineNum);
    if (it == mLines.end()) {
        return -1;
    }

    return distance(mLines.begin(), it)+1;
}
