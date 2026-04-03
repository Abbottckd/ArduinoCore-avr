/*
  UserProfile.cpp - User profile storage library with enhanced privacy features.
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

#include "UserProfile.h"
#include <string.h>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

UserProfile::UserProfile()
    : _eepromAddress(0)
    , _loaded(false)
{
    memset(&_data, 0, sizeof(_data));
    memset(_maskedName,  0, sizeof(_maskedName));
    memset(_maskedEmail, 0, sizeof(_maskedEmail));
}

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------

bool UserProfile::begin(int eepromAddress)
{
    _eepromAddress = eepromAddress;
    return load();
}

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------

bool UserProfile::load()
{
    UserProfileData tmp;
    uint8_t* ptr = reinterpret_cast<uint8_t*>(&tmp);

    for (int i = 0; i < (int)sizeof(tmp); i++) {
        ptr[i] = EEPROM.read(_eepromAddress + i);
    }

    if (tmp.magic != MAGIC) {
        _loaded = false;
        return false;
    }

    uint8_t expected = computeCRC(ptr, crcOffset());
    if (tmp.crc != expected) {
        _loaded = false;
        return false;
    }

    memcpy(&_data, &tmp, sizeof(_data));
    _loaded = true;
    return true;
}

bool UserProfile::save()
{
    _data.magic = MAGIC;

    // Ensure string fields are null-terminated within their bounds
    _data.name[USER_PROFILE_NAME_LEN - 1]   = '\0';
    _data.email[USER_PROFILE_EMAIL_LEN - 1] = '\0';

    // Compute and embed the CRC
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&_data);
    _data.crc = computeCRC(ptr, crcOffset());

    EEPROM.put(_eepromAddress, _data);
    _loaded = true;
    return true;
}

bool UserProfile::exists() const
{
    // Peek at EEPROM without loading into _data
    uint16_t magic;
    EEPROM.get(_eepromAddress, magic);
    if (magic != MAGIC) {
        return false;
    }

    UserProfileData tmp;
    EEPROM.get(_eepromAddress, tmp);
    uint8_t expected = computeCRC(
        reinterpret_cast<const uint8_t*>(&tmp), crcOffset());
    return tmp.crc == expected;
}

// ---------------------------------------------------------------------------
// Secure wipe
// ---------------------------------------------------------------------------

void UserProfile::wipe()
{
    // Overwrite every EEPROM byte occupied by the profile with 0x00 so that
    // no residual data can be read back afterwards.
    for (int i = 0; i < (int)sizeof(UserProfileData); i++) {
        EEPROM.write(_eepromAddress + i, 0x00);
    }

    // Clear the in-memory copy
    memset(&_data, 0, sizeof(_data));
    _loaded = false;
}

// ---------------------------------------------------------------------------
// Name field
// ---------------------------------------------------------------------------

const char* UserProfile::getName() const
{
    return _loaded ? _data.name : "";
}

const char* UserProfile::getMaskedName() const
{
    if (!_loaded) {
        return "";
    }
    if (!isPrivacyEnabled()) {
        return _data.name;
    }

    size_t len = strlen(_data.name);
    if (len == 0) {
        _maskedName[0] = '\0';
        return _maskedName;
    }

    // Keep only the first character; replace the rest with '*'
    _maskedName[0] = _data.name[0];
    for (size_t i = 1; i < len && i < (size_t)(USER_PROFILE_NAME_LEN - 1); i++) {
        _maskedName[i] = '*';
    }
    _maskedName[len] = '\0';
    return _maskedName;
}

void UserProfile::setName(const char* name)
{
    strncpy(_data.name, name, USER_PROFILE_NAME_LEN - 1);
    _data.name[USER_PROFILE_NAME_LEN - 1] = '\0';
}

// ---------------------------------------------------------------------------
// Email field
// ---------------------------------------------------------------------------

const char* UserProfile::getEmail() const
{
    return _loaded ? _data.email : "";
}

const char* UserProfile::getMaskedEmail() const
{
    if (!_loaded) {
        return "";
    }
    if (!isPrivacyEnabled()) {
        return _data.email;
    }

    const char* at = strchr(_data.email, '@');
    size_t len = strlen(_data.email);

    if (len == 0) {
        _maskedEmail[0] = '\0';
        return _maskedEmail;
    }

    if (at == NULL) {
        // No '@' sign – mask all but the first character
        _maskedEmail[0] = _data.email[0];
        for (size_t i = 1; i < len && i < (size_t)(USER_PROFILE_EMAIL_LEN - 1); i++) {
            _maskedEmail[i] = '*';
        }
        _maskedEmail[len] = '\0';
        return _maskedEmail;
    }

    // Keep the first character and the domain part; mask the rest of the local
    // part, e.g. "alice@example.com" -> "a****@example.com"
    size_t atIndex = (size_t)(at - _data.email);
    _maskedEmail[0] = _data.email[0];
    for (size_t i = 1; i < atIndex && i < (size_t)(USER_PROFILE_EMAIL_LEN - 1); i++) {
        _maskedEmail[i] = '*';
    }
    // Copy '@' and everything after it
    for (size_t i = atIndex; i < len && i < (size_t)(USER_PROFILE_EMAIL_LEN - 1); i++) {
        _maskedEmail[i] = _data.email[i];
    }
    _maskedEmail[len] = '\0';
    return _maskedEmail;
}

void UserProfile::setEmail(const char* email)
{
    strncpy(_data.email, email, USER_PROFILE_EMAIL_LEN - 1);
    _data.email[USER_PROFILE_EMAIL_LEN - 1] = '\0';
}

// ---------------------------------------------------------------------------
// Device ID
// ---------------------------------------------------------------------------

uint32_t UserProfile::getDeviceId() const
{
    return _loaded ? _data.deviceId : 0;
}

void UserProfile::setDeviceId(uint32_t id)
{
    _data.deviceId = id;
}

// ---------------------------------------------------------------------------
// Privacy mode
// ---------------------------------------------------------------------------

bool UserProfile::isPrivacyEnabled() const
{
    return (_data.preferences & (1u << USER_PROFILE_PRIVACY_BIT)) != 0;
}

void UserProfile::setPrivacyEnabled(bool enabled)
{
    if (enabled) {
        _data.preferences |=  (1u << USER_PROFILE_PRIVACY_BIT);
    } else {
        _data.preferences &= ~(1u << USER_PROFILE_PRIVACY_BIT);
    }
}

// ---------------------------------------------------------------------------
// CRC helpers
// ---------------------------------------------------------------------------

// CRC-8 with polynomial 0x07 (used by ATM / I2C)
uint8_t UserProfile::computeCRC(const uint8_t* buf, size_t len)
{
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (uint8_t)((crc << 1) ^ 0x07);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

size_t UserProfile::crcOffset()
{
    // The CRC covers everything in UserProfileData up to (but not including)
    // the crc field itself.
    return offsetof(UserProfileData, crc);
}
