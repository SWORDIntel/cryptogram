#pragma once

#include <openssl/evp.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <unistd.h>
#include <utility>
#include <vector>
#include <future>

// === qint/quint typedefs (must be before any code that uses them) ===
#ifndef QT_SHIMS_QUINT_DEFINED
#define QT_SHIMS_QUINT_DEFINED
using quint8 = uint8_t;
using quint16 = uint16_t;
using quint32 = uint32_t;
using quint64 = uint64_t;
using qint8 = int8_t;
using qint16 = int16_t;
using qint32 = int32_t;
using qint64 = int64_t;
#endif

class QByteArray;

namespace QtShims {

inline std::string Base64Encode(const unsigned char *data, size_t len) {
    static constexpr char kTable[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        const auto a = data[i];
        const auto b = (i + 1 < len) ? data[i + 1] : 0;
        const auto c = (i + 2 < len) ? data[i + 2] : 0;

        out.push_back(kTable[(a >> 2) & 0x3F]);
        out.push_back(kTable[((a & 0x3) << 4) | ((b >> 4) & 0xF)]);
        out.push_back((i + 1 < len) ? kTable[((b & 0xF) << 2) | ((c >> 6) & 0x3)] : '=');
        out.push_back((i + 2 < len) ? kTable[c & 0x3F] : '=');
    }
    return out;
}

inline int Base64Value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

inline std::string Base64Decode(const std::string &input) {
    std::string out;
    int val = 0;
    int bits = -8;
    for (const auto c : input) {
        if (std::isspace(static_cast<unsigned char>(c)) || c == '=') {
            continue;
        }
        const auto decoded = Base64Value(c);
        if (decoded < 0) {
            continue;
        }
        val = (val << 6) + decoded;
        bits += 6;
        if (bits >= 0) {
            out.push_back(static_cast<char>((val >> bits) & 0xFF));
            bits -= 8;
        }
    }
    return out;
}

inline std::string HexEncode(const unsigned char *data, size_t len) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        out << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return out.str();
}

inline std::string ReplaceFirstPlaceholder(
    const std::string &input,
    const std::string &replacement) {
    auto out = input;
    for (int i = 1; i <= 9; ++i) {
        const auto marker = "%" + std::to_string(i);
        const auto pos = out.find(marker);
        if (pos != std::string::npos) {
            out.replace(pos, marker.size(), replacement);
            break;
        }
    }
    return out;
}

template <typename T>
inline std::string ToString(const T &value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

} // namespace QtShims

class QString {
public:
    QString() = default;
    QString(const char *value) : _data(value ? value : "") {
    }
    QString(const std::string &value) : _data(value) {
    }
    QString(std::string &&value) : _data(std::move(value)) {
    }
    QString(const QByteArray &value);

    static QString fromUtf8(const char *value) {
        return QString(value);
    }
    static QString fromUtf8(const QByteArray &value);
    static QString fromLatin1(const char *value) {
        return QString(value);
    }
    static QString fromLatin1(const QByteArray &value);
    template <typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
    static QString number(Integer value) {
        return QString(std::to_string(value));
    }
    static QString number(double value) {
        return QString(std::to_string(value));
    }
    static QString number(float value) {
        return QString(std::to_string(value));
    }

    QByteArray toUtf8() const;
    QByteArray toLatin1() const;

    bool isEmpty() const {
        return _data.empty();
    }
    int size() const {
        return static_cast<int>(_data.size());
    }
    int length() const {
        return static_cast<int>(_data.size());
    }
    const char *constData() const {
        return _data.c_str();
    }
    const char *c_str() const {
        return _data.c_str();
    }
    std::string toStdString() const {
        return _data;
    }
    QString toLower() const {
        auto out = _data;
        std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return QString(std::move(out));
    }
    QString toUpper() const {
        auto out = _data;
        std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        return QString(std::move(out));
    }
    QString mid(int pos, int len = -1) const {
        if (pos < 0 || pos >= size()) {
            return QString();
        }
        const auto count = (len < 0) ? std::string::npos : static_cast<size_t>(len);
        return QString(_data.substr(static_cast<size_t>(pos), count));
    }
    bool startsWith(const QString &prefix) const {
        return _data.rfind(prefix._data, 0) == 0;
    }
    bool contains(const QString &needle) const {
        return _data.find(needle._data) != std::string::npos;
    }
    bool contains(const QString &needle, int caseSensitivity) const {
        if (caseSensitivity == 0) { // CaseInsensitive
            auto lowerHay = _data;
            auto lowerNeedle = needle._data;
            std::transform(lowerHay.begin(), lowerHay.end(), lowerHay.begin(), ::tolower);
            std::transform(lowerNeedle.begin(), lowerNeedle.end(), lowerNeedle.begin(), ::tolower);
            return lowerHay.find(lowerNeedle) != std::string::npos;
        }
        return contains(needle);
    }
    int indexOf(const QString &needle, int from = 0) const {
        if (from < 0 || from >= size()) {
            return -1;
        }
        const auto pos = _data.find(needle._data, static_cast<size_t>(from));
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    }
    char at(int index) const {
        return _data.at(static_cast<size_t>(index));
    }
    void append(const QString &other) {
        _data += other._data;
    }
    void clear() {
        _data.clear();
    }
    void truncate(int len) {
        if (len >= 0 && len < size()) {
            _data.resize(static_cast<size_t>(len));
        }
    }
    void chop(int len) {
        if (len <= 0) {
            return;
        }
        if (len >= size()) {
            _data.clear();
        } else {
            _data.resize(_data.size() - static_cast<size_t>(len));
        }
    }
    unsigned long long toULongLong() const {
        return _data.empty() ? 0ULL : std::stoull(_data);
    }
    unsigned int toUInt() const {
        return _data.empty() ? 0U : static_cast<unsigned int>(std::stoul(_data));
    }
    long long toLongLong() const {
        return _data.empty() ? 0LL : std::stoll(_data);
    }
    QString arg(const QString &value) const {
        return QString(QtShims::ReplaceFirstPlaceholder(_data, value._data));
    }
    template <typename T>
    QString arg(const T &value) const {
        return QString(QtShims::ReplaceFirstPlaceholder(_data, QtShims::ToString(value)));
    }

    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end(); }

    operator std::string() const {
        return _data;
    }

    friend QString operator+(const QString &lhs, const QString &rhs) {
        return QString(lhs._data + rhs._data);
    }
    friend QString operator+(const QString &lhs, const char *rhs) {
        return QString(lhs._data + (rhs ? rhs : ""));
    }
    friend QString operator+(const char *lhs, const QString &rhs) {
        return QString((lhs ? lhs : "") + rhs._data);
    }
    friend QString operator+(const QString &lhs, char rhs) {
        return QString(lhs._data + std::string(1, rhs));
    }
    friend QString operator+(char lhs, const QString &rhs) {
        return QString(std::string(1, lhs) + rhs._data);
    }
    QString& operator+=(const QString &rhs) { _data += rhs._data; return *this; }
    QString& operator+=(const char *rhs) { if (rhs) _data += rhs; return *this; }
    QString& operator+=(char rhs) { _data += rhs; return *this; }
    friend bool operator==(const QString &lhs, const QString &rhs) {
        return lhs._data == rhs._data;
    }
    friend bool operator!=(const QString &lhs, const QString &rhs) {
        return !(lhs == rhs);
    }
    friend bool operator<(const QString &lhs, const QString &rhs) {
        return lhs._data < rhs._data;
    }
    friend std::ostream &operator<<(std::ostream &out, const QString &value) {
        out << value._data;
        return out;
    }

private:
    std::string _data;
};

class QByteArray {
public:
    QByteArray() = default;
    QByteArray(const char *value) : _data(value ? value : "") {
    }
    QByteArray(const std::string &value) : _data(value) {
    }
    QByteArray(std::string &&value) : _data(std::move(value)) {
    }
    QByteArray(const char *value, size_t len) : _data(value ? std::string(value, len) : std::string()) {
    }
    QByteArray(int count, char ch) : _data(static_cast<size_t>(count), ch) {
    }

    static QByteArray fromBase64(const QByteArray &value) {
        return QByteArray(QtShims::Base64Decode(value._data));
    }
    static QByteArray fromBase64(const QString &value) {
        return QByteArray(QtShims::Base64Decode(value.toStdString()));
    }

    QByteArray toBase64() const {
        return QByteArray(QtShims::Base64Encode(
            reinterpret_cast<const unsigned char*>(_data.data()),
            _data.size()));
    }
    QByteArray toHex() const {
        return QByteArray(QtShims::HexEncode(
            reinterpret_cast<const unsigned char*>(_data.data()),
            _data.size()));
    }
    const char *constData() const {
        return _data.c_str();
    }
    char *data() {
        return _data.data();
    }
    const char *data() const {
        return _data.data();
    }
    char& operator[](int i) {
        return _data[static_cast<size_t>(i)];
    }
    char operator[](int i) const {
        return _data[static_cast<size_t>(i)];
    }
    int size() const {
        return static_cast<int>(_data.size());
    }
    int length() const {
        return static_cast<int>(_data.size());
    }
    bool isEmpty() const {
        return _data.empty();
    }
    bool empty() const {
        return _data.empty();
    }
    void append(const QByteArray &other) {
        _data += other._data;
    }
    void append(const char *data, size_t len) {
        if (data) {
            _data.append(data, len);
        }
    }
    void clear() {
        _data.clear();
    }
    void resize(size_t len) {
        _data.resize(len);
    }
    void reserve(size_t len) {
        _data.reserve(len);
    }
    char at(int index) const {
        return _data.at(static_cast<size_t>(index));
    }

    auto begin() { return _data.begin(); }
    auto end() { return _data.end(); }
    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end(); }

    operator std::string() const {
        return _data;
    }

    std::string toStdString() const {
        return _data;
    }

private:
    std::string _data;
};

inline QByteArray QString::toUtf8() const {
    return QByteArray(_data);
}

inline QByteArray QString::toLatin1() const {
    return QByteArray(_data);
}

inline QString QString::fromUtf8(const QByteArray &value) {
    return QString(static_cast<std::string>(value));
}

inline QString QString::fromLatin1(const QByteArray &value) {
    return QString(static_cast<std::string>(value));
}

inline QString::QString(const QByteArray &value) : _data(value.toStdString()) {}

template <typename T>
class QVector : public std::vector<T> {
public:
    using std::vector<T>::vector;

