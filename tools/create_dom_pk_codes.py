#!/usr/bin/env python
# coding=utf-8
# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# The DOM KeyboardEvent field 'code' contains a locale/language independent
# identifier of a pressed key on the keyboard, i.e. a "physical" keyboard code, which
# is often useful for games that want to provide a physical keyboard layout that does
# not get confused by the language setting the user has.

# For example, in Unreal Engine 4 developer mode, the physical keyboard key above the Tab
# but below the Esc key should open up the developer console, independent of which keyboard
# layout is active. This key produces the backquote (`) character on US keyboard layout,
# whereas on the Finnish/Swedish keyboard layout, it generates but the section sign (§)
# character. Other keyboard layouts might give different characters, and independent of
# which character is produced, we would like to generate a layout-agnostic identifier for
# the key at this physical location on the keyboard.

# The DOM KeyboardEvent field 'code' provides such a layout-agnostic identifier.
# Unfortunately this identifier is not an integral ID that could be used as an enum
# or #define, but it is a human-readable English language string that represents the
# physical key. This is very inconvenient for most applications.

# This utility script creates a mapping from the different documented values of the
# KeyboardEvent 'code' field, to integral IDs that can be easily used to identify
# physical key locations. This mapping is implemented by constructing a hash function
# that is a perfect hash (https://en.wikipedia.org/wiki/Perfect_hash_function) of the
# known strings.

# Use #include <emscripten/dom_pk_codes.h> in your code to access these IDs.

from __future__ import print_function
import sys, random

