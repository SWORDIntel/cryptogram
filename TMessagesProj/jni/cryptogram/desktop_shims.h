#pragma once

#include "qt_shims.h"
#include <array>
#include <memory>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <unordered_map>

// Base types
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
#ifndef QT_SHIMS_QUINT_DEFINED
#define QT_SHIMS_QUINT_DEFINED
using quint8 = uint8_t;
using quint16 = uint16_t;
using quint32 = uint32_t;
using quint64 = uint64_t;
#endif
using qulonglong = unsigned long long;
using int32 = int32_t;
using int64 = int64_t;
using uchar = unsigned char;
using MTPint256 = std::array<unsigned char, 32>;

namespace gsl {
    template<typename T> using not_null = T;
}
using gsl::not_null;

// Base namespace
namespace base {
    template<typename T> class RandomValueHelper {
    public:
        operator T() { return static_cast<T>(rand()); }
    };
    template<typename T> inline T RandomValue() { return static_cast<T>(rand()); }
    inline void RandomFill(void *data, size_t size) {
        auto *bytes = static_cast<unsigned char*>(data);
        for (size_t i = 0; i < size; ++i) {
            bytes[i] = static_cast<unsigned char>(rand() & 0xFF);
        }
    }

    namespace unixtime {
        inline long long now() { return std::time(nullptr); }
    }

    template<typename K, typename V> using flat_map = std::map<K, V>;
    template<typename T> using flat_set = std::set<T>;

    inline QString& GlobalStoragePath() {
        static QString path = "/sdcard/cryptogram";
        return path;
    }

    inline void SetGlobalStoragePath(const QString& path) {
        GlobalStoragePath() = path;
    }
}

// Data namespace
namespace Data {
    struct UserId {
        uint64 value = 0;
        UserId() = default;
        explicit UserId(uint64 v) : value(v) {}
        uint64 valueOf() const { return value; }
        friend bool operator<(const UserId &lhs, const UserId &rhs) { return lhs.value < rhs.value; }
        friend bool operator==(const UserId &lhs, const UserId &rhs) { return lhs.value == rhs.value; }
        friend bool operator!=(const UserId &lhs, const UserId &rhs) { return !(lhs == rhs); }
    };
    struct PeerId {
        uint64 value = 0;
        PeerId() = default;
        explicit PeerId(uint64 v) : value(v) {}
        uint64 valueOf() const { return value; }
        friend bool operator<(const PeerId &lhs, const PeerId &rhs) { return lhs.value < rhs.value; }
        friend bool operator==(const PeerId &lhs, const PeerId &rhs) { return lhs.value == rhs.value; }
        friend bool operator!=(const PeerId &lhs, const PeerId &rhs) { return !(lhs == rhs); }
    };
    class UserData {
    public:
        UserId id;
    };
    class HistoryItem;
    using MsgId = int32;
    using TimeId = int32;

    class Session {
    public:
        class HistoryProxy {
        public:
            template <typename... Args>
            HistoryItem *addNewLocalMessage(Args&&...) {
                return nullptr;
            }
        };
        class DataProxy {
        public:
            HistoryProxy *history(PeerId) {
                static HistoryProxy history;
                return &history;
            }
        };
        struct Local {
            QString basePath() { return base::GlobalStoragePath(); }
        };
        Local local() { return Local(); }
        UserId userId() { return UserId(0); }
        DataProxy &data() { static DataProxy data; return data; }
    };

    class PeerData {
    public:
        PeerId id;
        explicit PeerData(PeerId peerId) : id(peerId) {}

        static PeerData* from(Session* s, PeerId peerId) {
            (void)s;
            static std::unordered_map<uint64, std::unique_ptr<PeerData>> peers;
            auto &entry = peers[peerId.value];
            if (!entry) {
                entry = std::make_unique<PeerData>(peerId);
            }
            return entry.get();
        }
    };