    void append(const T &value) {
        this->push_back(value);
    }
    bool isEmpty() const {
        return this->empty();
    }
    bool contains(const T &value) const {
        return std::find(this->begin(), this->end(), value) != this->end();
    }
    T &first() {
        return this->front();
    }
    const T &first() const {
        return this->front();
    }
    void removeFirst() {
        this->erase(this->begin());
    }
    void removeAll(const T &value) {
        this->erase(std::remove(this->begin(), this->end(), value), this->end());
    }
    T takeFirst() {
        T val = std::move(this->front());
        this->erase(this->begin());
        return val;
    }
    T takeAt(int i) {
        T val = std::move((*this)[static_cast<size_t>(i)]);
        this->erase(this->begin() + i);
        return val;
    }
    void prepend(const T &value) {
        this->insert(this->begin(), value);
    }
    QVector& operator<<(const T &value) {
        this->push_back(value);
        return *this;
    }
    QVector& operator<<(const QVector<T> &other) {
        this->insert(this->end(), other.begin(), other.end());
        return *this;
    }
};

// === QList alias (after QVector is defined) ===
template<typename T> using QList = QVector<T>;

// === QStringList with join support ===
class QStringList : public QVector<QString> {
public:
    using QVector<QString>::QVector;
    using QVector<QString>::append;
    QString join(const QString &separator) const {
        std::string result;
        for (size_t i = 0; i < this->size(); ++i) {
            if (i > 0) result += separator.toStdString();
            result += (*this)[i].toStdString();
        }
        return QString(result);
    }
    void append(const QStringList &other) {
        this->insert(this->end(), other.begin(), other.end());
    }
    QStringList operator+(const QStringList &other) const {
        QStringList result = *this;
        result.insert(result.end(), other.begin(), other.end());
        return result;
    }
};

template <typename K, typename V>
class QMap : public std::map<K, V> {
public:
    using std::map<K, V>::map;

    bool contains(const K &key) const {
        return this->find(key) != this->end();
    }
    bool isEmpty() const {
        return this->empty();
    }
    V &first() {
        return this->begin()->second;
    }
    const V &first() const {
        return this->begin()->second;
    }
    V value(const K &key, const V &defaultValue = V()) const {
        const auto it = this->find(key);
        return (it != this->end()) ? it->second : defaultValue;
    }
    void remove(const K &key) {
        this->erase(key);
    }
};

template <typename T>
class QSet : public std::set<T> {
public:
    using std::set<T>::set;

    bool contains(const T &value) const {
        return this->find(value) != this->end();
    }
    bool isEmpty() const {
        return this->empty();
    }
};

template<typename A, typename B> using QPair = std::pair<A, B>;

namespace QtShims {
    inline QString fromUtf8(const char* s) { return QString::fromUtf8(s); }
    inline QString number(long long n) { return QString::number(n); }
}

#define Q_DECLARE_METATYPE(x)

struct QDateTime {
    long long msecs = 0;

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
    static QDateTime fromMSecsSinceEpoch(long long value) {
        return { value };
    }
    long long toMSecsSinceEpoch() const {
        return msecs;
    }
    QDateTime addDays(int days) const {
        return { msecs + static_cast<long long>(days) * 24 * 60 * 60 * 1000 };
    }
    bool isValid() const {
        return msecs != 0;
    }
    QString toString(int format = 0) const {
        (void)format;
        return QString::number(msecs);
    }
    friend bool operator<(const QDateTime &lhs, const QDateTime &rhs) {
        return lhs.msecs < rhs.msecs;
    }
    friend bool operator==(const QDateTime &lhs, const QDateTime &rhs) {
        return lhs.msecs == rhs.msecs;
    }
    friend bool operator!=(const QDateTime &lhs, const QDateTime &rhs) {
        return !(lhs == rhs);
    }
};

struct QJsonObject;
struct QJsonArray;

struct QJsonValue {
    enum Type { Null, Bool, Double, String, Array, Object };

    Type _type = Null;
    QString _string;
    double _double = 0;
    bool _bool = false;
    std::shared_ptr<QJsonObject> _object;
    std::shared_ptr<QJsonArray> _array;

    QJsonValue() = default;
    QJsonValue(const char* s) : _type(String), _string(s) {}
    QJsonValue(const QString &s) : _type(String), _string(s) {}
    QJsonValue(double d) : _type(Double), _double(d) {}
    QJsonValue(int d) : _type(Double), _double(d) {}
    QJsonValue(bool b) : _type(Bool), _bool(b) {}
    QJsonValue(const QJsonObject &object);
    QJsonValue(const QJsonArray &array);

    bool isString() const { return _type == String; }
    bool isDouble() const { return _type == Double; }
    bool isBool() const { return _type == Bool; }
    bool isObject() const { return _type == Object; }
    bool isArray() const { return _type == Array; }
    bool isNull() const { return _type == Null; }

    QString toString() const { return _string; }
    double toDouble(double def = 0) const { return _type == Double ? _double : def; }
    int toInt(int def = 0) const { return _type == Double ? static_cast<int>(_double) : def; }
    bool toBool(bool def = false) const { return _type == Bool ? _bool : def; }
    QJsonObject toObject() const;
    QJsonArray toArray() const;
};

struct QJsonObject {
    using storage_type = std::map<QString, QJsonValue>;

    struct iterator {
        storage_type::iterator inner;
        iterator &operator++() { ++inner; return *this; }
        bool operator!=(const iterator &other) const { return inner != other.inner; }
        auto &operator*() { return *inner; }
        auto operator->() { return inner.operator->(); }
        QString key() const { return inner->first; }
        QJsonValue &value() const { return inner->second; }
    };

    struct const_iterator {
        storage_type::const_iterator inner;
        const_iterator &operator++() { ++inner; return *this; }
        bool operator!=(const const_iterator &other) const { return inner != other.inner; }
        const auto &operator*() const { return *inner; }
        auto operator->() const { return inner.operator->(); }
        QString key() const { return inner->first; }
        const QJsonValue &value() const { return inner->second; }
    };

    storage_type _map;

    bool contains(const QString& key) const { return _map.find(key) != _map.end(); }
    QJsonValue &operator[](const QString& key) { return _map[key]; }
    QJsonValue operator[](const QString& key) const {
        auto it = _map.find(key);
        return it != _map.end() ? it->second : QJsonValue();
    }
    QJsonValue value(const QString& key, const QJsonValue& def = QJsonValue()) const {
        auto it = _map.find(key);
        return it != _map.end() ? it->second : def;
    }
    void insert(const QString& key, const QJsonValue& value) { _map[key] = value; }
    iterator begin() { return iterator{_map.begin()}; }
    iterator end() { return iterator{_map.end()}; }
    const_iterator begin() const { return const_iterator{_map.begin()}; }
    const_iterator end() const { return const_iterator{_map.end()}; }
};

struct QJsonArray {
    QVector<QJsonValue> _vec;

    void append(const QJsonValue& val) { _vec.push_back(val); }
    void append(const QJsonObject &val) { _vec.push_back(QJsonValue(val)); }
    int size() const { return static_cast<int>(_vec.size()); }
    bool isEmpty() const { return _vec.empty(); }
    QJsonValue operator[](int i) const { return _vec[static_cast<size_t>(i)]; }
    auto begin() { return _vec.begin(); }
    auto end() { return _vec.end(); }
    auto begin() const { return _vec.begin(); }
    auto end() const { return _vec.end(); }
};

inline QJsonValue::QJsonValue(const QJsonObject &object)
    : _type(Object)
    , _object(std::make_shared<QJsonObject>(object)) {
}

inline QJsonValue::QJsonValue(const QJsonArray &array)
    : _type(Array)
    , _array(std::make_shared<QJsonArray>(array)) {
}

inline QJsonObject QJsonValue::toObject() const {
    return _object ? *_object : QJsonObject();
}

inline QJsonArray QJsonValue::toArray() const {
    return _array ? *_array : QJsonArray();
}

struct QJsonParseError {
    enum ParseError {
        NoError = 0,
        IllegalValue,
    };

    ParseError error = NoError;
    QString errorString() const {
        return (error == NoError) ? QString() : QString("parse error");
    }
};

