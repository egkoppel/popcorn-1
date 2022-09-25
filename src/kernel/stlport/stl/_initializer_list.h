#ifndef _STLP_INTERNAL_INITLIST
#define _STLP_INTERNAL_INITLIST

#ifndef _STLP_INTERNAL_CSTDDEF
// size_t
#  include <stl/_cstddef.h>
#endif

#if defined (__BORLANDC__) && (__BORLANDC__ < 0x570)
// new.h uses ::malloc ;(
#  include _STLP_NATIVE_CPP_C_HEADER(cstdlib)
using _STLP_VENDOR_CSTD::malloc;
#endif

#if !defined (_STLP_NO_NEW_NEW_HEADER)
// eMbedded Visual C++ .NET unfortunately uses _INC_NEW for both <new.h> and <new>
// we undefine the symbol to get the stuff in the SDK's <new>
#  if defined (_STLP_WCE_NET) && defined (_INC_NEW)
#    undef _INC_NEW
#  endif

#  if defined (initializer_list)
/* STLport cannot replace native Std library new header if new is a macro,
 * please define new macro after <new> header inclusion.
 */
#    error Cannot include native initializer_list header as initializer_list is a macro.
#  endif

#  if defined (_STLP_HAS_INCLUDE_NEXT)
#    include_next <initializer_list>
#  else
#    include _STLP_NATIVE_CPP_RUNTIME_HEADER(new)
#  endif
#else
#  include <initializer_list.h>
#endif

#if defined (_STLP_USE_OWN_NAMESPACE)

_STLP_BEGIN_NAMESPACE

#if defined(__clang__)
// Copyright (c) 2019 Chandler Carruth <https://github.com/chandlerc>
// Copyright (c) 2018 Louis Dionne <https://github.com/ldionne>
// Copyright (c) 2017 Eric <https://github.com/EricWF>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ---- LLVM Exceptions to the Apache 2.0 License ----
//
// As an exception, if, as a result of your compiling your source code, portions
// of this Software are embedded into an Object form of such source code, you
// may redistribute such embedded portions in such Object form without complying
// with the conditions of Sections 4(a), 4(b) and 4(d) of the License.
//
// In addition, if you combine or link compiled forms of this Software with
// software that is licensed under the GPLv2 ("Combined Software") and if a
// court of competent jurisdiction determines that the patent provision (Section
// 3), the indemnity provision (Section 9) or other Section of the License
// conflicts with the conditions of the GPLv2, you may retroactively and
// prospectively choose to deem waived or otherwise exclude such Section(s) of
// the License, but only in their entirety and only with respect to the Combined
// Software.

    template<typename T>
    class initializer_list
    {
    private:
        const T* m_first;
        const T* m_last;

    public:
        using value_type      = T;
        using reference       = const T&;
        using const_reference = const T&;
        using size_type       = size_t;
        using iterator        = const T*;
        using const_iterator  = const T*;

        initializer_list() noexcept : m_first(nullptr), m_last(nullptr) {}

        // Number of elements.
        size_t size() const noexcept { return m_last - m_first; }

        // First element.
        const T* begin() const noexcept { return m_first; }

        // One past the last element.
        const T* end() const noexcept { return m_last; }
    };
#elif defined(__GNUC__)
// Copyright (C) 2008-2020 Free Software Foundation, Inc.
// Copyright (C) 2020 Daniel Rossinsky <danielrossinsky@gmail.com>
//
// This file is part of GCC.
//
// GCC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// GCC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

    template<typename T>
    class initializer_list
    {
    public:
        using value_type = T;
        using reference = const T&;
        using const_reference = const T&;
        using size_type = size_t;
        using iterator = const T*;
        using const_iterator = const T*;

    private:
        iterator  m_array;
        size_type m_len;

        // The compiler can call a private constructor.
        constexpr initializer_list(const_iterator itr, size_type st)
            : m_array(itr), m_len(st) { }

    public:
        constexpr initializer_list() noexcept : m_array(0), m_len(0) { }

        // Number of elements.
        constexpr size_type size() const noexcept { return m_len; }

        // First element.
        constexpr const_iterator begin() const noexcept { return m_array; }

        // One past the last element.
        constexpr const_iterator end() const noexcept { return begin() + size(); }
    };
#else
#  error "Unsupported compiler"
#endif

_STLP_END_NAMESPACE
#endif /* _STLP_USE_OWN_NAMESPACE */

#endif /* _STLP_INTERNAL_NEW */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
