﻿#ifndef LONGTIMEOPERATION_H
#define LONGTIMEOPERATION_H

struct LongtimeOperation {
    int from;
    int to;
    int cur;
    bool terminate{false};
};

#endif // LONGTIMEOPERATION_H
