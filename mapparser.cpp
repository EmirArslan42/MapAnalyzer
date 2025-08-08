#include "MapParser.h"
#include <QFile>
#include <QTextStream>

static int hexToInt(const QString &hexStr) {
    bool ok;
    return hexStr.toUInt(&ok, 16);
}

static double hexToKB(const QString &hexStr) {
    QString clean = hexStr.trimmed().remove("0x");
    bool ok;
    return clean.toUInt(&ok, 16) / 1024.0;
}

static int hexToByte(const QString &hexStr) {
    QString clean = hexStr.trimmed().remove("0x");
    bool ok;
    return clean.toUInt(&ok, 16);
}


bool parseMapFile(const QString &filePath, MemoryStats &stats) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QTextStream stream(&file);
    bool configSection = false;

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();

        if (line.startsWith("Memory Configuration")) {
            configSection = true;
            continue;
        }

        if (configSection && line.startsWith("Name")) {
            // Header satırı, bir sonraki satırlar veri
            continue;
        }

        if (configSection && !line.isEmpty()) {
            QStringList tokens = line.split(QRegExp("\\s+"));
            if (tokens.size() >= 5) {
                QString name = tokens[0];
                double used  = hexToByte(tokens[3]); // KB
                double total = hexToByte(tokens[2]); // KB

                if (name.contains("STACK", Qt::CaseInsensitive)) {
                    stats.stackUsed  = used;
                    stats.stackTotal = total;
                } else if (name.contains("FLASH", Qt::CaseInsensitive)) {
                    stats.flashUsed  = used;
                    stats.flashTotal = total;
                } else if (name.contains("RAM", Qt::CaseInsensitive)) {
                    stats.ramUsed    = used;
                    stats.ramTotal   = total;
                }
            }
        }

        if (line.startsWith("Linker script") || line.startsWith("Sections")) {
            configSection = false;
        }
    }

    return true;
}

