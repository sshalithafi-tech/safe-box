# Keypad Floating Pin Issue

## Problem
Keypad columns were reading random/ghost values without any key being pressed.
This caused false PIN entries and unpredictable behaviour.

## Root Cause
The column pins were left floating (no pull-up resistors configured).
Floating pins pick up electrical noise and read random HIGH/LOW values.

## Fix Applied
Used INPUT_PULLUP on all column pins so they default to HIGH when no key is pressed.
A key press pulls the pin LOW through the row — giving a clean, reliable signal.

## Test Sketch
keypad_floating_fix.ino demonstrates INPUT_PULLUP behaviour on a single button pin.