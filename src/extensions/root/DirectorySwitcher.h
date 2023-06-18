// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <TDirectory.h>

/**
 * @class DirectoryGuard
 *
 * @brief A Resource Ownership Idiom (ROI) guard class for managing TDirectory pointers from the CERN ROOT library.
 *
 * The DirectoryGuard class encapsulates the handling of the global `gDirectory` pointer,
 * which changes every time `TDirectory::cd()` is called.
 * An instance of DirectoryGuard captures a `TDirectory*` on construction and
 * calls `directory->cd()`, changing the context to the given directory.
 * The previously active directory (value of `gDirectory` before the switch) is saved.
 * When the DirectoryGuard object is destructed, the context switches back to the original directory.
 *
 * This class follows the Rule of Five, which means that it provides a destructor,
 * a move constructor, a move assignment operator, and explicitly deletes the copy constructor and
 * copy assignment operator to properly manage the resource.
 *
 * @note The DirectoryGuard class is not thread-safe. If you use it in a multi-threaded context,
 * you must ensure that it's used in a thread-safe manner.
 *
 * Example usage:
 * \code{.cpp}
 * {
 *     DirectoryGuard guard(directory);
 *     // ... code that works with directory
 * } // guard object goes out of scope here and directory is switched back
 * \endcode
 *
 * @note The copy constructor and copy assignment operator are deleted to prevent copying of the guard object.
 * The move constructor and move assignment operator are provided to allow for transferring ownership of
 * the `TDirectory` pointer.
 *
 * @warning The TDirectory object passed to the guard must outlive the guard itself or
 * be nullptr at the time of the guard's destruction. Otherwise, undefined behavior may occur.
 */
namespace extensions::root {
    class SwitchTDirectory {
    private:
        TDirectory *m_dir;
        TDirectory *m_prev_dir;

    public:
        // Constructor
        SwitchTDirectory(TDirectory *dir) : m_dir(dir) {
            if (m_dir) {
                m_prev_dir = gDirectory;
                m_dir->cd();
            }
        }

        // Destructor
        ~SwitchTDirectory() {
            if (m_prev_dir) {
                m_prev_dir->cd();
            }
        }

        // Delete the copy constructor
        SwitchTDirectory(const SwitchTDirectory &) = delete;

        // Delete the copy assignment operator
        SwitchTDirectory &operator=(const SwitchTDirectory &) = delete;

        // Move constructor
        SwitchTDirectory(SwitchTDirectory &&other) noexcept: m_dir(other.m_dir), m_prev_dir(other.m_prev_dir) {
            other.m_dir = nullptr;
            other.m_prev_dir = nullptr;
        }

        // Move assignment operator
        SwitchTDirectory &operator=(SwitchTDirectory &&other) noexcept {
            if (this != &other) {
                if (m_prev_dir) {
                    m_prev_dir->cd();
                }

                m_dir = other.m_dir;
                m_prev_dir = other.m_prev_dir;
                other.m_dir = nullptr;
                other.m_prev_dir = nullptr;
            }
            return *this;
        }
    };
}