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

// Quantum Guard - Provides post-quantum cryptographic protection
class QuantumGuard {
public:
    QuantumGuard() = default;
    ~QuantumGuard() = default;

    // Initialize the quantum guard module
    void initialize();

    // Check if quantum protection is active
    bool isProtected() const;

    // Enable/disable quantum protection
    void setProtected(bool protected_);

    // Encrypt data with quantum-resistant algorithm
    QByteArray encrypt(const QByteArray &plaintext);

    // Decrypt data with quantum-resistant algorithm
    QByteArray decrypt(const QByteArray &ciphertext);

private:
    bool _protected = false;
    QuantumGuard(const QuantumGuard &other) = delete;
    QuantumGuard &operator=(const QuantumGuard &other) = delete;
};

} // namespace Data