// Minimal JSON serializer
inline std::string jsonEscapeString(const std::string &s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

inline std::string jsonValueToString(const QJsonValue &v);

inline std::string jsonObjectToString(const QJsonObject &obj) {
    std::string out = "{";
    bool first = true;
    for (const auto &kv : obj._map) {
        if (!first) out += ",";
        first = false;
        out += "\"" + jsonEscapeString(kv.first.toStdString()) + "\":";
        out += jsonValueToString(kv.second);
    }
    out += "}";
    return out;
}

inline std::string jsonArrayToString(const QJsonArray &arr) {
    std::string out = "[";
    bool first = true;
    for (const auto &v : arr._vec) {
        if (!first) out += ",";
        first = false;
        out += jsonValueToString(v);
    }
    out += "]";
    return out;
}

inline std::string jsonValueToString(const QJsonValue &v) {
    switch (v._type) {
        case QJsonValue::Null: return "null";
        case QJsonValue::Bool: return v._bool ? "true" : "false";
        case QJsonValue::Double: return std::to_string(v._double);
        case QJsonValue::String: return "\"" + jsonEscapeString(v._string.toStdString()) + "\"";
        case QJsonValue::Object: return v._object ? jsonObjectToString(*v._object) : "{}";
        case QJsonValue::Array: return v._array ? jsonArrayToString(*v._array) : "[]";
    }
    return "null";
}

// Minimal JSON parser
struct JsonParser {
    const std::string &s;
    size_t pos = 0;

    explicit JsonParser(const std::string &str) : s(str) {}

    void skipWs() {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r'))
            ++pos;
    }

    bool parse(QJsonValue &out) {
        skipWs();
        if (pos >= s.size()) return false;
        char c = s[pos];
        if (c == '{') return parseObject(out);
        if (c == '[') return parseArray(out);
        if (c == '"') return parseString(out);
        if (c == 't' || c == 'f') return parseBool(out);
        if (c == 'n') return parseNull(out);
        return parseNumber(out);
    }

    bool parseObject(QJsonValue &out) {
        ++pos; // skip '{'
        out._type = QJsonValue::Object;
        out._object = std::make_shared<QJsonObject>();
        skipWs();
        if (pos < s.size() && s[pos] == '}') { ++pos; return true; }
        while (pos < s.size()) {
            skipWs();
            if (pos >= s.size() || s[pos] != '"') return false;
            QJsonValue keyVal;
            if (!parseString(keyVal)) return false;
            skipWs();
            if (pos >= s.size() || s[pos] != ':') return false;
            ++pos;
            QJsonValue val;
            if (!parse(val)) return false;
            (*out._object)[keyVal._string] = val;
            skipWs();
            if (pos < s.size() && s[pos] == ',') { ++pos; continue; }
            if (pos < s.size() && s[pos] == '}') { ++pos; return true; }
            return false;
        }
        return false;
    }

    bool parseArray(QJsonValue &out) {
        ++pos; // skip '['
        out._type = QJsonValue::Array;
        out._array = std::make_shared<QJsonArray>();
        skipWs();
        if (pos < s.size() && s[pos] == ']') { ++pos; return true; }
        while (pos < s.size()) {
            QJsonValue val;
            if (!parse(val)) return false;
            out._array->append(val);
            skipWs();
            if (pos < s.size() && s[pos] == ',') { ++pos; continue; }
            if (pos < s.size() && s[pos] == ']') { ++pos; return true; }
            return false;
        }
        return false;
    }

    bool parseString(QJsonValue &out) {
        ++pos; // skip opening '"'
        std::string result;
        while (pos < s.size()) {
            char c = s[pos++];
            if (c == '"') {
                out._type = QJsonValue::String;
                out._string = QString(result);
                return true;
            }
            if (c == '\\' && pos < s.size()) {
                char esc = s[pos++];
                switch (esc) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': {
                        if (pos + 4 > s.size()) return false;
                        int code = 0;
                        for (int i = 0; i < 4; ++i) {
                            char hex = s[pos++];
                            code <<= 4;
                            if (hex >= '0' && hex <= '9') code += hex - '0';
                            else if (hex >= 'a' && hex <= 'f') code += hex - 'a' + 10;
                            else if (hex >= 'A' && hex <= 'F') code += hex - 'A' + 10;
                            else return false;
                        }
                        if (code < 0x80) result += static_cast<char>(code);
                        else if (code < 0x800) {
                            result += static_cast<char>(0xC0 | (code >> 6));
                            result += static_cast<char>(0x80 | (code & 0x3F));
                        } else {
                            result += static_cast<char>(0xE0 | (code >> 12));
                            result += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (code & 0x3F));
                        }
                        break;
                    }
                    default: return false;
                }
            } else {
                result += c;
            }
        }
        return false;
    }

    bool parseBool(QJsonValue &out) {
        if (s.compare(pos, 4, "true") == 0) {
            out._type = QJsonValue::Bool;
            out._bool = true;
            pos += 4;
            return true;
        }
        if (s.compare(pos, 5, "false") == 0) {
            out._type = QJsonValue::Bool;
            out._bool = false;
            pos += 5;
            return true;
        }
        return false;
    }

    bool parseNull(QJsonValue &out) {
        if (s.compare(pos, 4, "null") == 0) {
            out._type = QJsonValue::Null;
            pos += 4;
            return true;
        }
        return false;
    }

    bool parseNumber(QJsonValue &out) {
        size_t start = pos;
        if (pos < s.size() && (s[pos] == '-' || s[pos] == '+')) ++pos;
        while (pos < s.size() && (std::isdigit(static_cast<unsigned char>(s[pos])) || s[pos] == '.' || s[pos] == 'e' || s[pos] == 'E' || s[pos] == '+' || s[pos] == '-'))
            ++pos;
        if (pos == start) return false;
        try {
            out._type = QJsonValue::Double;
            out._double = std::stod(s.substr(start, pos - start));
            return true;
        } catch (...) {
            return false;
        }
    }
};

struct QJsonDocument {
    enum JsonFormat {
        Compact = 0,
    };

    QJsonObject _obj;
    QJsonArray _arr;
    bool _isObject = true;

    QJsonDocument() = default;
    explicit QJsonDocument(const QJsonObject &obj) : _obj(obj), _isObject(true) {
    }
    explicit QJsonDocument(const QJsonArray &arr) : _arr(arr), _isObject(false) {
    }

    static QJsonDocument fromJson(const QByteArray& data, QJsonParseError* error = nullptr) {
        auto str = data.toStdString();
        JsonParser parser(str);
        QJsonValue val;
        if (parser.parse(val)) {
            if (val.isObject()) {
                if (error) error->error = QJsonParseError::NoError;
                return QJsonDocument(val.toObject());
            }
            if (val.isArray()) {
                if (error) error->error = QJsonParseError::NoError;
                return QJsonDocument(val.toArray());
            }
        }
        if (error) error->error = QJsonParseError::IllegalValue;
        return QJsonDocument();
    }
    bool isObject() const { return _isObject; }
    QJsonObject object() const { return _obj; }
    QJsonArray array() const { return _arr; }
    QByteArray toJson(int format = 0) const {
        (void)format;
        if (_isObject) return QByteArray(jsonObjectToString(_obj));
        return QByteArray(jsonArrayToString(_arr));
    }
};

struct QIODevice {
    enum OpenMode { ReadOnly = 1, WriteOnly = 2, ReadWrite = 3 };
};
using QIODeviceMode = QIODevice::OpenMode;

struct QFile {
    QString _path;
    int _mode = 0;
    std::string _buffer;
    bool _isOpen = false;

    explicit QFile(const QString& path) : _path(path) {}

    bool exists() const {
        struct stat st;
        return ::stat(_path.toStdString().c_str(), &st) == 0;
    }

    bool open(int mode) {
        _mode = mode;
        if (mode & QIODevice::ReadOnly) {
            std::ifstream f(_path.toStdString(), std::ios::binary);
            if (!f.is_open()) return false;
            _buffer.assign(std::istreambuf_iterator<char>(f),
                          std::istreambuf_iterator<char>());
            f.close();
            _isOpen = true;
            return true;
        }
        if (mode & QIODevice::WriteOnly) {
            _buffer.clear();
            _isOpen = true;
            return true;
        }
        if (mode & QIODevice::ReadWrite) {
            std::ifstream f(_path.toStdString(), std::ios::binary);
            if (f.is_open()) {
                _buffer.assign(std::istreambuf_iterator<char>(f),
                              std::istreambuf_iterator<char>());
                f.close();
            }
            _isOpen = true;
            return true;
        }
        return false;
    }

    QByteArray readAll() {
        return QByteArray(_buffer);
    }

    void write(const QByteArray& data) {
        if (_mode & QIODevice::WriteOnly) {
            _buffer = data.toStdString();
        } else if (_mode & QIODevice::ReadWrite) {
            _buffer = data.toStdString();
        }
    }

    void close() {
        if (!_isOpen) return;
        if (_mode & (QIODevice::WriteOnly | QIODevice::ReadWrite)) {
            std::ofstream f(_path.toStdString(), std::ios::binary | std::ios::trunc);
            if (f.is_open()) {
                f.write(_buffer.data(), _buffer.size());
                f.close();
            }
        }
        _isOpen = false;
    }

    void seek(long long pos) {
        (void)pos;
    }

    long long size() const {
        if (_isOpen) return static_cast<long long>(_buffer.size());
        struct stat st;
        if (::stat(_path.toStdString().c_str(), &st) == 0) {
            return static_cast<long long>(st.st_size);
        }
        return 0;
    }

    ~QFile() {
        if (_isOpen) close();
    }
};

struct QDir {
    QString _path;
    explicit QDir(const QString& path) : _path(path) {}

