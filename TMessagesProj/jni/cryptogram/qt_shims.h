#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Simple Qt-like types for Android/NDK build
using QString = std::string;
using QByteArray = std::string;
template<typename T> using QVector = std::vector<T>;
template<typename K, typename V> using QMap = std::map<K, V>;
template<typename T> using QSet = std::set<T>;

// Basic QString enhancements
namespace QtShims {
    inline QString fromUtf8(const char* s) { return QString(s); }
    inline QString number(long long n) { return std::to_string(n); }
}

#define Q_DECLARE_METATYPE(x)

// QDateTime shim
struct QDateTime {
    long long msecs;
    static QDateTime currentDateTime() {
        auto now = std::chrono::system_clock::now();
        return { std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() };
    }
    static long long currentMSecsSinceEpoch() {
        return currentDateTime().msecs;
    }
    static long long currentSecsSinceEpoch() {
        return currentDateTime().msecs / 1000;
    }
    QDateTime addDays(int days) const {
        return { msecs + (long long)days * 24 * 60 * 60 * 1000 };
    }
    QString toString(int format = 0) const {
        return std::to_string(msecs);
    }
};

// JSON Shims (very basic, can be expanded or replaced with nlohmann/json)
struct QJsonValue {
    enum Type { Null, Bool, Double, String, Array, Object };
    Type _type = Null;
    std::string _string;
    double _double = 0;
    bool _bool = false;

    QJsonValue() {}
    QJsonValue(const char* s) : _type(String), _string(s) {}
    QJsonValue(const std::string& s) : _type(String), _string(s) {}
    QJsonValue(double d) : _type(Double), _double(d) {}
    QJsonValue(bool b) : _type(Bool), _bool(b) {}

    bool isString() const { return _type == String; }
    bool isDouble() const { return _type == Double; }
    bool isBool() const { return _type == Bool; }
    bool isObject() const { return _type == Object; }
    bool isArray() const { return _type == Array; }

    std::string toString() const { return _string; }
    double toDouble(double def = 0) const { return _type == Double ? _double : def; }
    int toInt(int def = 0) const { return _type == Double ? (int)_double : def; }
    bool toBool(bool def = false) const { return _type == Bool ? _bool : def; }
};

struct QJsonObject {
    std::map<std::string, QJsonValue> _map;
    bool contains(const std::string& key) const { return _map.find(key) != _map.end(); }
    QJsonValue operator[](const std::string& key) const {
        auto it = _map.find(key);
        return it != _map.end() ? it->second : QJsonValue();
    }
    QJsonValue value(const std::string& key, const QJsonValue& def = QJsonValue()) const {
        auto it = _map.find(key);
        return it != _map.end() ? it->second : def;
    }
    void insert(const std::string& key, const QJsonValue& value) { _map[key] = value; }
};

struct QJsonArray {
    std::vector<QJsonValue> _vec;
    void append(const QJsonValue& val) { _vec.push_back(val); }
    int size() const { return (int)_vec.size(); }
    QJsonValue operator[](int i) const { return _vec[i]; }
};

struct QJsonDocument {
    QJsonObject _obj;
    QJsonArray _arr;
    bool _isObject = true;

    static QJsonDocument fromJson(const QByteArray& data, void* error = nullptr) {
        // Dummy implementation for now
        return QJsonDocument();
    }
    bool isObject() const { return _isObject; }
    QJsonObject object() const { return _obj; }
    QJsonArray array() const { return _arr; }
    QByteArray toJson(int format = 0) const { return "{}"; }
};

// I/O Shims
struct QIODevice {
    enum OpenMode { ReadOnly = 1, WriteOnly = 2, ReadWrite = 3 };
};
using QIODeviceMode = QIODevice::OpenMode;

struct QFile {
    std::string _path;
    QFile(const std::string& path) : _path(path) {}
    bool exists() const { return false; }
    bool open(int mode) { return false; }
    QByteArray readAll() { return ""; }
    void write(const QByteArray& data) {}
    void close() {}
    void seek(long long pos) {}
    long long size() const { return 0; }
};

enum QIODeviceMode { ReadOnly, WriteOnly, ReadWrite };

struct QDir {
    std::string _path;
    QDir(const std::string& path) : _path(path) {}
    bool exists() const { return true; }
    bool exists(const std::string& name) const { return true; }
    bool mkdir(const std::string& name) const { return true; }
    bool mkpath(const std::string& name) const { return true; }
};

struct QBuffer {
    QByteArray* _data;
    QBuffer(QByteArray* data) : _data(data) {}
    bool open(int mode) { return true; }
    void close() {}
};

// Logging shim
#define LOG(x) std::cout << x << std::endl
