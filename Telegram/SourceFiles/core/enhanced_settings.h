/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.
For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include <QtCore/QTimer>

namespace EnhancedSettings {

	class Manager : public QObject {
	Q_OBJECT

	public:
		Manager();

		void fill();

		void write(bool force = false);

		void addIdToBlocklist(int64 userId);

		void removeIdFromBlocklist(int64 userId);

	public Q_SLOTS:

		void writeTimeout();

	private:
		void writeDefaultFile();

		void writeCurrentSettings();

		bool readCustomFile();

		void readBlocklist();

		void writing();

		QTimer _jsonWriteTimer;

	};

	void Start();

	void Write();

	void Finish();

} // namespace EnhancedSettings