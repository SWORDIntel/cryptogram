/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "iv/iv_controller.h"

#include "base/event_filter.h"
#include "base/platform/base_platform_info.h"
#include "base/qt/qt_key_modifiers.h"
#include "base/qt_signal_producer.h"
#include "core/file_utilities.h"
#include "lang/lang_keys.h"
#include "ui/chat/attach/attach_bot_webview.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/rp_window.h"
#include "ui/basic_click_handlers.h"
#include "ui/painter.h"
#include "ui/rect.h"
#include "webview/webview_embed.h"
#include "webview/webview_interface.h"
#include "styles/palette.h"
#include "styles/style_iv.h"
#include "styles/style_payments.h"
#include "styles/style_window.h"

#include <QtCore/QUrl>
#include <QtGui/QGuiApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QWindow>

#include <string>

#include <ada.h>

namespace Iv {
namespace {

constexpr auto kZoomStep = int(10);
constexpr auto kZoomSmallStep = int(5);
constexpr auto kZoomTinyStep = int(1);
constexpr auto kDefaultZoom = int(100);

class ItemZoom final
	: public Ui::Menu::Action
	, public Ui::AbstractTooltipShower {
public:
	ItemZoom(
		not_null<Ui::Menu::Menu*> parent,
		const not_null<Delegate*> delegate,
		const style::Menu &st)
	: Ui::Menu::Action(
		parent->menu(),
		st,
		Ui::CreateChild<QAction>(parent),
		nullptr,
		nullptr)
	, _delegate(delegate)
	, _st(st) {
		init();
	}

	void init() {
		enableMouseSelecting();

		AbstractButton::setDisabled(true);

		const auto processTooltip = [=](not_null<Ui::RpWidget*> w) {
			w->events() | rpl::on_next([=](not_null<QEvent*> e) {
				if (e->type() == QEvent::Enter) {
					Ui::Tooltip::Show(1000, this);
				} else if (e->type() == QEvent::Leave) {
					Ui::Tooltip::Hide();
				}
			}, w->lifetime());
		};

		const auto reset = Ui::CreateChild<Ui::RoundButton>(
			this,
			rpl::single<QString>(QString()),
			st::ivResetZoom);
		processTooltip(reset);
		const auto resetLabel = Ui::CreateChild<Ui::FlatLabel>(
			reset,
			tr::lng_background_reset_default(),
			st::ivResetZoomLabel);
		resetLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
		reset->setTextTransform(Ui::RoundButtonTextTransform::NoTransform);
		reset->setClickedCallback([this] {
			_delegate->ivSetZoom(0);
		});
		reset->show();
		const auto plus = Ui::CreateSimpleCircleButton(
			this,
			st::defaultRippleAnimationBgOver);
		plus->resize(Size(st::ivZoomButtonsSize));
		plus->paintRequest() | rpl::on_next([=, fg = _st.itemFg] {
			auto p = QPainter(plus);
			p.setPen(fg);
			p.setFont(st::normalFont);
			p.drawText(plus->rect(), QChar('+'), style::al_center);
		}, plus->lifetime());
		processTooltip(plus);
		const auto step = [] {
			return base::IsAltPressed()
				? kZoomTinyStep
				: base::IsCtrlPressed()
				? kZoomSmallStep
				: kZoomStep;
		};
		plus->setClickedCallback([this, step] {
			_delegate->ivSetZoom(_delegate->ivZoom() + step());
		});
		plus->show();
		const auto minus = Ui::CreateSimpleCircleButton(
			this,
			st::defaultRippleAnimationBgOver);
		minus->resize(Size(st::ivZoomButtonsSize));
		minus->paintRequest() | rpl::on_next([=, fg = _st.itemFg] {
			auto p = QPainter(minus);
			const auto r = minus->rect();
			p.setPen(fg);
			p.setFont(st::normalFont);
			p.drawText(
				QRectF(r).translated(0, style::ConvertFloatScale(-1)),
				QChar(0x2013),
				style::al_center);
		}, minus->lifetime());
		processTooltip(minus);
		minus->setClickedCallback([this, step] {
			_delegate->ivSetZoom(_delegate->ivZoom() - step());
		});
		minus->show();

		{
			const auto maxWidthText = u"000%"_q;
			_text.setText(_st.itemStyle, maxWidthText);
			Ui::Menu::ItemBase::setMinWidth(
				_text.maxWidth()
					+ st::ivResetZoomInnerPadding
					+ resetLabel->width()
					+ plus->width()
					+ minus->width()
					+ _st.itemPadding.right() * 2);
		}

		_delegate->ivZoomValue(
		) | rpl::on_next([this](int value) {
			_text.setText(_st.itemStyle, QString::number(value) + '%');
			update();
		}, lifetime());

		rpl::combine(
			sizeValue(),
			reset->sizeValue()
		) | rpl::on_next([=](const QSize &size, const QSize &) {
			reset->setFullWidth(0
				+ resetLabel->width()
				+ st::ivResetZoomInnerPadding);
			resetLabel->moveToLeft(
				(reset->width() - resetLabel->width()) / 2,
				(reset->height() - resetLabel->height()) / 2);
			reset->moveToRight(
				_st.itemPadding.right(),
				(size.height() - reset->height()) / 2);
			plus->moveToRight(
				_st.itemPadding.right() + reset->width(),
				(size.height() - plus->height()) / 2);
			minus->moveToRight(
				_st.itemPadding.right() + plus->width() + reset->width(),
				(size.height() - minus->height()) / 2);
		}, lifetime());
	}

	void paintEvent(QPaintEvent *event) override {
		auto p = QPainter(this);
		p.setPen(_st.itemFg);
		_text.draw(p, {
			.position = QPoint(
				_st.itemIconPosition.x(),
				(height() - _text.minHeight()) / 2),
			.outerWidth = width(),
			.availableWidth = width(),
		});
	}

	QString tooltipText() const override {
#ifdef Q_OS_MAC
		return tr::lng_iv_zoom_tooltip_cmd(tr::now);
#else
		return tr::lng_iv_zoom_tooltip_ctrl(tr::now);
#endif
	}

	QPoint tooltipPos() const override {
		return QCursor::pos();
	}

	bool tooltipWindowActive() const override {
		return true;
	}

private:
	const not_null<Delegate*> _delegate;
	const style::Menu &_st;
	Ui::Text::String _text;

};

[[nodiscard]] QByteArray ComputeStyles(int zoom) {
	static const auto map = base::flat_map<QByteArray, const style::color*>{
		{ "shadow-fg", &st::shadowFg },
		{ "scroll-bg", &st::scrollBg },
		{ "scroll-bg-over", &st::scrollBgOver },
		{ "scroll-bar-bg", &st::scrollBarBg },
		{ "scroll-bar-bg-over", &st::scrollBarBgOver },
		{ "window-bg", &st::windowBg },
		{ "window-bg-over", &st::windowBgOver },
		{ "window-bg-ripple", &st::windowBgRipple },
		{ "window-bg-active", &st::windowBgActive },
		{ "window-fg", &st::windowFg },
		{ "window-sub-text-fg", &st::windowSubTextFg },
		{ "window-active-text-fg", &st::windowActiveTextFg },
		{ "window-shadow-fg", &st::windowShadowFg },
		{ "box-divider-bg", &st::boxDividerBg },
		{ "box-divider-fg", &st::boxDividerFg },
		{ "light-button-fg", &st::lightButtonFg },
		//{ "light-button-bg-over", &st::lightButtonBgOver },
		{ "menu-icon-fg", &st::menuIconFg },
		{ "menu-icon-fg-over", &st::menuIconFgOver },
		{ "menu-bg", &st::menuBg },
		{ "menu-bg-over", &st::menuBgOver },
		{ "history-to-down-fg", &st::historyToDownFg },
		{ "history-to-down-fg-over", &st::historyToDownFgOver },
		{ "history-to-down-bg", &st::historyToDownBg },
		{ "history-to-down-bg-over", &st::historyToDownBgOver },
		{ "history-to-down-bg-ripple", &st::historyToDownBgRipple },
		{ "history-to-down-shadow", &st::historyToDownShadow },
		{ "toast-bg", &st::toastBg },
		{ "toast-fg", &st::toastFg },
	};
	static const auto phrases = base::flat_map<QByteArray, tr::phrase<>>{
		{ "iv-join-channel", tr::lng_iv_join_channel },
	};
	return Ui::ComputeStyles(map, phrases, zoom)
		+ ';'
		+ Ui::ComputeSemiTransparentOverStyle(
			"light-button-bg-over",
			st::lightButtonBgOver,
			st::windowBg);
}

[[nodiscard]] QByteArray WrapPage(const Prepared &page, int zoom) {
#ifdef Q_OS_MAC
	const auto classAttribute = ""_q;
#else // Q_OS_MAC
	const auto classAttribute = " class=\"custom_scroll\""_q;
#endif // Q_OS_MAC

	const auto js = QByteArray()
		+ (page.hasCode ? "IV.initPreBlocks();" : "")
		+ (page.hasEmbeds ? "IV.initEmbedBlocks();" : "")
		+ "IV.init();"
		+ page.script;

	return R"(<!DOCTYPE html>
<html)"_q
	+ classAttribute
	+ R"( style=")"
	+ Ui::EscapeForAttribute(ComputeStyles(zoom))
	+ R"(">
	<head>
		<meta charset="utf-8">
		<meta name="robots" content="noindex, nofollow">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<script src="/iv/page.js"></script>
		<link rel="stylesheet" href="/iv/page.css" />
	</head>
	<body>
		<div id="top_shadow"></div>
		<button class="fixed_button hidden" id="bottom_up" onclick="IV.scrollTo(0);">
			<svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
				<path d="M14.9972363,18 L9.13865768,12.1414214 C9.06055283,12.0633165 9.06055283,11.9366835 9.13865768,11.8585786 L14.9972363,6 L14.9972363,6" transform="translate(11.997236, 12.000000) scale(-1, -1) rotate(-90.000000) translate(-11.997236, -12.000000) "></path>
			</svg>
		</button>
		<div class="page-scroll" tabindex="-1">)"_q + page.content.trimmed() + R"(</div>
		<script>)"_q + js + R"(</script>
	</body>
</html>
)"_q;
}