    bool exists() const {
        struct stat st;
        return ::stat(_path.toStdString().c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    bool exists(const QString& name) const {
        auto full = _path.toStdString() + "/" + name.toStdString();
        struct stat st;
        return ::stat(full.c_str(), &st) == 0;
    }

    bool mkdir(const QString& name) const {
        auto full = _path.toStdString() + "/" + name.toStdString();
        return ::mkdir(full.c_str(), 0755) == 0 || errno == EEXIST;
    }

    bool mkpath(const QString& name) const {
        if (name.toStdString() == ".") {
            return mkpathRecursive(_path.toStdString());
        }
        auto full = _path.toStdString() + "/" + name.toStdString();
        return mkpathRecursive(full);
    }

    static bool mkpathRecursive(const std::string& path) {
        if (path.empty()) return true;
        if (::access(path.c_str(), F_OK) == 0) return true;
        auto parent = path.substr(0, path.find_last_of('/'));
        if (!parent.empty() && parent != path) {
            if (!mkpathRecursive(parent)) return false;
        }
        return ::mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
    }

    QStringList entryList(const QStringList &filters = QStringList(), int sort = 0, int filter = 0) const;
};

struct QBuffer {
    QByteArray* _data;
    explicit QBuffer(QByteArray* data) : _data(data) {}
    bool open(int mode) { (void)mode; return true; }
    void close() {}
};

#define LOG(x) std::cout << QString x << std::endl

// === Qt meta-object macros (no MOC on Android) ===
#define Q_OBJECT
#define Q_GADGET
#define Q_PROPERTY(...)
#define Q_ENUMS(x)
#define Q_ENUM(x)
#define Q_INVOKABLE
#define Q_SIGNAL public
#define Q_SLOT public
#define Q_SIGNALS public
#define Q_SLOTS public
#define emit
#define Q_DECLARE_METATYPE(x)
#define Q_DISABLE_COPY(Class) \
    Class(const Class &) = delete; \
    Class &operator=(const Class &) = delete;
#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;
#define Q_INTERFACES(x)
#define Q_CLASSINFO(name, value)
#define Q_PLUGIN_METADATA(x)
#define Q_NAMESPACE

// === Qt keyword shims ===
#define Q_DECL_EXPORT
#define Q_DECL_IMPORT
#define Q_DECL_OVERRIDE
#define Q_DECL_FINAL final
#define Q_DECL_NOEXCEPT noexcept
#define Q_DECL_NOTHROW noexcept
#define Q_DECL_PURE_FUNCTION
#define Q_DECL_CONST_FUNCTION
#define QT_VERSION 0x060000
#define QT_VERSION_STR "6.0.0"
#define QT_BEGIN_NAMESPACE
#define QT_END_NAMESPACE
#define QT_BEGIN_HEADER
#define QT_END_HEADER
#define Q_REQUIRED_RESULT
#define Q_NODISCARD
#define Q_ALWAYS_INLINE inline
#define Q_NEVER_INLINE
#define Q_LIKELY(x) (x)
#define Q_UNLIKELY(x) (x)
#define Q_ASSUME(x)
#define Q_UNREACHABLE()
#define QByteArrayLiteral(s) s
#define QStringLiteral(s) QString(s)

// === Forward declarations ===
class QObject;
class QWidget;

// === QObject stub (no MOC, no signals/slots) ===
class QObject {
public:
    QObject(QObject *parent = nullptr) : _parent(parent) {}
    virtual ~QObject() = default;
    QObject *parent() const { return _parent; }
    void setParent(QObject *parent) { _parent = parent; }
    bool blockSignals(bool b) { (void)b; return false; }
    bool signalsBlocked() const { return false; }
    void deleteLater() {}
    QString objectName() const { return _objectName; }
    void setObjectName(const QString &name) { _objectName = name; }
    template<typename Func> void connect(QObject *, const char *, Func) {}
    template<typename Func> void disconnect(QObject *, const char *, Func) {}
protected:
    virtual void timerEvent(class QTimerEvent *) {}
private:
    QObject *_parent;
    QString _objectName;
};

// === QTimer stub ===
class QTimer : public QObject {
public:
    QTimer(QObject *parent = nullptr) : QObject(parent) {}
    ~QTimer() = default;

    void setInterval(int msec) { _interval = msec; }
    int interval() const { return _interval; }
    bool isActive() const { return _active; }
    void start() { _active = true; }
    void start(int msec) { _interval = msec; _active = true; }
    void stop() { _active = false; }
    void setSingleShot(bool single) { _singleShot = single; }
    bool isSingleShot() const { return _singleShot; }
    static void singleShot(int msec, QObject *, std::function<void()> slot) {
        (void)msec; (void)slot;
    }
    static void singleShot(int msec, std::function<void()> slot) {
        (void)msec; (void)slot;
    }
    std::function<void()> timeout;
private:
    int _interval = 0;
    bool _active = false;
    bool _singleShot = false;
};

// === QThread stub ===
class QThread : public QObject {
public:
    QThread(QObject *parent = nullptr) : QObject(parent) {}
    ~QThread() { if (_thread.joinable()) _thread.detach(); }

    void start() {
        _running = true;
        _thread = std::thread([this]() { this->run(); });
    }
    void wait() { if (_thread.joinable()) _thread.join(); }
    bool isRunning() const { return _running; }
    bool isFinished() const { return !_running; }
    void requestInterruption() { _interrupted = true; }
    bool isInterruptionRequested() const { return _interrupted; }
    void quit() { _running = false; }
    void terminate() { _running = false; }
    static void sleep(unsigned long secs) { std::this_thread::sleep_for(std::chrono::seconds(secs)); }
    static void msleep(unsigned long msecs) { std::this_thread::sleep_for(std::chrono::milliseconds(msecs)); }
    static void usleep(unsigned long usecs) { std::this_thread::sleep_for(std::chrono::microseconds(usecs)); }
    static QThread *currentThread() { return nullptr; }
protected:
    virtual void run() {}
private:
    std::thread _thread;
    std::atomic<bool> _running{false};
    std::atomic<bool> _interrupted{false};
};

// === QMutex / QMutexLocker ===
class QMutex {
public:
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }
    bool tryLock() { return _mutex.try_lock(); }
    bool tryLock(int timeout) {
        (void)timeout;
        return _mutex.try_lock();
    }
private:
    std::mutex _mutex;
};

class QMutexLocker {
public:
    explicit QMutexLocker(QMutex *mutex) : _mutex(mutex) { if (_mutex) _mutex->lock(); }
    ~QMutexLocker() { if (_mutex) _mutex->unlock(); }
    QMutex *mutex() const { return _mutex; }
private:
    QMutex *_mutex;
};

// === QReadWriteLock stub ===
class QReadWriteLock {
public:
    void lockForRead() { _mutex.lock_shared(); }
    void lockForWrite() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }
private:
    std::shared_mutex _mutex;
};

// === QDebug stub ===
class QDebug {
public:
    QDebug() = default;
    explicit QDebug(const QString &) {}
    template<typename T> QDebug &operator<<(const T &value) {
        std::cout << value;
        return *this;
    }
    QDebug &operator<<(const char *str) { std::cout << (str ? str : ""); return *this; }
    QDebug &operator<<(const QString &str) { std::cout << str.toStdString(); return *this; }
    QDebug &operator<<(const QByteArray &str) { std::cout << str.toStdString(); return *this; }
    QDebug &noquote() { return *this; }
    QDebug &nospace() { return *this; }
    QDebug &space() { std::cout << " "; return *this; }
};
inline QDebug qDebug() { return QDebug(); }
inline QDebug qInfo() { return QDebug(); }
inline QDebug qWarning() { return QDebug(); }
inline QDebug qCritical() { return QDebug(); }

// === QFileInfo stub ===
struct QFileInfo {
    QString _path;
    explicit QFileInfo(const QString &path) : _path(path) {}
    explicit QFileInfo(const QFile &file) : _path(file._path) {}

    bool exists() const {
        struct stat st;
        return ::stat(_path.toStdString().c_str(), &st) == 0;
    }
    long long size() const {
        struct stat st;
        if (::stat(_path.toStdString().c_str(), &st) == 0) return static_cast<long long>(st.st_size);
        return 0;
    }
    QString fileName() const {
        auto s = _path.toStdString();
        auto pos = s.find_last_of('/');
        return (pos != std::string::npos) ? QString(s.substr(pos + 1)) : _path;
    }
    QString absoluteFilePath() const { return _path; }
    QString filePath() const { return _path; }
    QString suffix() const {
        auto s = _path.toStdString();
        auto pos = s.find_last_of('.');
        return (pos != std::string::npos) ? QString(s.substr(pos + 1)) : QString();
    }
    QString completeBaseName() const {
        auto s = _path.toStdString();
        auto slash = s.find_last_of('/');
        if (slash != std::string::npos) s = s.substr(slash + 1);
        auto dot = s.find_last_of('.');
        return (dot != std::string::npos) ? QString(s.substr(0, dot)) : QString(s);
    }
    bool isFile() const {
        struct stat st;
        return ::stat(_path.toStdString().c_str(), &st) == 0 && S_ISREG(st.st_mode);
    }
    bool isDir() const {
        struct stat st;
        return ::stat(_path.toStdString().c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }
    QDateTime lastModified() const {
        struct stat st;
        if (::stat(_path.toStdString().c_str(), &st) == 0) {
            return QDateTime::fromMSecsSinceEpoch(static_cast<long long>(st.st_mtime) * 1000);
        }
        return QDateTime();
    }
};

// === QTextStream stub ===
class QTextStream {
public:
    explicit QTextStream(QByteArray *array) : _ss(*array) {}
    explicit QTextStream(QString *string) : _ss(string->toStdString()) {}
    explicit QTextStream(QIODevice *device) { (void)device; }

    QTextStream &operator<<(const QString &s) { _ss << s.toStdString(); return *this; }
    QTextStream &operator<<(const char *s) { _ss << (s ? s : ""); return *this; }
    QTextStream &operator<<(int v) { _ss << v; return *this; }
    QTextStream &operator<<(long long v) { _ss << v; return *this; }
    QTextStream &operator<<(double v) { _ss << v; return *this; }
    QTextStream &operator<<(char c) { _ss << c; return *this; }

    QString readAll() { return QString(_ss.str()); }
    QString readLine() {
        std::string line;
        if (std::getline(_ss, line)) return QString(line);
        return QString();
    }
    bool atEnd() const { return _ss.eof(); }
private:
    std::stringstream _ss;
};

// === QCryptographicHash stub using OpenSSL ===
class QCryptographicHash {
public:
    enum Algorithm {
        Md4, Md5, Sha1, Sha224, Sha256, Sha384, Sha512, Keccak_256, RealSha3_256
    };

    explicit QCryptographicHash(Algorithm method) : _method(method) {}

    void addData(const char *data, int length) {
        _buffer.append(data, static_cast<size_t>(length));
    }
    void addData(const QByteArray &data) {
        _buffer += data.toStdString();
    }
    QByteArray result() const {
        std::string out;
        switch (_method) {
            case Sha256: out.resize(32); break;
            case Sha512: out.resize(64); break;
            case Sha1: out.resize(20); break;
            case Md5: out.resize(16); break;
            case Sha224: out.resize(28); break;
            case Sha384: out.resize(48); break;
            default: out.resize(32); break;
        }
        auto *data = reinterpret_cast<const unsigned char*>(_buffer.data());
        switch (_method) {
            case Sha256: EVP_Digest(data, _buffer.size(), reinterpret_cast<unsigned char*>(out.data()), nullptr, EVP_sha256(), nullptr); break;
            case Sha512: EVP_Digest(data, _buffer.size(), reinterpret_cast<unsigned char*>(out.data()), nullptr, EVP_sha512(), nullptr); break;
            case Sha1: EVP_Digest(data, _buffer.size(), reinterpret_cast<unsigned char*>(out.data()), nullptr, EVP_sha1(), nullptr); break;
            case Md5: EVP_Digest(data, _buffer.size(), reinterpret_cast<unsigned char*>(out.data()), nullptr, EVP_md5(), nullptr); break;
            case Sha224: EVP_Digest(data, _buffer.size(), reinterpret_cast<unsigned char*>(out.data()), nullptr, EVP_sha224(), nullptr); break;
            case Sha384: EVP_Digest(data, _buffer.size(), reinterpret_cast<unsigned char*>(out.data()), nullptr, EVP_sha384(), nullptr); break;
            default: EVP_Digest(data, _buffer.size(), reinterpret_cast<unsigned char*>(out.data()), nullptr, EVP_sha256(), nullptr); break;
        }
        return QByteArray(out);
    }
    static QByteArray hash(const QByteArray &data, Algorithm method) {
        QCryptographicHash h(method);
        h.addData(data);
        return h.result();
    }
    void reset() { _buffer.clear(); }
private:
    Algorithm _method;
    std::string _buffer;
};

// === QStandardPaths stub ===
struct QStandardPaths {
    enum StandardLocation {
        DesktopLocation, DocumentsLocation, FontsLocation, ApplicationsLocation,
        MusicLocation, MoviesLocation, PicturesLocation, TempLocation,
        HomeLocation, DataLocation, CacheLocation, GenericDataLocation,
        DownloadLocation, GenericCacheLocation, GenericConfigLocation,
        AppDataLocation, AppConfigLocation, AppLocalDataLocation
    };

    static QString writableLocation(StandardLocation where) {
        (void)where;
        return QString("/data/data/org.telegram.messenger/files");
    }
    static QStringList standardLocations(StandardLocation where) {
        (void)where;
        QStringList list;
        list.append(QString("/data/data/org.telegram.messenger/files"));
        list.append(QString("/sdcard/cryptogram"));
        return list;
    }
};

// === QRegularExpression stub ===
class QRegularExpression {
public:
    QRegularExpression() = default;
    QRegularExpression(const QString &pattern) : _pattern(pattern), _re(pattern.toStdString()) {}
    QRegularExpression(const QString &pattern, int options) : _pattern(pattern), _re(pattern.toStdString()) { (void)options; }

