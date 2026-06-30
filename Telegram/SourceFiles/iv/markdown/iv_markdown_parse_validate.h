/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "iv/markdown/iv_markdown_parse.h"

namespace Iv::Markdown {

[[nodiscard]] ParseResult Failure(QString sourceName, QString error);
[[nodiscard]] MarkdownSourceValidationResult ValidationFailure(
	QString sourceName,
	QString error);

} // namespace Iv::Markdown
