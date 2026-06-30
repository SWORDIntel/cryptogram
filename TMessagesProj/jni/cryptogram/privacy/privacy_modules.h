#pragma once

#include "desktop_shims.h"

class ContactObfuscator : public QObject {
public:
    explicit ContactObfuscator(QObject *parent = nullptr) : QObject(parent) {}
    ~ContactObfuscator() = default;

    void setObfuscationLevel(float level) { _level = level; }
    float obfuscationLevel() const { return _level; }

private:
    float _level = 0.0f;
};

class LocationRandomizer : public QObject {
public:
    explicit LocationRandomizer(QObject *parent = nullptr) : QObject(parent) {}
    ~LocationRandomizer() = default;

    void setRandomizationRadius(int radiusKm) { _radiusKm = radiusKm; }
    int randomizationRadius() const { return _radiusKm; }

private:
    int _radiusKm = 0;
};

class QuantumCryptoSystem : public QObject {
public:
    explicit QuantumCryptoSystem(QObject *parent = nullptr) : QObject(parent) {}
    ~QuantumCryptoSystem() = default;

    bool initialize() { return true; }
    bool isAvailable() const { return true; }
};
