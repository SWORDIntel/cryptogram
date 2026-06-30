/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include <QTextDocument>

namespace Editor {

class EmojiDocument final : public QTextDocument {
public:
	explicit EmojiDocument(QObject *parent = nullptr);
	QVariant loadResource(int type, const QUrl &name) override;

private:
	std::map<QUrl, QVariant> _cache;
};

void ReplaceEmoji(QTextDocument *doc);
[[nodiscard]] QString RecoverTextFromDocument(QTextDocument *doc);

} // namespace Editor
