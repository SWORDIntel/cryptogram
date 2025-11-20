/*
This file is part of CRYPTOGRAM,
the most advanced secure messaging application.

For license and copyright information please follow this link:
https://github.com/SWORDOps/CRYPTOGRAM/blob/main/LICENSE
*/
#pragma once

#include <memory>
#include <QString>

namespace Data {

// Quantum Signal - Provides quantum-resistant cryptographic signaling
class QuantumSignal {
public:
    QuantumSignal() = default;
    ~QuantumSignal() = default;

    // Initialize the quantum signal module
    void initialize();

    // Check if quantum signal is enabled
    bool isEnabled() const;

    // Enable/disable quantum signal processing
    void setEnabled(bool enabled);

    // Generate quantum-resistant signature
    QString generateSignature(const QByteArray &data);

    // Verify quantum-resistant signature
    bool verifySignature(const QByteArray &data, const QString &signature);

private:
    bool _enabled = false;
    QuantumSignal(const QuantumSignal &other) = delete;
    QuantumSignal &operator=(const QuantumSignal &other) = delete;
};

} // namespace Data
