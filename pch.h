
/** $VER: pch.h (2026.07.05) P. Stuer **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5039 5045 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>

#define NOMINMAX

#include <helpers\foobar2000+atl.h>
#include <helpers\helpers.h>

#include <sdk\autoplaylist.h>

#include <comdef.h> // For _com_error

#include <stdlib.h>
#include <strsafe.h>

#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <memory>
#include <ranges>
#include <string>

#include <libmsc.h>

#ifndef THIS_HINSTANCE
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define THIS_HINSTANCE ((HINSTANCE) &__ImageBase)
#endif
