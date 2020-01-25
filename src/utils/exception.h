#ifndef COR_UTILS_EXCEPTION_H
#define COR_UTILS_EXCEPTION_H

#include <exception>
#include <string>

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief basic exception class, with a THROW and GUARD define
 */
class Exception : public std::exception {
public:
    /// contsructor
    template <class... T>
    explicit Exception(const char* func, T&&... t)
        : mFunction{func},
          mMessage{"cor::Exception at: " + mFunction + " : " + std::forward<T>(t)...} {}

    /// getter for exception's message
    virtual const char* what() const noexcept override { return mMessage.c_str(); }

private:
    /// function calling the error
    std::string mFunction;

    /// error message for exception
    std::string mMessage;
};

} // namespace cor

#define THROW_EXCEPTION(...) throw cor::Exception(__PRETTY_FUNCTION__, __VA_ARGS__)

#define GUARD_EXCEPTION(cond, ...)    \
    if (!(cond)) {                    \
        THROW_EXCEPTION(__VA_ARGS__); \
    }

#endif // COR_UTILS_EXCEPTION_H