[[nodiscard]] QByteArray ReadResource(const QString &name) {
	auto file = QFile(u":/iv/"_q + name);
	return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

[[nodiscard]] QString TonsiteToHttps(QString value) {
	const auto ChangeHost = [](QString tonsite) {
		const auto fake = "http://" + tonsite.toStdString();
		const auto parsed = ada::parse<ada::url>(fake);
		if (!parsed) {
			return QString();
		}
		tonsite = QString::fromStdString(parsed->get_hostname());
		tonsite = tonsite.replace('-', "-h");
		tonsite = tonsite.replace('.', "-d");
		return tonsite + ".magic.org";
	};
	const auto prefix = u"tonsite://"_q;
	if (!value.toLower().startsWith(prefix)) {
		return QString();
	}
	const auto part = value.mid(prefix.size());
	const auto split = part.indexOf('/');
	const auto host = ChangeHost((split < 0) ? part : part.left(split));
	if (host.isEmpty()) {
		return QString();
	}
	return "https://" + host + ((split < 0) ? u"/"_q : part.mid(split));
}

[[nodiscard]] QString HttpsToTonsite(QString value) {
	const auto ChangeHost = [](QString https) {
		const auto dot = https.indexOf('.');
		if (dot < 0 || https.mid(dot).toLower() != u".magic.org"_q) {
			return QString();
		}
		https = https.mid(0, dot);
		https = https.replace("-d", ".");
		https = https.replace("-h", "-");
		auto parts = https.split('.');
		for (auto &part : parts) {
			if (part.startsWith(u"xn--"_q)) {
				const auto utf8 = part.mid(4).toStdString();
				auto out = std::u32string();
				if (ada::idna::punycode_to_utf32(utf8, out)) {
					part = QString::fromUcs4(out.data(), out.size());
				}
			}
		}
		return parts.join('.');
	};
	const auto prefix = u"https://"_q;
	if (!value.toLower().startsWith(prefix)) {
		return value;
	}
	const auto part = value.mid(prefix.size());
	const auto split = part.indexOf('/');
	const auto host = ChangeHost((split < 0) ? part : part.left(split));
	if (host.isEmpty()) {
		return value;
	}
	return "tonsite://"
		+ host
		+ ((split < 0) ? u"/"_q : part.mid(split));
}

} // namespace

