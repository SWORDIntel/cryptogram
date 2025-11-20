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

// TSM (Telegram Security Module) Factory
// Provides factory methods for creating TSM components
class TSMFactory {
public:
    TSMFactory() = default;
    ~TSMFactory() = default;

    // Factory methods for TSM components
    static std::unique_ptr<class QuantumSignal> createQuantumSignal();
    static std::unique_ptr<class QuantumGuard> createQuantumGuard();
    static std::unique_ptr<class NSASecurity> createNSASecurity();

private:
    TSMFactory(const TSMFactory &other) = delete;
    TSMFactory &operator=(const TSMFactory &other) = delete;
};

} // namespace Data
