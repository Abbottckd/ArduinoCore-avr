/***
    UserProfilePrivacy example.

    Demonstrates the enhanced-privacy features of the UserProfile library:

      1. Privacy mode   – getMaskedName() and getMaskedEmail() return
                          partially-obscured values so that sensitive data is
                          not exposed in logs or on displays.

      2. Secure wipe    – wipe() overwrites every profile byte in EEPROM with
                          0x00 before clearing the in-memory copy, preventing
                          data remanence.

    Open the Serial Monitor at 9600 baud to view output.

    Circuit: no external components required.
***/

#include <UserProfile.h>

UserProfile profile;

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port (needed for native USB boards)
    }

    Serial.println("=== UserProfile Privacy Example ===\n");

    // -----------------------------------------------------------------------
    // 1. Create and save a profile
    // -----------------------------------------------------------------------
    profile.setName("Alice Smith");
    profile.setEmail("alice@example.com");
    profile.setDeviceId(0xDEADBEEFUL);
    profile.save();

    Serial.println("Profile saved.");
    Serial.print("  Name:  "); Serial.println(profile.getName());
    Serial.print("  Email: "); Serial.println(profile.getEmail());

    // -----------------------------------------------------------------------
    // 2. Enable privacy mode and show masked values
    // -----------------------------------------------------------------------
    Serial.println("\nEnabling privacy mode...");
    profile.setPrivacyEnabled(true);
    profile.save(); // persist the new preference flag

    Serial.print("  Masked name:  "); Serial.println(profile.getMaskedName());
    Serial.print("  Masked email: "); Serial.println(profile.getMaskedEmail());

    // Plain getters are unaffected by privacy mode
    Serial.print("  Plain name:   "); Serial.println(profile.getName());
    Serial.print("  Plain email:  "); Serial.println(profile.getEmail());

    // -----------------------------------------------------------------------
    // 3. Disable privacy mode and confirm plain values are shown again
    // -----------------------------------------------------------------------
    Serial.println("\nDisabling privacy mode...");
    profile.setPrivacyEnabled(false);

    Serial.print("  getMaskedName()  now returns: "); Serial.println(profile.getMaskedName());
    Serial.print("  getMaskedEmail() now returns: "); Serial.println(profile.getMaskedEmail());

    // -----------------------------------------------------------------------
    // 4. Secure wipe
    // -----------------------------------------------------------------------
    Serial.println("\nPerforming secure wipe...");
    profile.wipe();

    if (!profile.exists()) {
        Serial.println("EEPROM profile erased – exists() returns false.");
    }

    // Attempting to read after wipe returns empty strings / zero
    Serial.print("  getName() after wipe:  \""); Serial.print(profile.getName()); Serial.println("\"");
    Serial.print("  getEmail() after wipe: \""); Serial.print(profile.getEmail()); Serial.println("\"");
    Serial.print("  getDeviceId() after wipe: "); Serial.println(profile.getDeviceId());
}

void loop() {
    // Nothing to do here
}