Controller::Controller(not_null<Delegate*> delegate)
: _delegate(delegate) {
	createWindow();
}

Controller::~Controller() {
	if (_window) {
		_window->hide();
	}
	base::take(_webview);
	_back = nullptr;
	_forward = nullptr;
	_subtitle = nullptr;
	_subtitleWrap = nullptr;
	_window = nullptr;
}

void Controller::updateTitleGeometry(int newWidth) const {
	_subtitleWrap->setGeometry(
		0,
		0,
		newWidth,
		st::ivSubtitleHeight);
	_subtitleWrap->paintRequest() | rpl::on_next([=](QRect clip) {
		QPainter(_subtitleWrap.get()).fillRect(clip, st::windowBg);
	}, _subtitleWrap->lifetime());

	const auto left = (_back ? _back->width() : 0)
		+ (_forward ? _forward->width() : 0)
		+ st::ivSubtitleSkip;
	const auto right = st::ivSubtitleSkip;
	_subtitle->resizeToWidth(std::max(newWidth - left - right, 0));
	_subtitle->moveToLeft(left, st::ivSubtitleTop);

	if (_back) {
		_back->moveToLeft(0, 0);
	}
	if (_forward) {
		_forward->moveToLeft(_back ? _back->width() : 0, 0);
	}
}