    QString pattern() const { return _pattern; }
    bool isValid() const { return _re.mark_count() > 0 || _pattern.isEmpty(); }

    bool match(const QString &subject) const {
        return std::regex_search(subject.toStdString(), _re);
    }

    struct Match {
        bool hasMatch = false;
        QString captured(int nth = 0) const { (void)nth; return QString(); }
    };

    Match match(const QString &subject, int offset = 0) const {
        (void)offset;
        Match m;
        m.hasMatch = std::regex_search(subject.toStdString(), _re);
        return m;
    }

    QString replace(const QString &subject, const QString &replacement) const {
        try {
            return QString(std::regex_replace(subject.toStdString(), _re, replacement.toStdString()));
        } catch (...) {
            return subject;
        }
    }
private:
    QString _pattern;
    std::regex _re;
};

// === QElapsedTimer stub ===
class QElapsedTimer {
public:
    void start() { _begin = std::chrono::steady_clock::now(); }
    long long elapsed() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - _begin).count();
    }
    long long nsecsElapsed() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - _begin).count();
    }
    long long restart() {
        auto now = std::chrono::steady_clock::now();
        auto e = std::chrono::duration_cast<std::chrono::milliseconds>(now - _begin).count();
        _begin = now;
        return e;
    }
    bool isValid() const { return true; }
private:
    std::chrono::steady_clock::time_point _begin;
};

// === QRandomGenerator stub ===
class QRandomGenerator {
public:
    static QRandomGenerator global;
    static QRandomGenerator system;

    quint32 generate() {
        std::random_device rd;
        return static_cast<quint32>(rd());
    }
    quint64 generate64() {
        std::random_device rd;
        return (static_cast<quint64>(rd()) << 32) | static_cast<quint64>(rd());
    }
    double generateDouble() {
        std::random_device rd;
        return static_cast<double>(rd()) / static_cast<double>(std::numeric_limits<uint32_t>::max());
    }
    void fillRange(quint32 *begin, quint32 *end) {
        std::random_device rd;
        for (auto *p = begin; p != end; ++p) *p = rd();
    }
};
inline QRandomGenerator QRandomGenerator::global;
inline QRandomGenerator QRandomGenerator::system;

// === QDataStream stub ===
class QDataStream {
public:
    enum Version { Qt_5_0 = 16, Qt_6_0 = 20 };

    explicit QDataStream(QByteArray *array, int mode = 3) : _array(array) { (void)mode; }

    QDataStream &operator<<(quint32 v) { writeRaw(&v, sizeof(v)); return *this; }
    QDataStream &operator<<(quint64 v) { writeRaw(&v, sizeof(v)); return *this; }
    QDataStream &operator<<(qint32 v) { writeRaw(&v, sizeof(v)); return *this; }
    QDataStream &operator<<(qint64 v) { writeRaw(&v, sizeof(v)); return *this; }
    QDataStream &operator<<(bool v) { quint8 b = v ? 1 : 0; writeRaw(&b, 1); return *this; }
    QDataStream &operator<<(const QByteArray &data) {
        quint32 len = static_cast<quint32>(data.size());
        writeRaw(&len, sizeof(len));
        if (len > 0) writeRaw(data.constData(), len);
        return *this;
    }

    QDataStream &operator>>(quint32 &v) { readRaw(&v, sizeof(v)); return *this; }
    QDataStream &operator>>(quint64 &v) { readRaw(&v, sizeof(v)); return *this; }
    QDataStream &operator>>(qint32 &v) { readRaw(&v, sizeof(v)); return *this; }
    QDataStream &operator>>(qint64 &v) { readRaw(&v, sizeof(v)); return *this; }
    QDataStream &operator>>(bool &v) { quint8 b; readRaw(&b, 1); v = b != 0; return *this; }

    void setVersion(Version v) { (void)v; }
private:
    QByteArray *_array;
    size_t _pos = 0;

    void writeRaw(const void *data, size_t len) {
        if (!_array) return;
        auto s = _array->toStdString();
        s.append(static_cast<const char*>(data), len);
        *_array = QByteArray(s);
    }
    void readRaw(void *data, size_t len) {
        if (!_array) return;
        auto s = _array->toStdString();
        if (_pos + len > s.size()) return;
        std::memcpy(data, s.data() + _pos, len);
        _pos += len;
    }
};

// === QNetworkAccessManager stub (no network on native side) ===
class QNetworkReply {
public:
    enum NetworkError { NoError = 0, HostNotFoundError, TimeoutError, UnknownNetworkError };
    NetworkError error() const { return NoError; }
    QByteArray readAll() const { return QByteArray(); }
    QString errorString() const { return QString("stub"); }
    bool isFinished() const { return true; }
    void abort() {}
    void close() {}
};
class QNetworkRequest {
public:
    QNetworkRequest() = default;
    explicit QNetworkRequest(const QString &url) : _url(url) {}
    void setHeader(int header, const QString &value) { (void)header; (void)value; }
    void setRawHeader(const QString &name, const QString &value) { (void)name; (void)value; }
    QString url() const { return _url; }
private:
    QString _url;
};
class QNetworkAccessManager : public QObject {
public:
    QNetworkAccessManager(QObject *parent = nullptr) : QObject(parent) {}
    QNetworkReply *get(const QNetworkRequest &) { return &_dummyReply; }
    QNetworkReply *post(const QNetworkRequest &, const QByteArray &) { return &_dummyReply; }
    QNetworkReply *put(const QNetworkRequest &, const QByteArray &) { return &_dummyReply; }
    QNetworkReply *deleteResource(const QNetworkRequest &) { return &_dummyReply; }
    void setProxy(const QString &proxy) { (void)proxy; }
private:
    QNetworkReply _dummyReply;
};

// === QNetworkInterface stub ===
class QNetworkInterface {
public:
    static QList<QNetworkInterface> allInterfaces() { return {}; }
    static QList<QString> allAddresses() { return {}; }
    QString name() const { return QString(); }
    QString hardwareAddress() const { return QString(); }
};

// === QNetworkProxy stub ===
class QNetworkProxy {
public:
    enum ProxyType { NoProxy, DefaultProxy, Socks5Proxy, HttpProxy };
    void setType(ProxyType type) { (void)type; }
    void setHostName(const QString &host) { (void)host; }
    void setPort(int port) { (void)port; }
    void setUser(const QString &user) { (void)user; }
    void setPassword(const QString &password) { (void)password; }
};

// === QCoreApplication stub ===
class QCoreApplication {
public:
    static QString applicationDirPath() {
        return QString("/data/data/org.telegram.messenger/files");
    }
    static QString applicationFilePath() {
        return QString("/data/data/org.telegram.messenger/files/app");
    }
    static QString organizationName() { return QString("Cryptogram"); }
    static QString applicationName() { return QString("Cryptogram"); }
    static QString applicationVersion() { return QString("1.1.0"); }
    static QStringList arguments() { return {}; }
    static void processEvents() {}
    static void exit(int code = 0) { (void)code; }
    static void quit() {}
    static QCoreApplication *instance() { return nullptr; }
};

// === QtConcurrent stub ===
namespace QtConcurrent {
    template<typename F>
    auto run(F &&func) -> std::future<decltype(func())> {
        return std::async(std::launch::async, std::forward<F>(func));
    }
}

// === QAudioDevice stub ===
class QAudioDevice {
public:
    QString id() const { return QString(); }
    QString description() const { return QString(); }
    bool isNull() const { return true; }
    int minimumSampleRate() const { return 8000; }
    int maximumSampleRate() const { return 48000; }
    int minimumChannelCount() const { return 1; }
    int maximumChannelCount() const { return 2; }
};

// === QAudioFormat stub ===
class QAudioFormat {
public:
    void setSampleRate(int rate) { _sampleRate = rate; }
    int sampleRate() const { return _sampleRate; }
    void setChannelCount(int count) { _channels = count; }
    int channelCount() const { return _channels; }
    void setSampleSize(int size) { _sampleSize = size; }
    int sampleSize() const { return _sampleSize; }
    enum SampleType { Unknown, SignedInt, UnSignedInt, Float };
    void setSampleType(SampleType type) { _type = type; }
    SampleType sampleType() const { return _type; }
    enum ByteOrder { LittleEndian, BigEndian };
    void setByteOrder(ByteOrder order) { _byteOrder = order; }
    ByteOrder byteOrder() const { return _byteOrder; }
    bool isValid() const { return _sampleRate > 0; }
private:
    int _sampleRate = 44100;
    int _channels = 1;
    int _sampleSize = 16;
    SampleType _type = SignedInt;
    ByteOrder _byteOrder = LittleEndian;
};

// === QMediaDevices stub ===
class QMediaDevices {
public:
    static QList<QAudioDevice> audioInputs() { return {}; }
    static QList<QAudioDevice> audioOutputs() { return {}; }
    static QAudioDevice defaultAudioInput() { return QAudioDevice(); }
    static QAudioDevice defaultAudioOutput() { return QAudioDevice(); }
};

// === QList alias defined at top of file ===

// === QHash stub ===
template<typename K, typename V>
class QHash : public std::map<K, V> {
public:
    using std::map<K, V>::map;
    bool contains(const K &key) const { return this->find(key) != this->end(); }
    V value(const K &key, const V &def = V()) const {
        auto it = this->find(key);
        return it != this->end() ? it->second : def;
    }
    void remove(const K &key) { this->erase(key); }
};

// === QMultiMap stub ===
template<typename K, typename V>
class QMultiMap : public std::multimap<K, V> {
public:
    using std::multimap<K, V>::multimap;
    void insertMulti(const K &key, const V &value) { this->insert({key, value}); }
};

// === QVariant stub ===
class QVariant {
public:
    QVariant() = default;
    QVariant(int v) : _int(v), _type(Int) {}
    QVariant(long long v) : _ll(v), _type(LongLong) {}
    QVariant(double v) : _double(v), _type(Double) {}
    QVariant(bool v) : _bool(v), _type(Bool) {}
    QVariant(const QString &v) : _string(v), _type(String) {}
    QVariant(const QByteArray &v) : _string(v.toStdString()), _type(ByteArray) {}
    QVariant(const char *v) : _string(v ? v : ""), _type(String) {}

