#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>

class ContactObfuscator : public QObject {
    Q_OBJECT
public:
    explicit ContactObfuscator(QObject *parent = nullptr) : QObject(parent) {}
    ~ContactObfuscator() = default;

    void setObfuscationLevel(float level) { _level = level; }
    float obfuscationLevel() const { return _level; }

private:
    float _level = 0.0f;
};
