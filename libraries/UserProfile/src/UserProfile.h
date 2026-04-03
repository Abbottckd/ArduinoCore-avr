/*
  UserProfile.h - User profile storage library with enhanced privacy features.
  Copyright (c) 2026 Arduino LLC.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef UserProfile_h
#define UserProfile_h

#include <Arduino.h>
#include <EEPROM.h>

// Maximum length for the profile name field (including null terminator)
#define USER_PROFILE_NAME_LEN 17

// Maximum length for the profile email field (including null terminator)
#define USER_PROFILE_EMAIL_LEN 33

// Privacy mode flag bit in the preferences field
#define USER_PROFILE_PRIVACY_BIT 0

/***
    UserProfileData struct.

    Raw data stored in EEPROM for a user profile.  The struct is prefixed with
    a magic number and suffixed with a CRC-8 checksum so that corrupt or
    uninitialised EEPROM contents are detected.
***/
struct UserProfileData {
    uint16_t magic;                       // Must equal UserProfile::MAGIC to be valid
    char     name[USER_PROFILE_NAME_LEN]; // Display name (null-terminated)
    char     email[USER_PROFILE_EMAIL_LEN]; // Email address (null-terminated)
    uint32_t deviceId;                    // Numeric device identifier
    uint8_t  preferences;                 // Bit-field of user preferences/flags
    uint8_t  crc;                         // CRC-8 over all preceding bytes
};

/***
    UserProfile class.

    Persists a UserProfileData record to EEPROM starting at a configurable
    address.  Privacy-enhancing features include:

      * CRC validation – corrupt or uninitialised profiles are rejected.
      * Secure wipe  – wipe() overwrites every profile byte with 0x00 before
                       clearing the in-memory copy, preventing data remanence.
      * Privacy mode – when enabled, getMaskedName() and getMaskedEmail()
                       return partially-obscured values so that sensitive
                       fields are not exposed unnecessarily.
***/
class UserProfile {
public:
    // Magic number written to EEPROM to identify a valid profile record
    static const uint16_t MAGIC = 0x5550; // 'UP'

    UserProfile();

    // Initialise the library.  eepromAddress is the first EEPROM byte used
    // to store the profile.  Returns true if an existing profile was loaded.
    bool begin(int eepromAddress = 0);

    // Load the profile from EEPROM.  Returns true if a valid profile exists.
    bool load();

    // Save the current in-memory profile to EEPROM.  Returns true on success.
    bool save();

    // Returns true when a valid profile is present in EEPROM.
    bool exists() const;

    // Securely wipe the profile from EEPROM (overwrites with zeros) and clear
    // the in-memory copy.
    void wipe();

    // --- Name field ---
    // Returns the stored name, or an empty string when no profile is loaded.
    const char* getName() const;

    // Returns the name with all but the first character replaced by '*'.
    // When privacy mode is disabled the plain name is returned instead.
    const char* getMaskedName() const;

    // Set the name (truncated to USER_PROFILE_NAME_LEN - 1 characters).
    void setName(const char* name);

    // --- Email field ---
    // Returns the stored email, or an empty string when no profile is loaded.
    const char* getEmail() const;

    // Returns the email with the local part replaced by '*' characters.
    // When privacy mode is disabled the plain email is returned instead.
    const char* getMaskedEmail() const;

    // Set the email (truncated to USER_PROFILE_EMAIL_LEN - 1 characters).
    void setEmail(const char* email);

    // --- Device ID field ---
    uint32_t getDeviceId() const;
    void     setDeviceId(uint32_t id);

    // --- Privacy mode ---
    bool isPrivacyEnabled() const;
    void setPrivacyEnabled(bool enabled);

private:
    int             _eepromAddress;
    bool            _loaded;
    UserProfileData _data;

    // Mutable buffers used by the masking helper methods so that they can
    // return a const char* from a const method without static local storage.
    mutable char _maskedName[USER_PROFILE_NAME_LEN];
    mutable char _maskedEmail[USER_PROFILE_EMAIL_LEN];

    // Compute CRC-8 (polynomial 0x07) over the first 'len' bytes of 'buf'.
    static uint8_t computeCRC(const uint8_t* buf, size_t len);

    // Return the offset of the crc field inside UserProfileData.
    static size_t crcOffset();
};

#endif
