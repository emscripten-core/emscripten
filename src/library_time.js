/**
 * @license
 * Copyright 2024 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

assert(!STANDALONE_WASM, "library_time.js should not be included in standalone mode");

addToLibrary({
  _mktime_js__i53abi: true,
  _mktime_js__deps: ['$ydayFromDate'],
  _mktime_js: (tmPtr) => {
    var date = new Date({{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}} + 1900,
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
                        0);

    // There's an ambiguous hour when the time goes back; the tm_isdst field is
    // used to disambiguate it.  Date() basically guesses, so we fix it up if it
    // guessed wrong, or fill in tm_isdst with the guess if it's -1.
    var dst = {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'i32') }}};
    var guessedOffset = date.getTimezoneOffset();
    var start = new Date(date.getFullYear(), 0, 1);
    var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();
    var dstOffset = Math.min(winterOffset, summerOffset); // DST is in December in South
    if (dst < 0) {
      // Attention: some regions don't have DST at all.
      {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'Number(summerOffset != winterOffset && dstOffset == guessedOffset)', 'i32') }}};
    } else if ((dst > 0) != (dstOffset == guessedOffset)) {
      var nonDstOffset = Math.max(winterOffset, summerOffset);
      var trueOffset = dst > 0 ? dstOffset : nonDstOffset;
      // Don't try setMinutes(date.getMinutes() + ...) -- it's messed up.
      date.setTime(date.getTime() + (trueOffset - guessedOffset)*60000);
    }

    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getDay()', 'i32') }}};
    var yday = ydayFromDate(date)|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
    // To match expected behavior, update fields from date
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getYear()', 'i32') }}};

    var timeMs = date.getTime();
    if (isNaN(timeMs)) {
      return -1;
    }
    // Return time in microseconds
    return timeMs / 1000;
  },

  _gmtime_js__i53abi: true,
  _gmtime_js: (time, tmPtr) => {
    var date = new Date(time * 1000);
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getUTCSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getUTCMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getUTCHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getUTCDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getUTCMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getUTCFullYear()-1900', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getUTCDay()', 'i32') }}};
    var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
    var yday = ((date.getTime() - start) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
  },

  _timegm_js__i53abi: true,
  _timegm_js: (tmPtr) => {
    var time = Date.UTC({{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}} + 1900,
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
                        0);
    var date = new Date(time);

    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getUTCDay()', 'i32') }}};
    var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
    var yday = ((date.getTime() - start) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};

    return date.getTime() / 1000;
  },

  _localtime_js__i53abi: true,
  _localtime_js__deps: ['$ydayFromDate'],
  _localtime_js: (time, tmPtr) => {
    var date = new Date(time*1000);
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getFullYear()-1900', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getDay()', 'i32') }}};

    var yday = ydayFromDate(date)|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_gmtoff, '-(date.getTimezoneOffset() * 60)', LONG_TYPE) }}};

    // Attention: DST is in December in South, and some regions don't have DST at all.
    var start = new Date(date.getFullYear(), 0, 1);
    var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();
    var dst = (summerOffset != winterOffset && date.getTimezoneOffset() == Math.min(winterOffset, summerOffset))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'dst', 'i32') }}};
  },

  // musl-internal function used to implement both `asctime` and `asctime_r`
  __asctime_r: (tmPtr, buf) => {
    var date = {
      tm_sec: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
      tm_min: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
      tm_hour: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
      tm_mday: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
      tm_mon: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
      tm_year: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}},
      tm_wday: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'i32') }}}
    };
    var days = [ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" ];
    var months = [ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" ];
    var s = days[date.tm_wday] + ' ' + months[date.tm_mon] +
        (date.tm_mday < 10 ? '  ' : ' ') + date.tm_mday +
        (date.tm_hour < 10 ? ' 0' : ' ') + date.tm_hour +
        (date.tm_min < 10 ? ':0' : ':') + date.tm_min +
        (date.tm_sec < 10 ? ':0' : ':') + date.tm_sec +
        ' ' + (1900 + date.tm_year) + "\n";

    // asctime_r is specced to behave in an undefined manner if the algorithm would attempt
    // to write out more than 26 bytes (including the null terminator).
    // See http://pubs.opengroup.org/onlinepubs/9699919799/functions/asctime.html
    // Our undefined behavior is to truncate the write to at most 26 bytes, including null terminator.
    stringToUTF8(s, buf, 26);
    return buf;
  },

  _tzset_js__deps: ['$stringToUTF8',
#if ASSERTIONS
    '$lengthBytesUTF8',
#endif
  ],
  _tzset_js__internal: true,
  _tzset_js: (timezone, daylight, std_name, dst_name) => {
    // TODO: Use (malleable) environment variables instead of system settings.
    var currentYear = new Date().getFullYear();
    var winter = new Date(currentYear, 0, 1);
    var summer = new Date(currentYear, 6, 1);
    var winterOffset = winter.getTimezoneOffset();
    var summerOffset = summer.getTimezoneOffset();

    // Local standard timezone offset. Local standard time is not adjusted for
    // daylight savings.  This code uses the fact that getTimezoneOffset returns
    // a greater value during Standard Time versus Daylight Saving Time (DST).
    // Thus it determines the expected output during Standard Time, and it
    // compares whether the output of the given date the same (Standard) or less
    // (DST).
    var stdTimezoneOffset = Math.max(winterOffset, summerOffset);

    // timezone is specified as seconds west of UTC ("The external variable
    // `timezone` shall be set to the difference, in seconds, between
    // Coordinated Universal Time (UTC) and local standard time."), the same
    // as returned by stdTimezoneOffset.
    // See http://pubs.opengroup.org/onlinepubs/009695399/functions/tzset.html
    {{{ makeSetValue('timezone', '0', 'stdTimezoneOffset * 60', POINTER_TYPE) }}};

    {{{ makeSetValue('daylight', '0', 'Number(winterOffset != summerOffset)', 'i32') }}};

    var extractZone = (timezoneOffset) => {
      // Why inverse sign?
      // Read here https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date/getTimezoneOffset
      var sign = timezoneOffset >= 0 ? "-" : "+";

      var absOffset = Math.abs(timezoneOffset)
      var hours = String(Math.floor(absOffset / 60)).padStart(2, "0");
      var minutes = String(absOffset % 60).padStart(2, "0");

      return `UTC${sign}${hours}${minutes}`;
    }

    var winterName = extractZone(winterOffset);
    var summerName = extractZone(summerOffset);
#if ASSERTIONS
    assert(winterName);
    assert(summerName);
    assert(lengthBytesUTF8(winterName) <= {{{ cDefs.TZNAME_MAX }}}, `timezone name truncated to fit in TZNAME_MAX (${winterName})`);
    assert(lengthBytesUTF8(summerName) <= {{{ cDefs.TZNAME_MAX }}}, `timezone name truncated to fit in TZNAME_MAX (${summerName})`);
#endif
    if (summerOffset < winterOffset) {
      // Northern hemisphere
      stringToUTF8(winterName, std_name, {{{ cDefs.TZNAME_MAX + 1 }}});
      stringToUTF8(summerName, dst_name, {{{ cDefs.TZNAME_MAX + 1 }}});
    } else {
      stringToUTF8(winterName, dst_name, {{{ cDefs.TZNAME_MAX + 1 }}});
      stringToUTF8(summerName, std_name, {{{ cDefs.TZNAME_MAX + 1 }}});
    }
  },

  $MONTH_DAYS_REGULAR: [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
  $MONTH_DAYS_LEAP: [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
  $MONTH_DAYS_REGULAR_CUMULATIVE: [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334],
  $MONTH_DAYS_LEAP_CUMULATIVE: [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335],

  $isLeapYear: (year) => year%4 === 0 && (year%100 !== 0 || year%400 === 0),

  $ydayFromDate__deps: ['$isLeapYear', '$MONTH_DAYS_LEAP_CUMULATIVE', '$MONTH_DAYS_REGULAR_CUMULATIVE'],
  $ydayFromDate: (date) => {
    var leap = isLeapYear(date.getFullYear());
    var monthDaysCumulative = (leap ? MONTH_DAYS_LEAP_CUMULATIVE : MONTH_DAYS_REGULAR_CUMULATIVE);
    var yday = monthDaysCumulative[date.getMonth()] + date.getDate() - 1; // -1 since it's days since Jan 1

    return yday;
  },

  $arraySum: (array, index) => {
    var sum = 0;
    for (var i = 0; i <= index; sum += array[i++]) {
      // no-op
    }
    return sum;
  },

  $addDays__deps: ['$isLeapYear', '$MONTH_DAYS_LEAP', '$MONTH_DAYS_REGULAR'],
  $addDays: (date, days) => {
    var newDate = new Date(date.getTime());
    while (days > 0) {
      var leap = isLeapYear(newDate.getFullYear());
      var currentMonth = newDate.getMonth();
      var daysInCurrentMonth = (leap ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR)[currentMonth];

      if (days > daysInCurrentMonth-newDate.getDate()) {
        // we spill over to next month
        days -= (daysInCurrentMonth-newDate.getDate()+1);
        newDate.setDate(1);
        if (currentMonth < 11) {
          newDate.setMonth(currentMonth+1)
        } else {
          newDate.setMonth(0);
          newDate.setFullYear(newDate.getFullYear()+1);
        }
      } else {
        // we stay in current month
        newDate.setDate(newDate.getDate()+days);
        return newDate;
      }
    }

    return newDate;
  },

  strptime__deps: ['$isLeapYear', '$arraySum', '$addDays', '$MONTH_DAYS_REGULAR', '$MONTH_DAYS_LEAP',
                   '$jstoi_q', '$intArrayFromString' ],
  strptime: (buf, format, tm) => {
    // char *strptime(const char *restrict buf, const char *restrict format, struct tm *restrict tm);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/strptime.html
    var pattern = UTF8ToString(format);

    // escape special characters
    // TODO: not sure we really need to escape all of these in JS regexps
    var SPECIAL_CHARS = '\\!@#$^&*()+=-[]/{}|:<>?,.';
    for (var i=0, ii=SPECIAL_CHARS.length; i<ii; ++i) {
      pattern = pattern.replace(new RegExp('\\'+SPECIAL_CHARS[i], 'g'), '\\'+SPECIAL_CHARS[i]);
    }

    // reduce number of matchers
    var EQUIVALENT_MATCHERS = {
      'A':  '%a',
      'B':  '%b',
      'c':  '%a %b %d %H:%M:%S %Y',
      'D':  '%m\\/%d\\/%y',
      'e':  '%d',
      'F':  '%Y-%m-%d',
      'h':  '%b',
      'R':  '%H\\:%M',
      'r':  '%I\\:%M\\:%S\\s%p',
      'T':  '%H\\:%M\\:%S',
      'x':  '%m\\/%d\\/(?:%y|%Y)',
      'X':  '%H\\:%M\\:%S'
    };
    // TODO: take care of locale

    var DATE_PATTERNS = {
      /* weekday name */    'a': '(?:Sun(?:day)?)|(?:Mon(?:day)?)|(?:Tue(?:sday)?)|(?:Wed(?:nesday)?)|(?:Thu(?:rsday)?)|(?:Fri(?:day)?)|(?:Sat(?:urday)?)',
      /* month name */      'b': '(?:Jan(?:uary)?)|(?:Feb(?:ruary)?)|(?:Mar(?:ch)?)|(?:Apr(?:il)?)|May|(?:Jun(?:e)?)|(?:Jul(?:y)?)|(?:Aug(?:ust)?)|(?:Sep(?:tember)?)|(?:Oct(?:ober)?)|(?:Nov(?:ember)?)|(?:Dec(?:ember)?)',
      /* century */         'C': '\\d\\d',
      /* day of month */    'd': '0[1-9]|[1-9](?!\\d)|1\\d|2\\d|30|31',
      /* hour (24hr) */     'H': '\\d(?!\\d)|[0,1]\\d|20|21|22|23',
      /* hour (12hr) */     'I': '\\d(?!\\d)|0\\d|10|11|12',
      /* day of year */     'j': '00[1-9]|0?[1-9](?!\\d)|0?[1-9]\\d(?!\\d)|[1,2]\\d\\d|3[0-6]\\d',
      /* month */           'm': '0[1-9]|[1-9](?!\\d)|10|11|12',
      /* minutes */         'M': '0\\d|\\d(?!\\d)|[1-5]\\d',
      /* whitespace */      'n': ' ',
      /* AM/PM */           'p': 'AM|am|PM|pm|A\\.M\\.|a\\.m\\.|P\\.M\\.|p\\.m\\.',
      /* seconds */         'S': '0\\d|\\d(?!\\d)|[1-5]\\d|60',
      /* week number */     'U': '0\\d|\\d(?!\\d)|[1-4]\\d|50|51|52|53',
      /* week number */     'W': '0\\d|\\d(?!\\d)|[1-4]\\d|50|51|52|53',
      /* weekday number */  'w': '[0-6]',
      /* 2-digit year */    'y': '\\d\\d',
      /* 4-digit year */    'Y': '\\d\\d\\d\\d',
      /* whitespace */      't': ' ',
      /* time zone */       'z': 'Z|(?:[\\+\\-]\\d\\d:?(?:\\d\\d)?)'
    };

    var MONTH_NUMBERS = {JAN: 0, FEB: 1, MAR: 2, APR: 3, MAY: 4, JUN: 5, JUL: 6, AUG: 7, SEP: 8, OCT: 9, NOV: 10, DEC: 11};
    var DAY_NUMBERS_SUN_FIRST = {SUN: 0, MON: 1, TUE: 2, WED: 3, THU: 4, FRI: 5, SAT: 6};
    var DAY_NUMBERS_MON_FIRST = {MON: 0, TUE: 1, WED: 2, THU: 3, FRI: 4, SAT: 5, SUN: 6};

    var capture = [];
    var pattern_out = pattern
      .replace(/%(.)/g, (m, c) => EQUIVALENT_MATCHERS[c] || m)
      .replace(/%(.)/g, (_, c) => {
        let pat = DATE_PATTERNS[c];
        if (pat){
          capture.push(c);
          return `(${pat})`;
        } else {
          return c;
        }
      })
      .replace( // any number of space or tab characters match zero or more spaces
        /\s+/g,'\\s*'
      );

    var matches = new RegExp('^'+pattern_out, "i").exec(UTF8ToString(buf))

    function initDate() {
      function fixup(value, min, max) {
        return (typeof value != 'number' || isNaN(value)) ? min : (value>=min ? (value<=max ? value: max): min);
      };
      return {
        year: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_year, 'i32') }}} + 1900 , 1970, 9999),
        month: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_mon, 'i32') }}}, 0, 11),
        day: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_mday, 'i32') }}}, 1, 31),
        hour: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_hour, 'i32') }}}, 0, 23),
        min: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_min, 'i32') }}}, 0, 59),
        sec: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_sec, 'i32') }}}, 0, 59),
        gmtoff: 0
      };
    };

    if (matches) {
      var date = initDate();
      var value;

      var getMatch = (symbol) => {
        var pos = capture.indexOf(symbol);
        // check if symbol appears in regexp
        if (pos >= 0) {
          // return matched value or null (falsy!) for non-matches
          return matches[pos+1];
        }
        return;
      };

      // seconds
      if ((value=getMatch('S'))) {
        date.sec = jstoi_q(value);
      }

      // minutes
      if ((value=getMatch('M'))) {
        date.min = jstoi_q(value);
      }

      // hours
      if ((value=getMatch('H'))) {
        // 24h clock
        date.hour = jstoi_q(value);
      } else if ((value = getMatch('I'))) {
        // AM/PM clock
        var hour = jstoi_q(value);
        if ((value=getMatch('p'))) {
          hour += value.toUpperCase()[0] === 'P' ? 12 : 0;
        }
        date.hour = hour;
      }

      // year
      if ((value=getMatch('Y'))) {
        // parse from four-digit year
        date.year = jstoi_q(value);
      } else if ((value=getMatch('y'))) {
        // parse from two-digit year...
        var year = jstoi_q(value);
        if ((value=getMatch('C'))) {
          // ...and century
          year += jstoi_q(value)*100;
        } else {
          // ...and rule-of-thumb
          year += year<69 ? 2000 : 1900;
        }
        date.year = year;
      }

      // month
      if ((value=getMatch('m'))) {
        // parse from month number
        date.month = jstoi_q(value)-1;
      } else if ((value=getMatch('b'))) {
        // parse from month name
        date.month = MONTH_NUMBERS[value.substring(0,3).toUpperCase()] || 0;
        // TODO: derive month from day in year+year, week number+day of week+year
      }

      // day
      if ((value=getMatch('d'))) {
        // get day of month directly
        date.day = jstoi_q(value);
      } else if ((value=getMatch('j'))) {
        // get day of month from day of year ...
        var day = jstoi_q(value);
        var leapYear = isLeapYear(date.year);
        for (var month=0; month<12; ++month) {
          var daysUntilMonth = arraySum(leapYear ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR, month-1);
          if (day<=daysUntilMonth+(leapYear ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR)[month]) {
            date.day = day-daysUntilMonth;
          }
        }
      } else if ((value=getMatch('a'))) {
        // get day of month from weekday ...
        var weekDay = value.substring(0,3).toUpperCase();
        if ((value=getMatch('U'))) {
          // ... and week number (Sunday being first day of week)
          // Week number of the year (Sunday as the first day of the week) as a decimal number [00,53].
          // All days in a new year preceding the first Sunday are considered to be in week 0.
          var weekDayNumber = DAY_NUMBERS_SUN_FIRST[weekDay];
          var weekNumber = jstoi_q(value);

          // January 1st
          var janFirst = new Date(date.year, 0, 1);
          var endDate;
          if (janFirst.getDay() === 0) {
            // Jan 1st is a Sunday, and, hence in the 1st CW
            endDate = addDays(janFirst, weekDayNumber+7*(weekNumber-1));
          } else {
            // Jan 1st is not a Sunday, and, hence still in the 0th CW
            endDate = addDays(janFirst, 7-janFirst.getDay()+weekDayNumber+7*(weekNumber-1));
          }
          date.day = endDate.getDate();
          date.month = endDate.getMonth();
        } else if ((value=getMatch('W'))) {
          // ... and week number (Monday being first day of week)
          // Week number of the year (Monday as the first day of the week) as a decimal number [00,53].
          // All days in a new year preceding the first Monday are considered to be in week 0.
          var weekDayNumber = DAY_NUMBERS_MON_FIRST[weekDay];
          var weekNumber = jstoi_q(value);

          // January 1st
          var janFirst = new Date(date.year, 0, 1);
          var endDate;
          if (janFirst.getDay()===1) {
            // Jan 1st is a Monday, and, hence in the 1st CW
             endDate = addDays(janFirst, weekDayNumber+7*(weekNumber-1));
          } else {
            // Jan 1st is not a Monday, and, hence still in the 0th CW
            endDate = addDays(janFirst, 7-janFirst.getDay()+1+weekDayNumber+7*(weekNumber-1));
          }

          date.day = endDate.getDate();
          date.month = endDate.getMonth();
        }
      }

      // time zone
      if ((value = getMatch('z'))) {
        // GMT offset as either 'Z' or +-HH:MM or +-HH or +-HHMM
        if (value.toLowerCase() === 'z'){
          date.gmtoff = 0;
        } else {          
          var match = value.match(/^((?:\-|\+)\d\d):?(\d\d)?/);
          date.gmtoff = match[1] * 3600;
          if (match[2]) {
            date.gmtoff += date.gmtoff >0 ? match[2] * 60 : -match[2] * 60
          }
        }
      }

      /*
      tm_sec  int seconds after the minute  0-61*
      tm_min  int minutes after the hour  0-59
      tm_hour int hours since midnight  0-23
      tm_mday int day of the month  1-31
      tm_mon  int months since January  0-11
      tm_year int years since 1900
      tm_wday int days since Sunday 0-6
      tm_yday int days since January 1  0-365
      tm_isdst  int Daylight Saving Time flag
      tm_gmtoff long offset from GMT (seconds)
      */

      var fullDate = new Date(date.year, date.month, date.day, date.hour, date.min, date.sec, 0);
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_sec, 'fullDate.getSeconds()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_min, 'fullDate.getMinutes()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_hour, 'fullDate.getHours()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_mday, 'fullDate.getDate()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_mon, 'fullDate.getMonth()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_year, 'fullDate.getFullYear()-1900', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_wday, 'fullDate.getDay()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_yday, 'arraySum(isLeapYear(fullDate.getFullYear()) ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR, fullDate.getMonth()-1)+fullDate.getDate()-1', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_isdst, '0', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_gmtoff, 'date.gmtoff', LONG_TYPE) }}};
 
      // we need to convert the matched sequence into an integer array to take care of UTF-8 characters > 0x7F
      // TODO: not sure that intArrayFromString handles all unicode characters correctly
      return buf+intArrayFromString(matches[0]).length-1;
    }

    return 0;
  },
  strptime_l__deps: ['strptime'],
  strptime_l: (buf, format, tm, locale) => {
    return _strptime(buf, format, tm); // no locale support yet
  },
});