bool Controller::IsGoodTonSiteUrl(const QString &uri) {
	return !TonsiteToHttps(uri).isEmpty();
}

void Controller::showTonSite(
	const Webview::StorageId& storageId,
	QString uri) {
	const auto url = TonsiteToHttps(uri);
	Assert(!url.isEmpty());

	if (!_webview) {
		createWebview(storageId);
	}
	if (_webview && _webview->widget()) {
		_webview->navigate(url);
		activate();
	}
	_url = url;
	_subtitleText = _url.value(
	) | rpl::filter([=](const QString& url) {
		return !url.isEmpty() && url != u"about:blank"_q;
		}) | rpl::map([=](QString value) {
			return HttpsToTonsite(value);
			});
		_windowTitleText = _subtitleText.value();
}

void Controller::showTLViewer(
	const Webview::StorageId& storageId,
	QString url) {
	if (!_webview) {
		createWebview(storageId);
	}
	if (_webview && _webview->widget()) {
		_webview->navigate(url);
		activate();
	}
	_url = url;
	_subtitleText = tr::lng_context_view_as_json(tr::now);
	_windowTitleText = _subtitleText.value();
}

void Controller::createWindow() {
	_window = std::make_unique<Ui::RpWindow>();
	const auto window = _window.get();

	_subtitleWrap = std::make_unique<Ui::RpWidget>(_window->body().get());
	_subtitle = Ui::CreateChild<Ui::FlatLabel>(
		_subtitleWrap.get(),
		_subtitleText.value(),
		st::ivSubtitle);
	_subtitle->setSelectable(true);

	_windowTitleText = _subtitleText.value(
	) | rpl::map([=](const QString &subtitle) {
		const auto prefix = tr::lng_iv_window_title(tr::now);
		return prefix + ' ' + QChar(0x2014) + ' ' + subtitle;
	});
	_windowTitleText.value(
	) | rpl::on_next([=](const QString &title) {
		_window->setWindowTitle(title);
	}, _subtitle->lifetime());

	_back = Ui::CreateChild<Ui::IconButton>(_subtitleWrap.get(), st::ivBack);
	_back->setClickedCallback([=] {
		if (_webview) {
			_webview->eval("window.history.back();");
		}
	});
	_back->setDisabled(true);

	_forward = Ui::CreateChild<Ui::IconButton>(
		_subtitleWrap.get(),
		st::ivForward);
	_forward->setClickedCallback([=] {
		if (_webview) {
			_webview->eval("window.history.forward();");
		}
	});
	_forward->setDisabled(true);

	base::qt_signal_producer(
		qApp,
		&QGuiApplication::focusWindowChanged
	) | rpl::filter([=](QWindow *focused) {
		const auto handle = window->window()->windowHandle();
		return _webview && handle && (focused == handle);
	}) | rpl::on_next([=] {
		setInnerFocus();
	}, window->lifetime());

	window->body()->widthValue() | rpl::on_next([=](int width) {
		updateTitleGeometry(width);
	}, _subtitle->lifetime());

	window->events(
	) | rpl::on_next([=](not_null<QEvent*> e) {
		if (e->type() == QEvent::Close) {
			close();
		} else if (e->type() == QEvent::KeyPress) {
			processKey(static_cast<QKeyEvent*>(e.get()));
		}
	}, window->lifetime());

	base::install_event_filter(window, qApp, [=](not_null<QEvent*> e) {
		if (e->type() != QEvent::ShortcutOverride
			|| !window->isActiveWindow()) {
			return base::EventFilterResult::Continue;
		}
		const auto event = static_cast<QKeyEvent*>(e.get());
		const auto command = Platform::IsMac()
			? Qt::MetaModifier
			: Qt::ControlModifier;
		if (!(event->modifiers() & command)) {
			return base::EventFilterResult::Continue;
		} else if (event->key() == Qt::Key_Plus
			|| event->key() == Qt::Key_Equal) {
			_delegate->ivSetZoom(_delegate->ivZoom() + kZoomStep);
			return base::EventFilterResult::Cancel;
		} else if (event->key() == Qt::Key_Minus) {
			_delegate->ivSetZoom(_delegate->ivZoom() - kZoomStep);
			return base::EventFilterResult::Cancel;
		} else if (event->key() == Qt::Key_0) {
			_delegate->ivSetZoom(0);
			return base::EventFilterResult::Cancel;
		}
		return base::EventFilterResult::Continue;
	});

	window->setGeometry(_delegate->ivGeometry(window));
	window->setMinimumSize({ st::windowMinWidth, st::windowMinHeight });

	window->geometryValue(
	) | rpl::distinct_until_changed(
	) | rpl::skip(1) | rpl::on_next([=] {
		_delegate->ivSaveGeometry(window);
	}, window->lifetime());

	_container = Ui::CreateChild<Ui::RpWidget>(window->body().get());
	rpl::combine(
		window->body()->sizeValue(),
		_subtitleWrap->heightValue()
	) | rpl::on_next([=](QSize size, int title) {
		_container->setGeometry(QRect(QPoint(), size).marginsRemoved(
			{ 0, title, 0, 0 }));
	}, _container->lifetime());

	_container->paintRequest() | rpl::on_next([=](QRect clip) {
		QPainter(_container).fillRect(clip, st::windowBg);
	}, _container->lifetime());

	_container->show();
	window->show();
}