    struct EntityType {
        int value = 0;
        EntityType() = default;
        explicit EntityType(int v) : value(v) {}
        operator int() const { return value; }
    };
    struct EntityInText {
        EntityInText(EntityType t, int o, int l, QString d) {}
        int offset() const { return 0; }
        int length() const { return 0; }
        QString data() const { return ""; }
        EntityType type() const { return {}; }
        void shiftRight(int n) {}
    };
    
    struct TextWithEntities {
        QString text;
        std::vector<EntityInText> entities;
    };

    class HistoryItem {
    public:
        MsgId id;
        int32 date() const { return 0; }
        TextWithEntities originalText() const { return {}; }
    };

    struct MessageFlags {};
    inline UserId peerToUser(PeerId peerId) { return UserId(peerId.value); }
}

using namespace Data;

// Core namespace
namespace Core {
    class App {
    public:
        static App& instance() { static App a; return a; }
        uint64 deviceId() { return 12345; }
    };
}
inline Core::App& App() { return Core::App::instance(); }

// Storage namespace
namespace storage {
}

// History namespace
namespace history {
}

// Main namespace
namespace main {
    class Session;
    class Account;
}

// === base::Timer stub ===
namespace base {
#ifndef BASE_TIMER_DEFINED
#define BASE_TIMER_DEFINED
    class Timer {
    public:
        Timer() = default;
        template<typename Func> explicit Timer(Func &&callback) : _callback(std::forward<Func>(callback)) {}
        void callOnce(int msec) { (void)msec; }
        void cancel() {}
        bool isActive() const { return false; }
        void setCallback(std::function<void()> cb) { _callback = std::move(cb); }
    private:
        std::function<void()> _callback;
    };
#endif // BASE_TIMER_DEFINED

    class DelayedCallTimer {
    public:
        template<typename Func> int call(int msec, Func &&callback) {
            (void)msec;
            if (callback) callback();
            return 0;
        }
        void cancel(int id) { (void)id; }
    };

    inline void call_once(int msec, std::function<void()> fn) {
        (void)msec;
        fn();
    }

    template<typename T>
    inline T take(T &value) {
        T result = std::move(value);
        value = T();
        return result;
    }

    // === PlatformInfo stub ===
    namespace PlatformInfo {
        inline QString deviceModel() { return QString("Android"); }
        inline QString systemVersion() { return QString("Android"); }
        inline QString prettyVersion() { return QString("Android"); }
        inline bool isLinux() { return true; }
        inline bool isWindows() { return false; }
        inline bool isMac() { return false; }
        inline bool isMobile() { return true; }
        inline bool isTablet() { return false; }
        inline int screenRefreshRate() { return 60; }
    }
}

// === crl (crl::time) stub ===
namespace crl {
    using time = int64_t;
    inline time now() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    inline void on_main(std::function<void()> fn) { fn(); }
    inline void on_main_sync(std::function<void()> fn) { fn(); }
}

// === rpl (event system) stub ===
namespace rpl {
    template<typename T>
    class event_stream {
    public:
        void fire(T &&value) { (void)value; }
        template<typename Handler> void subscribe(Handler &&handler) { (void)handler; }
    };

    template<typename T>
    class variable {
    public:
        variable() = default;
        variable(T &&value) : _value(std::move(value)) {}
        T &current() { return _value; }
        const T &current() const { return _value; }
        void assign(T &&value) { _value = std::move(value); }
    private:
        T _value{};
    };

    template<typename T>
    class producer {
    public:
        template<typename Handler> void start(Handler &&handler) { (void)handler; }
    };

    template<typename T>
    inline auto single(T &&value) {
        return producer<T>();
    }

    template<typename T>
    inline auto never() {
        return producer<T>();
    }
}

// === Fn (function wrapper) ===
template<typename Signature>
class Fn;

template<typename R, typename... Args>
class Fn<R(Args...)> {
public:
    Fn() = default;
    Fn(std::function<R(Args...)> fn) : _fn(std::move(fn)) {}
    R operator()(Args... args) const { return _fn ? _fn(std::forward<Args>(args)...) : R(); }
    operator bool() const { return static_cast<bool>(_fn); }
private:
    std::function<R(Args...)> _fn;
};