input_strings = [
  (0x0, 'Unidentified',          'DOM_PK_UNKNOWN'),
  (0x1, 'Escape',                'DOM_PK_ESCAPE'),
  (0x2, 'Digit0',                'DOM_PK_0'),
  (0x3, 'Digit1',                'DOM_PK_1'),
  (0x4, 'Digit2',                'DOM_PK_2'),
  (0x5, 'Digit3',                'DOM_PK_3'),
  (0x6, 'Digit4',                'DOM_PK_4'),
  (0x7, 'Digit5',                'DOM_PK_5'),
  (0x8, 'Digit6',                'DOM_PK_6'),
  (0x9, 'Digit7',                'DOM_PK_7'),
  (0xA, 'Digit8',                'DOM_PK_8'),
  (0xB, 'Digit9',                'DOM_PK_9'),
  (0xC,  'Minus',                'DOM_PK_MINUS'),
  (0xD,  'Equal',                'DOM_PK_EQUAL'),
  (0xE,  'Backspace',            'DOM_PK_BACKSPACE'),
  (0xF,  'Tab',                  'DOM_PK_TAB'),
  (0x10, 'KeyQ',                 'DOM_PK_Q'),
  (0x11, 'KeyW',                 'DOM_PK_W'),
  (0x12, 'KeyE',                 'DOM_PK_E'),
  (0x13, 'KeyR',                 'DOM_PK_R'),
  (0x14, 'KeyT',                 'DOM_PK_T'),
  (0x15, 'KeyY',                 'DOM_PK_Y'),
  (0x16, 'KeyU',                 'DOM_PK_U'),
  (0x17, 'KeyI',                 'DOM_PK_I'),
  (0x18, 'KeyO',                 'DOM_PK_O'),
  (0x19, 'KeyP',                 'DOM_PK_P'),
  (0x1A, 'BracketLeft',          'DOM_PK_BRACKET_LEFT'),
  (0x1B, 'BracketRight',         'DOM_PK_BRACKET_RIGHT'),
  (0x1C, 'Enter',                'DOM_PK_ENTER'),
  (0x1D, 'ControlLeft',          'DOM_PK_CONTROL_LEFT'),
  (0x1E, 'KeyA',                 'DOM_PK_A'),
  (0x1F, 'KeyS',                 'DOM_PK_S'),
  (0x20, 'KeyD',                 'DOM_PK_D'),
  (0x21, 'KeyF',                 'DOM_PK_F'),
  (0x22, 'KeyG',                 'DOM_PK_G'),
  (0x23, 'KeyH',                 'DOM_PK_H'),
  (0x24, 'KeyJ',                 'DOM_PK_J'),
  (0x25, 'KeyK',                 'DOM_PK_K'),
  (0x26, 'KeyL',                 'DOM_PK_L'),
  (0x27, 'Semicolon',            'DOM_PK_SEMICOLON'),
  (0x28, 'Quote',                'DOM_PK_QUOTE'),
  (0x29, 'Backquote',            'DOM_PK_BACKQUOTE'),
  (0x2A, 'ShiftLeft',            'DOM_PK_SHIFT_LEFT'),
  (0x2B, 'Backslash',            'DOM_PK_BACKSLASH'),
  (0x2C, 'KeyZ',                 'DOM_PK_Z'),
  (0x2D, 'KeyX',                 'DOM_PK_X'),
  (0x2E, 'KeyC',                 'DOM_PK_C'),
  (0x2F, 'KeyV',                 'DOM_PK_V'),
  (0x30, 'KeyB',                 'DOM_PK_B'),
  (0x31, 'KeyN',                 'DOM_PK_N'),
  (0x32, 'KeyM',                 'DOM_PK_M'),
  (0x33, 'Comma',                'DOM_PK_COMMA'),
  (0x34, 'Period',               'DOM_PK_PERIOD'),
  (0x35, 'Slash',                'DOM_PK_SLASH'),
  (0x36, 'ShiftRight',           'DOM_PK_SHIFT_RIGHT'),
  (0x37, 'NumpadMultiply',       'DOM_PK_NUMPAD_MULTIPLY'),
  (0x38, 'AltLeft',              'DOM_PK_ALT_LEFT'),
  (0x39, 'Space',                'DOM_PK_SPACE'),
  (0x3A, 'CapsLock',             'DOM_PK_CAPS_LOCK'),
  (0x3B, 'F1',                   'DOM_PK_F1'),
  (0x3C, 'F2',                   'DOM_PK_F2'),
  (0x3D, 'F3',                   'DOM_PK_F3'),
  (0x3E, 'F4',                   'DOM_PK_F4'),
  (0x3F, 'F5',                   'DOM_PK_F5'),
  (0x40, 'F6',                   'DOM_PK_F6'),
  (0x41, 'F7',                   'DOM_PK_F7'),
  (0x42, 'F8',                   'DOM_PK_F8'),
  (0x43, 'F9',                   'DOM_PK_F9'),
  (0x44, 'F10',                  'DOM_PK_F10'),
  (0x45, 'Pause',                'DOM_PK_PAUSE'),
  (0x46, 'ScrollLock',           'DOM_PK_SCROLL_LOCK'),
  (0x47, 'Numpad7',              'DOM_PK_NUMPAD_7'),
  (0x48, 'Numpad8',              'DOM_PK_NUMPAD_8'),
  (0x49, 'Numpad9',              'DOM_PK_NUMPAD_9'),
  (0x4A, 'NumpadSubtract',       'DOM_PK_NUMPAD_SUBTRACT'),
  (0x4B, 'Numpad4',              'DOM_PK_NUMPAD_4'),
  (0x4C, 'Numpad5',              'DOM_PK_NUMPAD_5'),
  (0x4D, 'Numpad6',              'DOM_PK_NUMPAD_6'),
  (0x4E, 'NumpadAdd',            'DOM_PK_NUMPAD_ADD'),
  (0x4F, 'Numpad1',              'DOM_PK_NUMPAD_1'),
  (0x50, 'Numpad2',              'DOM_PK_NUMPAD_2'),
  (0x51, 'Numpad3',              'DOM_PK_NUMPAD_3'),
  (0x52, 'Numpad0',              'DOM_PK_NUMPAD_0'),
  (0x53, 'NumpadDecimal',        'DOM_PK_NUMPAD_DECIMAL'),
  (0x54, 'PrintScreen',          'DOM_PK_PRINT_SCREEN'),
# 0x0055 'Unidentified', ''
  (0x56, 'IntlBackslash',        'DOM_PK_INTL_BACKSLASH'),
  (0x57, 'F11',                  'DOM_PK_F11'),
  (0x58, 'F12',                  'DOM_PK_F12'),
  (0x59, 'NumpadEqual',          'DOM_PK_NUMPAD_EQUAL'),
# 0x005A 'Unidentified', ''
# 0x005B 'Unidentified', ''
# 0x005C 'Unidentified', ''
# 0x005D 'Unidentified', ''
# 0x005E 'Unidentified', ''
# 0x005F 'Unidentified', ''
# 0x0060 'Unidentified', ''
# 0x0061 'Unidentified', ''
# 0x0062 'Unidentified', ''
# 0x0063 'Unidentified', ''
  (0x64, 'F13',                  'DOM_PK_F13'),
  (0x65, 'F14',                  'DOM_PK_F14'),
  (0x66, 'F15',                  'DOM_PK_F15'),
  (0x67, 'F16',                  'DOM_PK_F16'),
  (0x68, 'F17',                  'DOM_PK_F17'),
  (0x69, 'F18',                  'DOM_PK_F18'),
  (0x6A, 'F19',                  'DOM_PK_F19'),
  (0x6B, 'F20',                  'DOM_PK_F20'),
  (0x6C, 'F21',                  'DOM_PK_F21'),
  (0x6D, 'F22',                  'DOM_PK_F22'),
  (0x6E, 'F23',                  'DOM_PK_F23'),
# 0x006F 'Unidentified', ''
  (0x70, 'KanaMode',             'DOM_PK_KANA_MODE'),
  (0x71, 'Lang2',                'DOM_PK_LANG_2'),
  (0x72, 'Lang1',                'DOM_PK_LANG_1'),
  (0x73, 'IntlRo',               'DOM_PK_INTL_RO'),
# 0x0074 'Unidentified', ''
# 0x0075 'Unidentified', ''
  (0x76, 'F24',                  'DOM_PK_F24'),
# 0x0077 'Unidentified', ''
# 0x0078 'Unidentified', ''
  (0x79, 'Convert',              'DOM_PK_CONVERT'),
# 0x007A 'Unidentified', ''
  (0x7B, 'NonConvert',           'DOM_PK_NON_CONVERT'),
# 0x007C 'Unidentified', ''
  (0x7D, 'IntlYen',              'DOM_PK_INTL_YEN'),
  (0x7E, 'NumpadComma',          'DOM_PK_NUMPAD_COMMA'),
# 0x007F 'Unidentified', ''
  (0xE00A, 'Paste',              'DOM_PK_PASTE'),
  (0xE010, 'MediaTrackPrevious', 'DOM_PK_MEDIA_TRACK_PREVIOUS'),
  (0xE017, 'Cut',                'DOM_PK_CUT'),
  (0xE018, 'Copy',               'DOM_PK_COPY'),
  (0xE019, 'MediaTrackNext',     'DOM_PK_MEDIA_TRACK_NEXT'),
  (0xE01C, 'NumpadEnter',        'DOM_PK_NUMPAD_ENTER'),
  (0xE01D, 'ControlRight',       'DOM_PK_CONTROL_RIGHT'),
  (0xE020, 'AudioVolumeMute',    'DOM_PK_AUDIO_VOLUME_MUTE'),
  (0xE020, 'VolumeMute',         'DOM_PK_AUDIO_VOLUME_MUTE', 'duplicate'),
  (0xE021, 'LaunchApp2',         'DOM_PK_LAUNCH_APP_2'),
  (0xE022, 'MediaPlayPause',     'DOM_PK_MEDIA_PLAY_PAUSE'),
  (0xE024, 'MediaStop',          'DOM_PK_MEDIA_STOP'),
  (0xE02C, 'Eject',              'DOM_PK_EJECT'),
  (0xE02E, 'AudioVolumeDown',    'DOM_PK_AUDIO_VOLUME_DOWN'),
  (0xE02E, 'VolumeDown',         'DOM_PK_AUDIO_VOLUME_DOWN', 'duplicate'),
  (0xE030, 'AudioVolumeUp',      'DOM_PK_AUDIO_VOLUME_UP'),
  (0xE030, 'VolumeUp',           'DOM_PK_AUDIO_VOLUME_UP', 'duplicate'),
  (0xE032, 'BrowserHome',        'DOM_PK_BROWSER_HOME'),
  (0xE035, 'NumpadDivide',       'DOM_PK_NUMPAD_DIVIDE'),
#  (0xE037, 'PrintScreen',        'DOM_PK_PRINT_SCREEN'),
  (0xE038, 'AltRight',           'DOM_PK_ALT_RIGHT'),
  (0xE03B, 'Help',               'DOM_PK_HELP'),
  (0xE045, 'NumLock',            'DOM_PK_NUM_LOCK'),
#  (0xE046, 'Pause', 'DOM_PK_'), # Says Ctrl+Pause
  (0xE047, 'Home',               'DOM_PK_HOME'),
  (0xE048, 'ArrowUp',            'DOM_PK_ARROW_UP'),
  (0xE049, 'PageUp',             'DOM_PK_PAGE_UP'),
  (0xE04B, 'ArrowLeft',          'DOM_PK_ARROW_LEFT'),
  (0xE04D, 'ArrowRight',         'DOM_PK_ARROW_RIGHT'),
  (0xE04F, 'End',                'DOM_PK_END'),
  (0xE050, 'ArrowDown',          'DOM_PK_ARROW_DOWN'),
  (0xE051, 'PageDown',           'DOM_PK_PAGE_DOWN'),
  (0xE052, 'Insert',             'DOM_PK_INSERT'),
  (0xE053, 'Delete',             'DOM_PK_DELETE'),
  (0xE05B, 'MetaLeft',           'DOM_PK_META_LEFT'),
  (0xE05B, 'OSLeft',             'DOM_PK_OS_LEFT', 'duplicate'),
  (0xE05C, 'MetaRight',          'DOM_PK_META_RIGHT'),
  (0xE05C, 'OSRight',            'DOM_PK_OS_RIGHT', 'duplicate'),
  (0xE05D, 'ContextMenu',        'DOM_PK_CONTEXT_MENU'),
  (0xE05E, 'Power',              'DOM_PK_POWER'),
  (0xE065, 'BrowserSearch',      'DOM_PK_BROWSER_SEARCH'),
  (0xE066, 'BrowserFavorites',   'DOM_PK_BROWSER_FAVORITES'),
  (0xE067, 'BrowserRefresh',     'DOM_PK_BROWSER_REFRESH'),
  (0xE068, 'BrowserStop',        'DOM_PK_BROWSER_STOP'),
  (0xE069, 'BrowserForward',     'DOM_PK_BROWSER_FORWARD'),
  (0xE06A, 'BrowserBack',        'DOM_PK_BROWSER_BACK'),
  (0xE06B, 'LaunchApp1',         'DOM_PK_LAUNCH_APP_1'),
  (0xE06C, 'LaunchMail',         'DOM_PK_LAUNCH_MAIL'),
  (0xE06D, 'LaunchMediaPlayer',  'DOM_PK_LAUNCH_MEDIA_PLAYER'),
  (0xE06D, 'MediaSelect',        'DOM_PK_MEDIA_SELECT', 'duplicate')
#  (0xE0F1, 'Lang2', 'DOM_PK_'), Hanja key
#  (0xE0F2, 'Lang2', 'DOM_PK_'), Han/Yeong
]

