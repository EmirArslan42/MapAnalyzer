#pragma once

#include <QString>

struct MemoryStats {
    double stackUsed = 0, stackTotal = 0;
    double flashUsed = 0, flashTotal = 0;
    double ramUsed   = 0, ramTotal   = 0;
};


bool parseMapFile(const QString &filePath, MemoryStats &stats);
