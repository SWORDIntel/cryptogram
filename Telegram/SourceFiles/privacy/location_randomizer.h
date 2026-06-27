#pragma once

#include <QtCore/QObject>

class LocationRandomizer : public QObject {
    Q_OBJECT
public:
    explicit LocationRandomizer(QObject *parent = nullptr) : QObject(parent) {}
    ~LocationRandomizer() = default;

    void setRandomizationRadius(int radiusKm) { _radiusKm = radiusKm; }
    int randomizationRadius() const { return _radiusKm; }

private:
    int _radiusKm = 0;
};