void Controller::createWebview(const Webview::StorageId &storageId) {
	Expects(!_webview);

	const auto window = _window.get();
	_webview = std::make_unique<Webview::Window>(
		_container,
		Webview::WindowConfig{
			.opaqueBg = st::windowBg->c,
			.storageId = storageId,
			.safe = true,
		});
	const auto raw = _webview.get();

	if (const auto webviewZoomController = raw->zoomController()) {
		webviewZoomController->zoomValue(
		) | rpl::on_next([this](int value) {
			if (value > 0) {
				_delegate->ivSetZoom(value);
			}
		}, lifetime());
		_delegate->ivZoomValue(
		) | rpl::on_next([=](int value) {
			webviewZoomController->setZoom(value);
		}, lifetime());
		webviewZoomController->setZoom(_delegate->ivZoom());
	}

	window->lifetime().add([=] {
		base::take(_webview);
	});

	const auto widget = raw->widget();
	if (!widget) {
		base::take(_webview);
		showWebviewError();
		return;
	}
	widget->show();

	QObject::connect(widget, &QObject::destroyed, [=] {
		if (!_webview) {
			return;
		}
		crl::on_main(window, [=] {
			showWebviewError({ "Error: WebView has crashed." });
		});
		base::take(_webview);
	});

	_container->sizeValue(
	) | rpl::on_next([=](QSize size) {
		if (const auto widget = raw->widget()) {
			widget->setGeometry(QRect(QPoint(), size));
		}
	}, _container->lifetime());

	raw->setNavigationStartHandler([=](const QString &uri, bool newWindow) {
		Q_UNUSED(newWindow);

		if (uri == u"about:blank"_q
			|| QUrl(uri).host().toLower().endsWith(u".magic.org"_q)) {
			return true;
		}
		_events.fire({ .type = Event::Type::OpenLink, .url = uri });
		return false;
	});
	raw->setNavigationDoneHandler([=](bool success) {
		Q_UNUSED(success);
	});
	raw->navigationHistoryState(
	) | rpl::on_next([=](Webview::NavigationHistoryState state) {
		_back->setDisabled(!state.canGoBack);
		_forward->setDisabled(!state.canGoForward);
		_url = QString::fromStdString(state.url);
	}, _webview->lifetime());

	raw->init(R"()");
}

