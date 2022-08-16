/*
 * Copyright (C) 2009-2010, Pino Toscano <pino@kde.org>
 * Copyright (C) 2010, Patrick Spendrin <ps_ml@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef POPPLER_GLOBAL_H
#define POPPLER_GLOBAL_H

#if defined(_WIN32)
#  define LIB_EXPORT __declspec(dllexport)
#  define LIB_IMPORT __declspec(dllimport)
#else
#  define LIB_EXPORT
#  define LIB_IMPORT
#endif

#if defined(poppler_cpp_EXPORTS)
#  define POPPLER_CPP_EXPORT LIB_EXPORT
#else
#  define POPPLER_CPP_EXPORT LIB_IMPORT
#endif

#include <iosfwd>
#include <string>
#include <vector>

namespace poppler
{

/// \cond DOXYGEN_SKIP_THIS
namespace detail
{

class POPPLER_CPP_EXPORT noncopyable
{
protected:
    noncopyable();
    ~noncopyable();
private:
    noncopyable(const noncopyable &);
    const noncopyable& operator=(const noncopyable &);
};

}

typedef detail::noncopyable noncopyable;
/// \endcond

enum rotation_enum { rotate_0, rotate_90, rotate_180, rotate_270 };

enum page_box_enum { media_box, crop_box, bleed_box, trim_box, art_box };

enum permission_enum { perm_print, perm_change, perm_copy, perm_add_notes,
                       perm_fill_forms, perm_accessibility, perm_assemble,
                       perm_print_high_resolution };

enum case_sensitivity_enum { case_sensitive, case_insensitive };

typedef std::vector<char> byte_array;

typedef unsigned int /* time_t */ time_type;

// to disable warning only for this occurrence
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) /* class 'A' needs to have dll interface for to be used by clients of class 'B'. */
#endif
class POPPLER_CPP_EXPORT ustring : public std::basic_string<unsigned short>
{
public:
    ustring();
    ustring(size_type len, value_type ch);
    ~ustring();

    byte_array to_utf8() const;
    std::string to_latin1() const;

    static ustring from_utf8(const char *str, int len = -1);
    static ustring from_latin1(const std::string &str);

private:
    // forbid implicit std::string conversions
    ustring(const std::string &);
    operator std::string() const;
    ustring& operator=(const std::string &);
};
#ifdef _MSC_VER
#pragma warning(pop)
#endif

POPPLER_CPP_EXPORT time_type convert_date(const std::string &date);

POPPLER_CPP_EXPORT std::ostream& operator<<(std::ostream& stream, const byte_array &array);

}

#endif
