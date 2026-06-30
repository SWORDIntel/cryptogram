#pragma once

#include <QtCore/QObject>

namespace SpyGram::Security {

class GnaSurveillanceEngine : public QObject {
    Q_OBJECT
public:
    explicit GnaSurveillanceEngine(QObject *parent = nullptr) : QObject(parent) {}
    void start() {}
    void stop() {}
    bool isActive() const { return false; }
};

} // namespace SpyGram::Security