def hash(s, k1, k2):
  h = 0
  for c in s:
    h = int(int(int(h ^ k1) << k2) ^ ord(c)) & 0xFFFFFFFF
  return h

def hash_all(k1, k2):
  hashes = {}
  str_to_hash = {}
  for s in input_strings:
    h = hash(s[1], k1, k2)
    print('String "' + s[1] + '" hashes to %s ' % hex(h), file=sys.stderr)
    if h in hashes:
      print('Collision! Earlier string ' + hashes[h] + ' also hashed to %s!' % hex(h), file=sys.stderr)
      return None
    else:
      hashes[h] = s[1]
      str_to_hash[s[1]] = h
  return (hashes, str_to_hash)

# Find an approprite hash function that is collision free within the set of all input strings
# Try hash function format h_i = ((h_(i-1) ^ k_1) << k_2) ^ s_i, where h_i is the hash function
# value at step i, k_1 and k_2 are the constants we are searching, and s_i is the i'th input
# character
perfect_hash_table = None
while perfect_hash_table == None:
  # The search space is super-narrow, but since there are so few items to hash, practically
  # almost any choice gives a collision free hash.
  k1 = int(random.randint(0, 0x7FFFFFFF))
  k2 = int(random.uniform(1, 8))
  perfect_hash_table = hash_all(k1, k2)