    bool isNull() const { return _type == Invalid; }
    bool isValid() const { return _type != Invalid; }
    int toInt(bool *ok = nullptr) const { if (ok) *ok = true; return _int; }
    long long toLongLong(bool *ok = nullptr) const { if (ok) *ok = true; return _ll; }
    double toDouble(bool *ok = nullptr) const { if (ok) *ok = true; return _double; }
    bool toBool() const { return _bool; }
    QString toString() const { return _string; }
    QByteArray toByteArray() const { return QByteArray(_string.toStdString()); }

    enum Type { Invalid, Int, LongLong, Double, Bool, String, ByteArray };
private:
    int _int = 0;
    long long _ll = 0;
    double _double = 0;
    bool _bool = false;
    QString _string;
    Type _type = Invalid;
};

// === QVariantMap ===
using QVariantMap = std::map<QString, QVariant>;

// === QSettings stub (file-based) ===
class QSettings {
public:
    enum Format { NativeFormat, IniFormat };
    enum Status { NoError, FormatError, AccessError };

    QSettings() = default;
    QSettings(const QString &organization, const QString &application) {
        (void)organization; (void)application;
    }
    QSettings(const QString &filename, Format format) : _filename(filename) { (void)format; }

    void setValue(const QString &key, const QVariant &value) {
        _values[key] = value.toString();
    }
    QVariant value(const QString &key, const QVariant &def = QVariant()) const {
        auto it = _values.find(key);
        if (it != _values.end()) return QVariant(it->second);
        return def;
    }
    bool contains(const QString &key) const { return _values.find(key) != _values.end(); }
    void remove(const QString &key) { _values.erase(key); }
    void clear() { _values.clear(); }
    void sync() {}
    Status status() const { return NoError; }
    QStringList allKeys() const {
        QStringList keys;
        for (const auto &kv : _values) keys.append(kv.first);
        return keys;
    }
    void beginGroup(const QString &prefix) { _group = prefix; }
    void endGroup() { _group.clear(); }
private:
    QString _filename;
    QString _group;
    std::map<QString, QString> _values;
};

// === QEventLoop stub ===
class QEventLoop {
public:
    void exec() {}
    void quit() {}
    void exit(int code = 0) { (void)code; }
};

// === QTimerEvent stub ===
class QTimerEvent {
public:
    explicit QTimerEvent(int timerId) : _timerId(timerId) {}
    int timerId() const { return _timerId; }
private:
    int _timerId;
};

// === QThreadPool stub ===
class QThreadPool {
public:
    static QThreadPool *globalInstance() { static QThreadPool pool; return &pool; }
    void start(std::function<void()> task) { std::thread(task).detach(); }
    void waitForDone() {}
    int maxThreadCount() const { return static_cast<int>(std::thread::hardware_concurrency()); }
};

// === QFuture stub ===
template<typename T>
class QFuture {
public:
    QFuture() = default;
    explicit QFuture(std::future<T> &&f) : _future(std::move(f)) {}
    bool isFinished() const { return !_future.valid() || _future.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }
    T result() const { return _future.get(); }
    void waitForFinished() { if (_future.valid()) _future.wait(); }
private:
    mutable std::future<T> _future;
};

// === QReadWriteLock already defined above ===

// === Signal/slot connect stubs ===
template<typename Sender, typename Signal, typename Receiver, typename Slot>
inline void connect(Sender *, Signal, Receiver *, Slot) {}
template<typename Sender, typename Signal, typename Functor>
inline void connect(Sender *, Signal, Functor) {}

// === qMax/qMin/qAbs shims ===
template<typename T> inline T qMax(const T &a, const T &b) { return std::max(a, b); }
template<typename T> inline T qMin(const T &a, const T &b) { return std::min(a, b); }
template<typename T> inline T qAbs(const T &a) { return std::abs(a); }
template<typename T> inline T qBound(const T &min, const T &val, const T &max) { return std::max(min, std::min(val, max)); }
template<typename T> inline void qSwap(T &a, T &b) { std::swap(a, b); }

// === qint/quint typedefs defined at top of file ===

// === QFlags stub ===
template<typename Enum>
class QFlags {
public:
    using Int = std::underlying_type_t<Enum>;
    QFlags() = default;
    QFlags(Enum flag) : _value(static_cast<Int>(flag)) {}
    QFlags(Int value) : _value(value) {}
    operator Int() const { return _value; }
    QFlags &operator|=(Enum f) { _value |= static_cast<Int>(f); return *this; }
    QFlags &operator&=(Int mask) { _value &= mask; return *this; }
    bool testFlag(Enum flag) const { return (_value & static_cast<Int>(flag)) == static_cast<Int>(flag); }
private:
    Int _value = 0;
};

// === Q_DISABLE_COPY already defined above ===

// === qWarning/qFatal/qDebug macros ===
#define qWarning(...) qDebug()
#define qFatal(...) qDebug()
#define qInfo(...) qDebug()
#define qCritical(...) qDebug()

// === Qt namespace enums ===
namespace Qt {
    enum AlignmentFlag { AlignLeft = 1, AlignRight = 2, AlignCenter = 4, AlignTop = 8, AlignBottom = 16, AlignVCenter = 32, AlignHCenter = 64 };
    enum Orientation { Horizontal = 1, Vertical = 2 };
    enum CheckState { Unchecked = 0, PartiallyChecked = 1, Checked = 2 };
    enum ItemDataRole { DisplayRole = 0, DecorationRole = 1, EditRole = 2, ToolTipRole = 3, UserRole = 0x100 };
    enum WindowState { WindowNoState = 0, WindowMinimized = 1, WindowMaximized = 2, WindowFullScreen = 4 };
    enum GlobalColor { white, black, red, green, blue, cyan, magenta, yellow, gray };
    enum Key { Key_Escape = 0x01000000, Key_Return = 0x01000004, Key_Enter = 0x01000005, Key_Space = 0x20 };
    enum MouseButton { NoButton = 0, LeftButton = 1, RightButton = 2, MiddleButton = 4 };
    enum WindowFlags { Widget = 0, Window = 1, Dialog = 2, SplashScreen = 4 };
    enum FocusPolicy { NoFocus = 0, TabFocus = 1, ClickFocus = 2, StrongFocus = 3, WheelFocus = 4 };
    enum ConnectionType { AutoConnection = 0, DirectConnection = 1, QueuedConnection = 2, BlockingQueuedConnection = 3 };
    enum AspectRatioMode { IgnoreAspectRatio = 0, KeepAspectRatio = 1, KeepAspectRatioByExpanding = 2 };
    enum TransformationMode { FastTransformation = 0, SmoothTransformation = 1 };
    enum ScrollBarPolicy { ScrollBarAsNeeded = 0, ScrollBarAlwaysOff = 1, ScrollBarAlwaysOn = 2 };
    enum ContextMenuPolicy { DefaultContextMenu = 0, NoContextMenu = 1, PreventContextMenu = 2, ActionsContextMenu = 3, CustomContextMenu = 4 };
    enum PenStyle { NoPen = 0, SolidLine = 1, DashLine = 2, DotLine = 3, DashDotLine = 4, DashDotDotLine = 5 };
    enum BrushStyle { NoBrush = 0, SolidPattern = 1 };
    enum FillRule { OddEvenFill = 0, WindingFill = 1 };
    enum WidgetAttribute { WA_TranslucentBackground = 120, WA_TransparentForMouseEvents = 51, WA_OpaquePaintEvent = 4 };
    enum SortOrder { AscendingOrder = 0, DescendingOrder = 1 };
    enum CursorShape { ArrowCursor = 0, UpArrowCursor = 1, CrossCursor = 2, WaitCursor = 3, IBeamCursor = 4, PointingHandCursor = 13 };
    enum TextFormat { PlainText = 0, RichText = 1, AutoText = 2, MarkdownText = 3 };
    enum ItemFlag { NoItemFlags = 0, ItemIsSelectable = 1, ItemIsEditable = 2, ItemIsDragEnabled = 4, ItemIsDropEnabled = 8, ItemIsUserCheckable = 16, ItemIsEnabled = 32, ItemIsTristate = 64 };
    enum WindowModality { NonModal = 0, WindowModal = 1, ApplicationModal = 2 };
    enum ImageConversionFlag { AutoColor = 0, ColorOnly = 3, MonoOnly = 8 };
    enum PenCapStyle { FlatCap = 0, SquareCap = 0x10, RoundCap = 0x20 };
    enum PenJoinStyle { MiterJoin = 0, BevelJoin = 0x40, RoundJoin = 0x80, SvgMiterJoin = 0x100 };
    enum Axis { XAxis = 0, YAxis = 1, ZAxis = 2 };
    enum ScreenOrientation { PrimaryOrientation = 0, PortraitOrientation = 1, LandscapeOrientation = 2, InvertedPortraitOrientation = 4, InvertedLandscapeOrientation = 8 };
    enum ApplicationState { ApplicationSuspended = 0, ApplicationHidden = 1, ApplicationInactive = 2, ApplicationActive = 4 };
    enum EnterKeyType { EnterKeyDefault, EnterKeyReturn, EnterKeyDone, EnterKeyGo, EnterKeySend, EnterKeySearch, EnterKeyNext, EnterKeyPrevious };
    enum ScrollPhase { NoScrollPhase = 0, ScrollBegin = 1, ScrollUpdate = 2, ScrollEnd = 3 };
    enum MouseEventSource { MouseEventNotSynthesized = 0, MouseEventSynthesizedBySystem = 1, MouseEventSynthesizedByQt = 2, MouseEventSynthesizedByApplication = 3 };
    enum TileMode { StretchTile = 0, RepeatTile = 1, RoundTile = 2 };
    enum ClipOperation { NoClip = 0, ReplaceClip = 1, IntersectClip = 2, UniteClip = 3 };
    enum DropAction { CopyAction = 1, MoveAction = 2, LinkAction = 4, IgnoreAction = 0, TargetMoveAction = 0x802 };
    enum CaseSensitivity { CaseInsensitive = 0, CaseSensitive = 1 };
    enum MatchFlag { MatchExactly = 0, MatchContains = 1, MatchStartsWith = 2, MatchEndsWith = 3, MatchRegExp = 4, MatchWildcard = 5, MatchFixedString = 8, MatchCaseSensitive = 16, MatchRecursive = 64 };
    enum SplitBehavior { KeepEmptyParts = 0, SkipEmptyParts = 1 };
    enum ItemSelectionOperation { ReplaceSelection = 0, AddToSelection = 1, ToggleSelection = 2, RemoveFromSelection = 3 };
    enum GestureState { NoGesture = 0, GestureStarted = 1, GestureUpdated = 2, GestureFinished = 3, GestureCanceled = 4 };
    enum GestureType { TapGesture = 1, TapAndHoldGesture = 2, PanGesture = 3, PinchGesture = 4, SwipeGesture = 5, CustomGesture = 0x100 };
    enum NativeGestureType { BeginNativeGesture = 0, EndNativeGesture = 1, PanNativeGesture = 2, ZoomNativeGesture = 3, RotateNativeGesture = 4, SwipeNativeGesture = 5 };
    enum TimerType { PreciseTimer = 0, CoarseTimer = 1, VeryCoarseTimer = 2 };
    enum ProcessEventsFlag { AllEvents = 0, ExcludeUserInputEvents = 1, ExcludeSocketNotifiers = 2, WaitForMoreEvents = 4, EventLoopExec = 8 };
    enum ApplicationAttribute { AA_ImmediateCallWorkaround = 1, AA_DontCreateNativeWidgetSiblings = 2, AA_X11InitThreads = 4, AA_ShareOpenGLContexts = 8, AA_CompressHighFrequencyEvents = 16, AA_UseHighDpiPixmaps = 32, AA_ForceRasterWidgets = 64, AA_DontUseNativeMenuBar = 128, AA_DontUseNativeDialogs = 256, AA_SynthesizeMouseForUnhandledTabletEvents = 512, AA_SynthesizeMouseForUnhandledTouchEvents = 1024, AA_Use96Dpi = 2048, AA_CompressTabletEvents = 4096, AA_DisableWindowContextHelpButton = 8192, AA_DisableSessionManager = 16384, AA_UseOpenGLES = 32768, AA_UseSoftwareOpenGL = 65536, AA_ShareMenuAndToolbars = 131072, AA_SetPalette = 262144 };
}