void Controller::processKey(QKeyEvent *event) {
	const auto command = Platform::IsMac()
		? Qt::MetaModifier
		: Qt::ControlModifier;
	if (event->key() == Qt::Key_Escape) {
		close();
	} else if (event->modifiers() & command) {
		if (event->key() == Qt::Key_W) {
			close();
		} else if (event->key() == Qt::Key_M) {
			minimize();
		} else if (event->key() == Qt::Key_Q) {
			quit();
		} else if (event->key() == Qt::Key_0) {
			_delegate->ivSetZoom(0);
		}
	}
}

void Controller::activate() {
	if (_window->isMinimized()) {
		_window->showNormal();
	} else if (_window->isHidden()) {
		_window->show();
	}
	_window->raise();
	_window->activateWindow();
	_window->setFocus();
	setInnerFocus();
}

void Controller::setInnerFocus() {
	if (_webview) {
		_webview->focus();
	}
}

bool Controller::active() const {
	return _window && _window->isActiveWindow();
}

void Controller::minimize() {
	if (_window) {
		_window->setWindowState(_window->windowState()
			| Qt::WindowMinimized);
	}
}

void Controller::close() {
	_events.fire({ Event::Type::Close });
}

void Controller::quit() {
	_events.fire({ Event::Type::Quit });
}

rpl::lifetime &Controller::lifetime() {
	return _lifetime;
}

void Controller::showWebviewError() {
	const auto available = Webview::Availability();
	if (available.error != Webview::Available::Error::None) {
		showWebviewError(Ui::BotWebView::ErrorText(available));
	} else {
		showWebviewError({ "Error: Could not initialize WebView." });
	}
}

void Controller::showWebviewError(TextWithEntities text) {
	const auto wrap = Ui::CreateChild<Ui::RpWidget>(_container);

	const auto error = Ui::CreateChild<Ui::PaddingWrap<Ui::FlatLabel>>(
		wrap,
		object_ptr<Ui::FlatLabel>(
			wrap,
			rpl::single(text),
			st::paymentsCriticalError),
		st::paymentsCriticalErrorPadding);
	error->entity()->setClickHandlerFilter([=](
			const ClickHandlerPtr &handler,
			Qt::MouseButton) {
		const auto entity = handler->getTextEntity();
		if (entity.type != EntityType::CustomUrl) {
			return true;
		}
		File::OpenUrl(entity.data);
		return false;
	});
	wrap->show();

	wrap->widthValue() | rpl::on_next([=](int width) {
		error->resizeToWidth(width);
		wrap->resize(width, error->height());
	}, wrap->lifetime());

	_container->sizeValue() | rpl::on_next([=](QSize size) {
		wrap->setGeometry(0, 0, size.width(), size.height() * 2 / 3);
	}, wrap->lifetime());
}

void Controller::showInWindow(
		const Webview::StorageId &storageId,
		Prepared page) {
	Expects(_container != nullptr);

	const auto url = page.url;
	_hash = page.hash;
	auto i = _indices.find(url);
	if (i == end(_indices)) {
		_pages.push_back(std::move(page));
		i = _indices.emplace(url, int(_pages.size() - 1)).first;
	}
	const auto index = i->second;
	_index = index;
	if (!_webview) {
		createWebview(storageId);
		if (_webview && _webview->widget()) {
			auto id = u"iv/page%1.html"_q.arg(index);
			if (!_hash.isEmpty()) {
				id += '#' + _hash;
			}
			_webview->navigateToData(id);
			activate();
		} else {
			_events.fire({ Event::Type::Close });
		}
	} else if (_ready) {
		_webview->eval(navigateScript(index, _hash));
		activate();
	} else {
		_navigateToIndexWhenReady = index;
		_navigateToHashWhenReady = _hash;
		activate();
	}
}

