/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#include "iv/editor/iv_editor_window.h"

#ifdef Q_OS_WIN
#include <QtNetwork/QNetworkProxy>

#include "core/sandbox.h"

#include <windows.h>
#elif defined Q_OS_MAC // Q_OS_WIN
#include "core/sandbox.h"
#include "platform/mac/native_event_mac.h"
#endif // Q_OS_WIN || Q_OS_MAC

namespace Iv::Editor {

Window::Window(QWidget *parent)
: Ui::RpWindow(parent) {
}

rpl::producer<> Window::imeCompositionStarts() const {
	return _imeCompositionStartReceived.events();
}

void Window::imeCompositionStartReceived() {
	_imeCompositionStartReceived.fire({});
}

#ifdef Q_OS_WIN

bool Window::nativeEvent(
		const QByteArray &eventType,
		void *message,
		native_event_filter_result *result) {
	if (message) {
		const auto msg = static_cast<MSG*>(message);
		if (msg->message == WM_IME_STARTCOMPOSITION) {
			Core::Sandbox::Instance().customEnterFromEventLoop([&] {
				imeCompositionStartReceived();
			});
		}
	}
	return false;
}

#elif defined Q_OS_MAC // Q_OS_WIN

bool Window::nativeEvent(
		const QByteArray &eventType,
		void *message,
		qintptr *result) {
	if (message && eventType == "NSEvent") {
		if (Platform::PossiblyTextTypingEvent(message)) {
			Core::Sandbox::Instance().customEnterFromEventLoop([&] {
				imeCompositionStartReceived();
			});
		}
	}
	return false;
}

#endif // Q_OS_WIN || Q_OS_MAC

} // namespace Iv::Editor