// === Core namespace expanded ===
namespace Core {
    class Settings {
    public:
        bool value(const QString &key, bool def = false) const { (void)key; return def; }
        void setValue(const QString &key, bool value) { (void)key; (void)value; }
        QString stringValue(const QString &key, const QString &def = QString()) const { (void)key; return def; }
        void setStringValue(const QString &key, const QString &value) { (void)key; (void)value; }
    };

    class Application {
    public:
        static Application &instance() { static Application a; return a; }
        Settings &settings() { return _settings; }
        uint64 deviceId() { return 12345; }
        QString dataPath() { return base::GlobalStoragePath(); }
        void lock() {}
        void unlock() {}
        bool isLocked() const { return false; }
    private:
        Settings _settings;
    };

    inline Application &App() { return Application::instance(); }
}

// === Main namespace expanded ===
namespace Main {
    class Session {
    public:
        Data::Session &data() { static Data::Session d; return d; }
        Data::UserId userId() { return Data::UserId(0); }
        QString basePath() { return base::GlobalStoragePath(); }
    };

    class Account {
    public:
        Session &session() { static Session s; return s; }
        bool loggedIn() const { return true; }
    };
}

// === Ui namespace ===
namespace Ui {
    namespace Toast {
        inline void Show(const QString &text) { (void)text; }
        inline void Show(class QWidget *, const QString &text) { (void)text; }
    }

    struct Show {
        template<typename... Args> void operator()(Args&&...) {}
    };
}

// === Lang namespace ===
namespace Lang {
    inline QString tr(const char *key) { return QString(key); }
    inline QString Translate(const char *key) { return QString(key); }
    class Translator {
    public:
        QString translate(const char *context, const char *sourceText, ...) const {
            (void)context;
            return QString(sourceText);
        }
    };
}

// === Storage namespace ===
namespace Storage {
    class Account {
    public:
        QString basePath() { return base::GlobalStoragePath(); }
    };

    inline QString fileFromName(const QString &name) {
        return base::GlobalStoragePath().toStdString() + "/" + name.toStdString();
    }
}

// === History namespace ===
namespace History {
    class History {
    public:
        Data::PeerId peerId() const { return Data::PeerId(0); }
    };

    class HistoryItem {
    public:
        Data::MsgId id() const { return 0; }
        Data::TimeId date() const { return 0; }
    };
}

// === ChatHelpers namespace ===
namespace ChatHelpers {
    struct ComposeFeatures {
        bool sendDisabled = false;
        bool textDisabled = false;
        bool mediaDisabled = false;
    };
}

// === Data::Photo / Data::Document stubs ===
namespace Data {
    struct PhotoId { uint64 value = 0; };
    struct DocumentId { uint64 value = 0; };

    class Photo {
    public:
        PhotoId id;
        int width() const { return 0; }
        int height() const { return 0; }
    };

    class Document {
    public:
        DocumentId id;
        int size() const { return 0; }
        QString mimeString() const { return QString(); }
        bool isImage() const { return false; }
        bool isVideo() const { return false; }
        bool isAudio() const { return false; }
    };

    struct FileOrigin {
        int type = 0;
        uint64 id = 0;
    };
}

// === Style namespace ===
namespace style {
    struct TextStyle {
        int fontHeight = 16;
        int fontAscent = 12;
        int fontDescent = 4;
    };

    inline TextStyle &defaultTextStyle() {
        static TextStyle s;
        return s;
    }
}

// === base::flags stub ===
namespace base {
    template<typename Enum>
    class flags {
    public:
        using Int = std::underlying_type_t<Enum>;
        flags() = default;
        flags(Enum f) : _value(static_cast<Int>(f)) {}
        flags(Int v) : _value(v) {}
        operator Int() const { return _value; }
        flags &operator|=(Enum f) { _value |= static_cast<Int>(f); return *this; }
        flags &operator&=(Int mask) { _value &= mask; return *this; }
        bool test(Enum f) const { return (_value & static_cast<Int>(f)) == static_cast<Int>(f); }
        flags operator|(Enum f) const { return flags(_value | static_cast<Int>(f)); }
        flags operator&(Int mask) const { return flags(_value & mask); }
    private:
        Int _value = 0;
    };
}