void Controller::activate() {
	if (_window->isMinimized()) {
		_window->showNormal();
	} else if (_window->isHidden()) {
		_window->show();
	}
	_window->raise();
	_window->activateWindow();
	_window->setFocus();
	setInnerFocus();
}

void Controller::setInnerFocus() {
	if (const auto onstack = _shareFocus) {
		onstack();
	} else if (_webview) {
		_webview->focus();
	}
}

QByteArray Controller::navigateScript(int index, const QString &hash) {
	return "IV.navigateTo("
		+ QByteArray::number(index)
		+ ", '"
		+ Ui::EscapeForScriptString(qthelp::url_decode(hash).toUtf8())
		+ "');";
}

QByteArray Controller::reloadScript(int index) {
	return "IV.reloadPage("
		+ QByteArray::number(index)
		+ ");";
}

void Controller::processKey(const QString &key, const QString &modifier) {
	const auto ctrl = Platform::IsMac() ? u"cmd"_q : u"ctrl"_q;
	if (key == u"escape"_q) {
		escape();
	} else if (key == u"w"_q && modifier == ctrl) {
		close();
	} else if (key == u"m"_q && modifier == ctrl) {
		minimize();
	} else if (key == u"q"_q && modifier == ctrl) {
		quit();
	} else if (key == u"0"_q && modifier == ctrl) {
		_delegate->ivSetZoom(0);
	}
}

void Controller::processLink(const QString &url, const QString &context) {
	const auto channelPrefix = u"channel"_q;
	const auto joinPrefix = u"join_link"_q;
	const auto webpagePrefix = u"webpage"_q;
	const auto viewerPrefix = u"viewer"_q;
	if (context == u"report-iv") {
		_events.fire({
			.type = Event::Type::Report,
			.context = QString::number(compuseCurrentPageId()),
		});
	} else if (context.startsWith(channelPrefix)) {
		_events.fire({
			.type = Event::Type::OpenChannel,
			.context = context.mid(channelPrefix.size()),
		});
	} else if (context.startsWith(joinPrefix)) {
		_events.fire({
			.type = Event::Type::JoinChannel,
			.context = context.mid(joinPrefix.size()),
		});
	} else if (context.startsWith(webpagePrefix)) {
		_events.fire({
			.type = Event::Type::OpenPage,
			.url = url,
			.context = context.mid(webpagePrefix.size()),
		});
	} else if (context.startsWith(viewerPrefix)) {
		_events.fire({
			.type = Event::Type::OpenMedia,
			.url = url,
			.context = context.mid(viewerPrefix.size()),
		});
	} else if (context.isEmpty()) {
		_events.fire({ .type = Event::Type::OpenLink, .url = url });
	}
}

bool Controller::active() const {
	return _window && _window->isActiveWindow();
}

void Controller::showJoinedTooltip() {
	if (_webview && _ready) {
		_webview->eval("IV.showTooltip('"
			+ Ui::EscapeForScriptString(
				tr::lng_action_you_joined(tr::now).toUtf8())
			+ "');");
	}
}

void Controller::minimize() {
	if (_window) {
		_window->setWindowState(_window->windowState()
			| Qt::WindowMinimized);
	}
}

QString Controller::composeCurrentUrl() const {
	const auto index = _index.current();
	Assert(index >= 0 && index < _pages.size());

	return _pages[index].url
		+ (_hash.isEmpty() ? u""_q : ('#' + _hash));
}

uint64 Controller::compuseCurrentPageId() const {
	const auto index = _index.current();
	Assert(index >= 0 && index < _pages.size());

	return _pages[index].pageId;
}