hash_to_str, str_to_hash = perfect_hash_table

print('Found collision-free hash function!', file=sys.stderr)
print('h_i = ((h_(i-1) ^ %s) << %s) ^ s_i' % (hex(k1), hex(k2)), file=sys.stderr)

def pad_to_length(s, length):
  return s + max(0, length - len(s)) * ' '

def longest_dom_pk_code_length():
  return max(map(len, [x[2] for x in input_strings]))

def longest_key_code_length():
  return max(map(len, [x[1] for x in input_strings]))

h_file = open('system/include/emscripten/dom_pk_codes.h', 'w')
c_file = open('system/lib/html5/dom_pk_codes.c', 'w')

# Generate the output file:

h_file.write('''\
/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * This file was automatically generated from script
 * tools/create_dom_pk_codes.py. Edit that file to make changes here.
 * Run
 *
 *   tools/create_dom_pk_codes.py
 *
 * in Emscripten root directory to regenerate this file.
 */

#pragma once

#define DOM_PK_CODE_TYPE int

''')

c_file.write('''/* This file was automatically generated from script
tools/create_dom_pk_codes.py. Edit that file to make changes here.
Run

  python tools/create_dom_pk_codes.py

in Emscripten root directory to regenerate this file. */

#include <emscripten/dom_pk_codes.h>
''')