// === base::optional (alias for std::optional) ===
namespace base {
    template<typename T> using optional = std::optional<T>;
}

// === base::weak / base::guarded ===
namespace base {
    template<typename T> using weak = std::weak_ptr<T>;

    template<typename T>
    inline std::shared_ptr<T> guarded(T *ptr) {
        return std::shared_ptr<T>(ptr, [](T *){});
    }
}

// === base::algorithm helpers ===
namespace base {
    template<typename Container, typename Predicate>
    inline void erase_if(Container &c, Predicate &&p) {
        c.erase(std::remove_if(c.begin(), c.end(), std::forward<Predicate>(p)), c.end());
    }

    template<typename Container, typename Value>
    inline auto find(Container &c, const Value &v) {
        return std::find(c.begin(), c.end(), v);
    }

    template<typename Container, typename Predicate>
    inline auto find_if(Container &c, Predicate &&p) {
        return std::find_if(c.begin(), c.end(), std::forward<Predicate>(p));
    }
}

// === MTP types stub ===
namespace MTP {
    using int128 = std::array<unsigned char, 16>;
    using int256 = std::array<unsigned char, 32>;

    struct string {
        QByteArray _data;
        string() = default;
        explicit string(const QByteArray &data) : _data(data) {}
        QByteArray v() const { return _data; }
    };

    template<typename T>
    struct vector {
        QVector<T> _data;
        vector() = default;
        explicit vector(QVector<T> d) : _data(std::move(d)) {}
        QVector<T> v() const { return _data; }
    };
}

// === Platform namespace ===
namespace Platform {
    inline bool IsLinux() { return true; }
    inline bool IsWindows() { return false; }
    inline bool IsMac() { return false; }
    inline bool IsAndroid() { return true; }
    inline QString DeviceModel() { return QString("Android"); }
    inline QString SystemVersion() { return QString("Android"); }
}

// === Info namespace ===
namespace Info {
    struct Profile {
        QString name;
        QString description;
    };
}

// === Window namespace ===
namespace Window {
    class Controller {
    public:
        Main::Session &session() { static Main::Session s; return s; }
    };
}

// === Media namespace ===
namespace Media {
    namespace Streaming {
        class Instance {
        public:
            bool playing() const { return false; }
            void play() {}
            void pause() {}
            void stop() {}
        };
    }
}

// === Tags/Type traits ===
namespace base {
    template<typename T>
    inline constexpr bool is_pointer_v = std::is_pointer_v<T>;
}

// === base::static_guard ===
namespace base {
    struct static_guard {};
}

// === base::unique_function ===
namespace base {
    template<typename Signature>
    class unique_function;

    template<typename R, typename... Args>
    class unique_function<R(Args...)> {
    public:
        unique_function() = default;
        unique_function(std::function<R(Args...)> fn) : _fn(std::move(fn)) {}
        R operator()(Args... args) const { return _fn ? _fn(std::forward<Args>(args)...) : R(); }
        operator bool() const { return static_cast<bool>(_fn); }
    private:
        std::function<R(Args...)> _fn;
    };
}

// === Forward declarations for desktop types ===
namespace Ui {
    class BoxContent;
    class RpWidget;
    class LabelSimple;
    class FlatLabel;
    class InputField;
    class RippleButton;
    class IconButton;
    class RoundButton;
    class PopupMenu;
    class ScrollArea;
    class CrossButton;
    class PlainShadow;
    class ToggleView;
}

// === Export namespace ===
namespace Export {
    class Controller;
}

// === Stats namespace ===
namespace Stats {
    template<typename T> class Track {
    public:
        void add(T &&value) { (void)value; }
        T sum() const { return T(); }
    };
}
