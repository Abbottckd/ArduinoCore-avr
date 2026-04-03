/***
    UserProfileBasic example.

    Demonstrates how to create, save, and load a user profile using the
    UserProfile library.  Open the Serial Monitor at 9600 baud to view output.

    Circuit: no external components required.
***/

#include <UserProfile.h>

UserProfile profile;

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port (needed for native USB boards)
    }

    Serial.println("=== UserProfile Basic Example ===");

    // Check whether a saved profile already exists
    if (profile.begin()) {
        Serial.println("Existing profile loaded from EEPROM:");
        Serial.print("  Name:      "); Serial.println(profile.getName());
        Serial.print("  Email:     "); Serial.println(profile.getEmail());
        Serial.print("  Device ID: "); Serial.println(profile.getDeviceId());
    } else {
        Serial.println("No existing profile found – creating a new one.");

        profile.setName("Ada Lovelace");
        profile.setEmail("ada@lovelace.example");
        profile.setDeviceId(42);

        if (profile.save()) {
            Serial.println("Profile saved successfully.");
        } else {
            Serial.println("ERROR: failed to save profile.");
        }
    }

    // Re-load to confirm round-trip persistence
    Serial.println("\nReloading profile from EEPROM...");
    if (profile.load()) {
        Serial.println("Profile reloaded successfully:");
        Serial.print("  Name:      "); Serial.println(profile.getName());
        Serial.print("  Email:     "); Serial.println(profile.getEmail());
        Serial.print("  Device ID: "); Serial.println(profile.getDeviceId());
    } else {
        Serial.println("ERROR: profile could not be reloaded.");
    }
}

void loop() {
    // Nothing to do here
}
