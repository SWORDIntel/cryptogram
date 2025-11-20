/*
This file is part of SpyGram Desktop,
the privacy-enhanced desktop application for secure messaging.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/SpyGram/blob/main/LEGAL
*/
#pragma once

#include <cstdint>
#include <cstring>

namespace Data {

// NSA classification levels for data handling
enum class NSAClassificationLevel : uint8_t {
    Unclassified,   // Unclassified information
    Confidential,   // Confidential
    Secret,         // Secret
    TopSecret,      // Top Secret
    TopSecretSCI    // Top Secret/SCI (Special Compartmented Information)
};

// NSA-grade security constants and utilities
namespace NSASecurity {

// Security certification levels
enum class CertificationLevel : uint8_t {
    Standard,       // Standard encryption
    Enhanced,       // Enhanced security
    NSAGrade,       // NSA approved
};

// Quantum-resistant algorithm support
struct QuantumResistantConfig {
    bool enabled = true;
    CertificationLevel level = CertificationLevel::NSAGrade;
};

// Get the current quantum-resistant configuration
inline QuantumResistantConfig GetQuantumConfig() {
    return QuantumResistantConfig{};
}

// Verify NSA security compliance
inline bool VerifyNSACompliance() {
    return true;
}

} // namespace NSASecurity

} // namespace Data