void Controller::showMenu() {
	const auto index = _index.current();
	if (_menu || index < 0 || index > _pages.size()) {
		return;
	}
	_menu = base::make_unique_q<Ui::PopupMenu>(
		_window.get(),
		st::popupMenuWithIcons);
	if (_webview && _ready) {
		_webview->eval("IV.menuShown(true);");
	}
	_menu->setDestroyedCallback(crl::guard(_window.get(), [
			this,
			weakButton = base::make_weak(_menuToggle.data()),
			menu = _menu.get()] {
		if (_menu == menu && weakButton) {
			weakButton->setForceRippled(false);
		}
		if (const auto widget = _webview ? _webview->widget() : nullptr) {
			InvokeQueued(widget, crl::guard(_window.get(), [=] {
				if (_webview && _ready) {
					_webview->eval("IV.menuShown(false);");
				}
			}));
		}
	}));
	_menuToggle->setForceRippled(true);

	const auto url = composeCurrentUrl();
	const auto openInBrowser = crl::guard(_window.get(), [=] {
		_events.fire({ .type = Event::Type::OpenLinkExternal, .url = url });
	});
	_menu->addAction(
		tr::lng_iv_open_in_browser(tr::now),
		openInBrowser,
		&st::menuIconIpAddress);

	_menu->addAction(tr::lng_iv_share(tr::now), [=] {
		showShareMenu();
	}, &st::menuIconShare);

	_menu->addSeparator();
	_menu->addAction(
		base::make_unique_q<ItemZoom>(_menu->menu(), _delegate, _menu->menu()->st()));
	_menu->setForcedOrigin(Ui::PanelAnimation::Origin::TopRight);
	_menu->popup(_window->body()->mapToGlobal(
		QPoint(_window->body()->width(), 0) + st::ivMenuPosition));
}

void Controller::escape() {
	if (const auto onstack = _shareHide) {
		onstack();
	} else {
		close();
	}
}

void Controller::close() {
	_events.fire({ Event::Type::Close });
}

void Controller::quit() {
	_events.fire({ Event::Type::Quit });
}

rpl::lifetime &Controller::lifetime() {
	return _lifetime;
}

void Controller::destroyShareMenu() {
	_shareHide = nullptr;
	if (_shareFocus) {
		_shareFocus = nullptr;
		setInnerFocus();
	}
	if (_shareWrap) {
		if (_shareContainer) {
			_shareWrap->windowHandle()->setParent(nullptr);
		}
		_shareWrap = nullptr;
		_shareContainer = nullptr;
	}
	if (_shareHidesContent) {
		_shareHidesContent = false;
		if (const auto content = _webview ? _webview->widget() : nullptr) {
			content->show();
		}
	}
}

void Controller::showShareMenu() {
	const auto index = _index.current();
	if (_shareWrap || index < 0 || index > _pages.size()) {
		return;
	}
	_shareHidesContent = Platform::IsMac();
	if (_shareHidesContent) {
		if (const auto content = _webview ? _webview->widget() : nullptr) {
			content->hide();
		}
	}

	_shareWrap = std::make_unique<Ui::RpWidget>(_shareHidesContent
		? _window->body().get()
		: nullptr);
	if (!_shareHidesContent) {
		_shareWrap->setGeometry(_window->body()->rect());
		_shareWrap->setWindowFlag(Qt::FramelessWindowHint);
		_shareWrap->setAttribute(Qt::WA_TranslucentBackground);
		_shareWrap->setAttribute(Qt::WA_NoSystemBackground);
		_shareWrap->createWinId();

		_shareContainer.reset(QWidget::createWindowContainer(
			_shareWrap->windowHandle(),
			_window->body().get(),
			Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint));
	}
	_window->body()->sizeValue() | rpl::on_next([=](QSize size) {
		const auto widget = _shareHidesContent
			? _shareWrap.get()
			: _shareContainer.get();
		widget->setGeometry(QRect(QPoint(), size));
	}, _shareWrap->lifetime());

	auto result = _showShareBox({
		.parent = _shareWrap.get(),
		.url = composeCurrentUrl(),
	});
	_shareFocus = result.focus;
	_shareHide = result.hide;

	std::move(result.destroyRequests) | rpl::on_next([=] {
		destroyShareMenu();
	}, _shareWrap->lifetime());

	Ui::ForceFullRepaintSync(_shareWrap.get());

	if (_shareHidesContent) {
		_shareWrap->show();
	} else {
		_shareContainer->show();
	}
	activate();
}

} // namespace Iv
