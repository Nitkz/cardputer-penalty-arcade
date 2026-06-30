#ifndef SOUNDEFFECTS_H
#define SOUNDEFFECTS_H

#include <M5Unified.h>

class SoundEffects {
public:
  void begin() {
    M5.Speaker.begin();
    M5.Speaker.setVolume(128); // Set a reasonable volume
  }

  // A quick high pitch beep when touched
  void playShootSound() {
    M5.Speaker.tone(1000, 100); // 1000Hz for 100ms
  }

  // A celebratory melody when a goal is scored
  void playGoalSound() {
    // Non-blocking tones using tone with duration
    // A quick arpeggio (C5, E5, G5)
    M5.Speaker.tone(523, 100);
    // Note: To keep it strictly non-blocking in a single call without tasks,
    // we can either play a single nice tone or use a simple sequence state machine.
    // For simplicity and avoiding blocking delay(), just one distinct celebratory tone here.
    // In a real scenario we'd use a timer to chain tones, but M5.Speaker can handle simple tones.
    M5.Speaker.tone(1046, 300); // High C
  }

  // A low disappointing buzz when saved
  void playSavedSound() {
    M5.Speaker.tone(150, 300); // Low frequency for 300ms
  }
};

#endif // SOUNDEFFECTS_H