// === QSizePolicy stub ===
class QSizePolicy {
public:
    enum Policy { Fixed = 0, Minimum = 1, Maximum = 2, Preferred = 3, Expanding = 4, MinimumExpanding = 5, Ignored = 6 };
    void setHorizontalPolicy(Policy p) { (void)p; }
    void setVerticalPolicy(Policy p) { (void)p; }
};

// === QColor stub ===
class QColor {
public:
    QColor() = default;
    QColor(int r, int g, int b, int a = 255) : _r(r), _g(g), _b(b), _a(a) {}
    int red() const { return _r; }
    int green() const { return _g; }
    int blue() const { return _b; }
    int alpha() const { return _a; }
    bool isValid() const { return _a > 0; }
    void setRgb(int r, int g, int b) { _r = r; _g = g; _b = b; }
    static QColor fromRgb(int r, int g, int b, int a = 255) { return QColor(r, g, b, a); }
private:
    int _r = 0, _g = 0, _b = 0, _a = 255;
};

// === QFont stub ===
class QFont {
public:
    void setPointSize(int size) { _size = size; }
    int pointSize() const { return _size; }
    void setBold(bool bold) { _bold = bold; }
    bool bold() const { return _bold; }
    void setFamily(const QString &family) { _family = family; }
    QString family() const { return _family; }
private:
    int _size = 12;
    bool _bold = false;
    QString _family;
};

// === QSize stub ===
class QSize {
public:
    QSize() = default;
    QSize(int w, int h) : _w(w), _h(h) {}
    int width() const { return _w; }
    int height() const { return _h; }
    void setWidth(int w) { _w = w; }
    void setHeight(int h) { _h = h; }
    bool isEmpty() const { return _w <= 0 || _h <= 0; }
    bool isValid() const { return _w > 0 && _h > 0; }
    QSize scaled(int w, int h) const { return QSize(w, h); }
private:
    int _w = 0, _h = 0;
};

// === QRect stub ===
class QRect {
public:
    QRect() = default;
    QRect(int x, int y, int w, int h) : _x(x), _y(y), _w(w), _h(h) {}
    int x() const { return _x; }
    int y() const { return _y; }
    int width() const { return _w; }
    int height() const { return _h; }
    int left() const { return _x; }
    int top() const { return _y; }
    int right() const { return _x + _w - 1; }
    int bottom() const { return _y + _h - 1; }
    void setX(int x) { _x = x; }
    void setY(int y) { _y = y; }
    void setWidth(int w) { _w = w; }
    void setHeight(int h) { _h = h; }
    bool isEmpty() const { return _w <= 0 || _h <= 0; }
    bool isValid() const { return _w > 0 && _h > 0; }
    bool contains(int px, int py) const { return px >= _x && px < _x + _w && py >= _y && py < _y + _h; }
private:
    int _x = 0, _y = 0, _w = 0, _h = 0;
};

// === QPoint stub ===
class QPoint {
public:
    QPoint() = default;
    QPoint(int x, int y) : _x(x), _y(y) {}
    int x() const { return _x; }
    int y() const { return _y; }
    void setX(int x) { _x = x; }
    void setY(int y) { _y = y; }
private:
    int _x = 0, _y = 0;
};

// === QPointF stub ===
class QPointF {
public:
    QPointF() = default;
    QPointF(double x, double y) : _x(x), _y(y) {}
    double x() const { return _x; }
    double y() const { return _y; }
    void setX(double x) { _x = x; }
    void setY(double y) { _y = y; }
private:
    double _x = 0, _y = 0;
};

// === QLine stub ===
class QLine {
public:
    QLine() = default;
    QLine(int x1, int y1, int x2, int y2) : _x1(x1), _y1(y1), _x2(x2), _y2(y2) {}
    int x1() const { return _x1; }
    int y1() const { return _y1; }
    int x2() const { return _x2; }
    int y2() const { return _y2; }
private:
    int _x1 = 0, _y1 = 0, _x2 = 0, _y2 = 0;
};

// === QImage stub ===
class QImage {
public:
    enum Format { Format_Invalid, Format_Mono, Format_MonoLSB, Format_Indexed8, Format_RGB32, Format_ARGB32, Format_ARGB32_Premultiplied, Format_RGB16, Format_RGB888, Format_RGBA8888 };
    QImage() = default;
    QImage(int width, int height, Format format) : _w(width), _h(height), _format(format) {}
    int width() const { return _w; }
    int height() const { return _h; }
    Format format() const { return _format; }
    bool isNull() const { return _w == 0 || _h == 0; }
    unsigned char *bits() { return _data.data(); }
    const unsigned char *constBits() const { return _data.data(); }
    void setPixel(int x, int y, unsigned int value) { (void)x; (void)y; (void)value; }
    unsigned int pixel(int x, int y) const { (void)x; (void)y; return 0; }
    QSize size() const { return QSize(_w, _h); }
    QRect rect() const { return QRect(0, 0, _w, _h); }
    QImage scaled(int w, int h) const { (void)w; (void)h; return *this; }
    QImage scaled(const QSize &s) const { return scaled(s.width(), s.height()); }
    bool save(const QString &path) const { (void)path; return false; }
    static QImage fromData(const QByteArray &data) { (void)data; return QImage(); }
private:
    int _w = 0, _h = 0;
    Format _format = Format_Invalid;
    std::vector<unsigned char> _data;
};

// === QProcess stub ===
class QProcess : public QObject {
public:
    QProcess(QObject *parent = nullptr) : QObject(parent) {}
    ~QProcess() = default;

    void start(const QString &program, const QStringList &args = {}) {
        (void)program; (void)args;
    }
    bool waitForStarted(int msecs = 30000) { (void)msecs; return true; }
    bool waitForFinished(int msecs = 30000) { (void)msecs; return true; }
    bool waitForReadyRead(int msecs = 30000) { (void)msecs; return false; }
    QByteArray readAllStandardOutput() { return QByteArray(); }
    QByteArray readAllStandardError() { return QByteArray(); }
    QString program() const { return _program; }
    void setProgram(const QString &program) { _program = program; }
    QStringList arguments() const { return _args; }
    void setArguments(const QStringList &args) { _args = args; }
    int exitCode() const { return 0; }
    bool atEnd() const { return true; }

    static QString execute(const QString &program, const QStringList &args = {}) {
        (void)program; (void)args;
        return QString();
    }
    static QString startDetached(const QString &program, const QStringList &args = {}) {
        (void)program; (void)args;
        return QString();
    }

    enum ProcessState { NotRunning, Starting, Running };
    ProcessState state() const { return NotRunning; }

    std::function<void()> readyReadStandardOutput;
    std::function<void()> readyReadStandardError;
    std::function<void(int)> finished;
private:
    QString _program;
    QStringList _args;
};

// === QUrl stub ===
class QUrl {
public:
    QUrl() = default;
    QUrl(const QString &url) : _url(url) {}
    QString toString() const { return _url; }
    QString toDisplayString() const { return _url; }
    QString host() const { return _url; }
    QString path() const { return _url; }
    bool isValid() const { return !_url.isEmpty(); }
    bool isEmpty() const { return _url.isEmpty(); }
    static QUrl fromLocalFile(const QString &path) { return QUrl(path); }
    QString toLocalFile() const { return _url; }
private:
    QString _url;
};

// === QMessageLogger stub ===
class QMessageLogger {
public:
    QMessageLogger() = default;
    QMessageLogger(const char *, int, const char *) {}
    template<typename... Args> void debug(const char *, Args...) {}
    template<typename... Args> void info(const char *, Args...) {}
    template<typename... Args> void warning(const char *, Args...) {}
    template<typename... Args> void critical(const char *, Args...) {}
};

// === QLoggingCategory stub ===
class QLoggingCategory {
public:
    explicit QLoggingCategory(const char *name) : _name(name) {}
    const char *categoryName() const { return _name; }
    bool isDebugEnabled() const { return true; }
    bool isInfoEnabled() const { return true; }
    bool isWarningEnabled() const { return true; }
    bool isCriticalEnabled() const { return true; }
private:
    const char *_name;
};

#define Q_LOGGING_CATEGORY(name, ...) static QLoggingCategory name(__VA_ARGS__);

// === QSemaphore stub ===
class QSemaphore {
public:
    explicit QSemaphore(int n = 0) : _count(n) {}
    void acquire(int n = 1) {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock, [this, n]() { return _count >= n; });
        _count -= n;
    }
    void release(int n = 1) {
        std::lock_guard<std::mutex> lock(_mutex);
        _count += n;
        _cond.notify_all();
    }
    bool tryAcquire(int n = 1) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_count < n) return false;
        _count -= n;
        return true;
    }
    int available() const { return _count.load(); }
