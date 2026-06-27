#pragma once

#include <QtCore/QObject>
#include <memory>

class QuantumCryptoSystem : public QObject {
    Q_OBJECT
public:
    explicit QuantumCryptoSystem(QObject *parent = nullptr) : QObject(parent) {}
    ~QuantumCryptoSystem() = default;

    bool initialize() { return true; }
    bool isAvailable() const { return true; }
};