for s in input_strings:
  h_file.write('#define ' + pad_to_length(s[2], longest_dom_pk_code_length()) + ' 0x%04X /* "%s */' % (s[0], pad_to_length(s[1] + '"', longest_key_code_length()+1)) + '\n')

h_file.write('''
#ifdef __cplusplus
extern "C" {
#endif
/* Maps the EmscriptenKeyboardEvent::code field from emscripten/html5.h to one of the DOM_PK codes above. */
DOM_PK_CODE_TYPE emscripten_compute_dom_pk_code(const char *keyCodeString);

/* Returns the string representation of the given key code ID. Useful for debug printing. */
const char *emscripten_dom_pk_code_to_string(DOM_PK_CODE_TYPE code);
#ifdef __cplusplus
}
#endif
''')

c_file.write('''
DOM_PK_CODE_TYPE emscripten_compute_dom_pk_code(const char *keyCodeString)
{
  if (!keyCodeString) return 0;

  /* Compute the collision free hash. */
  unsigned int hash = 0;
  while(*keyCodeString) hash = ((hash ^ 0x%04XU) << %d) ^ (unsigned int)*keyCodeString++;

  /* Don't expose the hash values out to the application, but map to fixed IDs. This is useful for
     mapping back codes to MDN documentation page at

       https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/code */
  switch(hash)
  {
''' % (k1, k2))

for s in input_strings:
  c_file.write('    case 0x%08XU /* %s */: return %s /* 0x%04X */' % (str_to_hash[s[1]], pad_to_length(s[1], longest_key_code_length()), pad_to_length(s[2] + ';', longest_dom_pk_code_length()+1), s[0]) + '\n')

c_file.write('''    default: return DOM_PK_UNKNOWN;
  }
}

const char *emscripten_dom_pk_code_to_string(DOM_PK_CODE_TYPE code)
{
  switch(code)
  {
''')

for s in input_strings:
  if len(s) == 3:
    c_file.write('    case %s return "%s";' % (pad_to_length(s[2] + ':', longest_dom_pk_code_length()+1), s[2]) + '\n')

c_file.write('''    default: return "Unknown DOM_PK code";
  }
}
''')
