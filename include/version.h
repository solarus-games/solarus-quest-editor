/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus Quest Editor is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus Quest Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <solarus/Common.h>

/**
 * @brief Solarus Quest Editor major version.
 *
 * Must match the Solarus library major version.
 */
#define SOLARUSEDITOR_MAJOR_VERSION 1

/**
 * @brief Solarus Quest Editor minor version.
 *
 * Must match the Solarus library minor version.
 */
#define SOLARUSEDITOR_MINOR_VERSION 5

/**
 * @brief Solarus Quest Editor patch version.
 *
 * Patch versions are only bug fixes.
 * No need to match the Solarus library patch version.
 */
#define SOLARUSEDITOR_PATCH_VERSION 2

/**
 * @brief The Solarus Quest Editor version as a string.
 *
 * This string has the form "x.y.z" where x is the major version, y is the
 * minor version and z is the patch version.
 */
#define SOLARUSEDITOR_VERSION \
    SOLARUS_STRINGIFY(SOLARUSEDITOR_MAJOR_VERSION) "." \
    SOLARUS_STRINGIFY(SOLARUSEDITOR_MINOR_VERSION) "." \
    SOLARUS_STRINGIFY(SOLARUSEDITOR_PATCH_VERSION)

/**
 * @brief The Solarus Quest Editor version as a string, without patch number.
 *
 * This string has the form "x.y" where x is the major version and y is the
 * minor version.
 */
#define SOLARUSEDITOR_VERSION_WITHOUT_PATCH \
    SOLARUS_STRINGIFY(SOLARUSEDITOR_MAJOR_VERSION) "." \
    SOLARUS_STRINGIFY(SOLARUSEDITOR_MINOR_VERSION)

static_assert(SOLARUSEDITOR_MAJOR_VERSION == SOLARUS_MAJOR_VERSION && SOLARUSEDITOR_MINOR_VERSION == SOLARUS_MINOR_VERSION,
              "The quest editor version is " SOLARUSEDITOR_VERSION_WITHOUT_PATCH " but the Solarus library version is " SOLARUS_VERSION_WITHOUT_PATCH);