private:
    std::mutex _mutex;
    std::condition_variable _cond;
    std::atomic<int> _count{0};
};

// === QWaitCondition stub ===
class QWaitCondition {
public:
    void wait(QMutex *mutex) { (void)mutex; }
    void wakeOne() {}
    void wakeAll() {}
};

// === QAtomicInt stub ===
class QAtomicInt {
public:
    QAtomicInt(int value = 0) : _value(value) {}
    operator int() const { return _value.load(); }
    QAtomicInt &operator=(int v) { _value = v; return *this; }
    bool ref() { return ++_value > 0; }
    bool deref() { return --_value > 0; }
    int load() const { return _value.load(); }
    void store(int v) { _value = v; }
    bool testAndSetOrdered(int expected, int newVal) { return _value.compare_exchange_strong(expected, newVal); }
    int fetchAndAddOrdered(int valueToAdd) { return _value.fetch_add(valueToAdd); }
private:
    std::atomic<int> _value;
};

// === QSharedPointer stub ===
template<typename T>
using QSharedPointer = std::shared_ptr<T>;

template<typename T, typename... Args>
QSharedPointer<T> QSharedPointer_make(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// === QWeakPointer stub ===
template<typename T>
using QWeakPointer = std::weak_ptr<T>;

// === QScopedPointer stub ===
template<typename T>
class QScopedPointer {
public:
    explicit QScopedPointer(T *p = nullptr) : _ptr(p) {}
    ~QScopedPointer() { delete _ptr; }
    T *data() const { return _ptr; }
    T *operator->() const { return _ptr; }
    T &operator*() const { return *_ptr; }
    operator bool() const { return _ptr != nullptr; }
    T *take() { T *p = _ptr; _ptr = nullptr; return p; }
    void reset(T *p = nullptr) { delete _ptr; _ptr = p; }
private:
    Q_DISABLE_COPY(QScopedPointer)
    T *_ptr;
};

// === QEnableMoveForQScopedPointer ===
#define Q_DECLARE_OPAQUE_POINTER(PointerType)

// === QNonConstOverload / qOverload stubs ===
template<typename R, typename... Args>
auto qOverload(R (*func)(Args...)) { return func; }

// === Q_DECL_DEPRECATED ===
#define Q_DECL_DEPRECATED
#define Q_DECL_DEPRECATED_X(msg)

// === Q_RETURN_ARG ===
#define Q_RETURN_ARG(type) type{}

// === QMetaObject stub ===
namespace QMetaObject {
    struct Connection {};
    inline bool invokeMethod(QObject *, const char *, ...) { return false; }
}

// === QAccessible stub ===
namespace QAccessible {
    enum Role { NoRole = 0, Client = 1 };
    enum Event { NoEvent = 0 };
}

// === QPlatformNativeInterface stub ===
class QPlatformNativeInterface {};

// === QPA headers ===
namespace QPA {
    inline void *platformNativeInterface() { return nullptr; }
}

// === QSslSocket stub ===
class QSslSocket {
public:
    void connectToHostEncrypted(const QString &, int) {}
    bool waitForEncrypted(int = 30000) { return false; }
    void disconnectFromHost() {}
    bool isOpen() const { return false; }
};

// === QHostAddress stub ===
class QHostAddress {
public:
    QHostAddress() = default;
    explicit QHostAddress(const QString &) {}
    QString toString() const { return QString(); }
};

// === QHostInfo stub ===
class QHostInfo {
public:
    static QHostInfo fromName(const QString &) { return QHostInfo(); }
    QStringList addresses() const { return {}; }
    QString hostName() const { return QString(); }
    int error() const { return 0; }
};

// === QDnsLookup stub ===
class QDnsLookup {
public:
    enum Type { A = 1, AAAA = 28, CNAME = 5, MX = 15, NS = 2, TXT = 16, SRV = 33 };
    void setType(Type type) { (void)type; }
    void setName(const QString &name) { (void)name; }
    void lookup() {}
    bool isFinished() const { return true; }
};

// === QSocketNotifier stub ===
class QSocketNotifier : public QObject {
public:
    enum Type { Read = 0, Write = 1, Exception = 2 };
    QSocketNotifier(int, Type, QObject *parent = nullptr) : QObject(parent) {}
    void setEnabled(bool) {}
    bool isEnabled() const { return false; }
};

// === QTranslate stub ===
class QTranslator : public QObject {
public:
    explicit QTranslator(QObject *parent = nullptr) : QObject(parent) {}
    bool load(const QString &) { return false; }
    QString translate(const char *, const char *, ...) const { return QString(); }
};

// === QLibrary stub ===
class QLibrary {
public:
    explicit QLibrary(const QString &name) : _name(name) {}
    explicit QLibrary(const QString &name, int ver) : _name(name), _ver(ver) { (void)_ver; }
    bool load() { return false; }
    bool isLoaded() const { return false; }
    void unload() {}
    void *resolve(const char *) { return nullptr; }
    QString errorString() const { return QString(); }
    void setFileName(const QString &name) { _name = name; }
    QString fileName() const { return _name; }
private:
    QString _name;
    int _ver = 0;
};

// === QFileInfoList ===
using QFileInfoList = QList<QFileInfo>;

// === QDir additional methods ===
inline QStringList QDir::entryList(const QStringList &filters, int sort, int filter) const {
    (void)filters; (void)sort; (void)filter;
    QStringList result;
    DIR *dir = opendir(_path.toStdString().c_str());
    if (!dir) return result;
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == '.') continue;
        result.append(QString(entry->d_name));
    }
    closedir(dir);
    return result;
}

// === QDesktopServices stub ===
class QDesktopServices {
public:
    static bool openUrl(const QUrl &url) { (void)url; return false; }
    static void setUrlHandler(const QString &scheme, QObject *receiver, const char *method) {
        (void)scheme; (void)receiver; (void)method;
    }
};

// === QMimeDatabase stub ===
class QMimeType {
public:
    QString name() const { return QString(); }
    QString comment() const { return QString(); }
    QStringList suffixes() const { return {}; }
};
class QMimeDatabase {
public:
    QMimeType mimeTypeForFile(const QString &) const { return QMimeType(); }
    QMimeType mimeTypeForData(const QByteArray &) const { return QMimeType(); }
    QMimeType mimeTypeForUrl(const QUrl &) const { return QMimeType(); }
};

// === QSaveFile stub ===
class QSaveFile {
public:
    explicit QSaveFile(const QString &path) : _path(path) {}
    bool open(int) { return true; }
    void write(const QByteArray &) {}
    void commit() {}
    void cancelWriting() {}
    QString fileName() const { return _path; }
private:
    QString _path;
};

// === QStorageInfo stub ===
class QStorageInfo {
public:
    static QList<QStorageInfo> mountedVolumes() { return {}; }
    QString rootPath() const { return QString(); }
    long long bytesAvailable() const { return 0; }
    long long bytesTotal() const { return 0; }
};

// === QProcessEnvironment stub ===
class QProcessEnvironment {
public:
    void insert(const QString &name, const QString &value) { (void)name; (void)value; }
    QString value(const QString &name, const QString &def = QString()) const { (void)name; return def; }
    bool contains(const QString &name) const { (void)name; return false; }
    static QProcessEnvironment systemEnvironment() { return QProcessEnvironment(); }
};

// === QDeadlineTimer stub ===
class QDeadlineTimer {
public:
    QDeadlineTimer() = default;
    explicit QDeadlineTimer(long long msecs) : _msecs(msecs) {}
    bool hasExpired() const { return false; }
    long long remainingTime() const { return _msecs; }
private:
    long long _msecs = 0;
};

// === QPointer stub ===
template<typename T>
class QPointer {
public:
    QPointer() = default;
    QPointer(T *p) : _ptr(p) {}
    T *data() const { return _ptr; }
    T *operator->() const { return _ptr; }
    T &operator*() const { return *_ptr; }
    operator bool() const { return _ptr != nullptr; }
    bool isNull() const { return _ptr == nullptr; }
    void clear() { _ptr = nullptr; }
private:
    T *_ptr = nullptr;
};

// === QEnableIfForQTypeTraits ===
template<bool B, typename T = void>
using QEnableIfIf = std::enable_if<B, T>;

// === QHashFunctions stub ===
inline uint qHash(const QString &key) {
    return static_cast<uint>(std::hash<std::string>{}(key.toStdString()));
}
inline uint qHash(int key) { return static_cast<uint>(key); }

// === QInitializerList ===
#define Q_INITIALIZER_LIST(x) {x}

// === Q_DECL_CONSTEXPR ===
#define Q_DECL_CONSTEXPR constexpr
#define Q_DECL_RELAXED_CONSTEXPR
#define Q_DECL_EQ_DEFAULT = default
#define Q_DECL_EQ_DELETE = delete

// === Q_ENUM_NS ===
#define Q_ENUM_NS(x)

// === Q_NAMESPACE_EXPORT ===
#define Q_NAMESPACE_EXPORT

// === Q_CLASSINFO ===
#undef Q_CLASSINFO
#define Q_CLASSINFO(name, value)

// === Q_PLUGIN_METADATA ===
#undef Q_PLUGIN_METADATA
#define Q_PLUGIN_METADATA(x)

// === Q_INTERFACES ===
#undef Q_INTERFACES
#define Q_INTERFACES(x)

// === Q_REVISION ===
#define Q_REVISION(x)

// === Q_SET_OBJECT_NAME ===
#define Q_SET_OBJECT_NAME(x)

// === Q_DO_NOT_GENERATE ===
#define Q_DO_NOT_GENERATE

// === QtAndroidExtras stub ===
namespace QtAndroid {
    inline void *androidActivity() { return nullptr; }
    inline void runOnAndroidThread(std::function<void()> func) { func(); }
}

// === QJniObject stub (Qt6 style) ===
class QJniObject {
public:
    QJniObject() = default;
    QJniObject(const char *) {}
    static QJniObject fromString(const QString &) { return QJniObject(); }
    QString toString() const { return QString(); }
    template<typename... Args> static QJniObject callStaticObjectMethod(const char *, const char *, Args...) { return QJniObject(); }
    template<typename... Args> void callMethod(const char *, const char *, Args...) {}
};

// === QAndroidJniObject stub (Qt5 style) ===
using QAndroidJniObject = QJniObject;

// === QAndroidActivityResultReceiver stub ===
class QAndroidActivityResultReceiver {};

// === QAndroidService stub ===
class QAndroidService {};

// OpenSSL include moved to top of file
