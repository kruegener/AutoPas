/**
 * @file ExceptionHandler.h
 * @author seckler
 * @date 17.04.18
 */

#pragma once

#include <cstdlib>
#include <functional>
#include "Logger.h"

namespace autopas {
namespace utils {
/**
 * enum that defines the behavior of the expectionhandling
 * please check the enum values for a more detailed description
 */
enum ExceptionBehavior {
  ignore,                    /// ignore all exceptions
  throwException,            /// throw the exception
  printAbort,                /// print the exception and
  printCustomAbortFunction,  /// print the exception and call a custom abort
  /// function
};

/**
 * Defines and handles the throwing and printing of exceptions.
 * This class defines what should happen if an error occurs within AutoPas.
 * For a detailed list please check the enum ExceptionBehavior
 */
class ExceptionHandler {
 public:
  /**
   * Set the behavior of the handler
   * @param behavior the behavior
   */
  static void setBehavior(ExceptionBehavior behavior) {
    std::lock_guard<std::mutex> guard(exceptionMutex);
    _behavior = behavior;
  }

  /**
   * Handle an exception derived by std::exception.
   * If the behavior is set to throw and this function is called in a catch
   * clause, instead of using the passed exception the underlying error is
   * rethrown.
   * @param e the exception to be handled
   * @tparam Exception the type of the exception, needed as throw only uses the
   * static type of e
   */
  template <class Exception>
  static void exception(const Exception e) {
    std::lock_guard<std::mutex> guard(exceptionMutex);
    std::exception_ptr p;
    switch (_behavior) {
      case throwException:
        throw e;  // NOLINT
      default:
        nonThrowException(e);
    }
  }

  /**
   * Rethrows the current exception or prints it.
   * Depending on the set behavior the currently active exception is either
   * rethrown, printed or otherwise handled.
   * @note Use this only inside a catch clause.
   */
  static void rethrow();

  /**
   * Set a custom abort function
   * @param function the custom abort function
   */
  static void setCustomAbortFunction(std::function<void()> function) {
    std::lock_guard<std::mutex> guard(exceptionMutex);
    _customAbortFunction = std::move(function);
  }

 private:
  static std::mutex exceptionMutex;
  static ExceptionBehavior _behavior;
  static std::function<void()> _customAbortFunction;

  static void nonThrowException(const std::exception& e) {
    switch (_behavior) {
      case ignore:
        // do nothing
        break;
      case printAbort:
        AutoPasLogger->error("{}\naborting", e.what());
        AutoPasLogger->flush();
        std::abort();
      case printCustomAbortFunction:
        spdlog::get("AutoPasLog");
        AutoPasLogger->error("{}\nusing custom abort function", e.what());
        AutoPasLogger->flush();
        _customAbortFunction();
        break;
      default:
        break;
    }
  }

 public:
  /**
   * Default exception class for autopas exceptions.
   * @note normally generated using ExceptionHandler::exception("some string")
   */
  class AutoPasException : public std::exception {
   public:
    /**
     * constructor
     * @param description a descriptive string
     */
    explicit AutoPasException(std::string description) : _description(std::move(description)){};

    /**
     * returns the description
     * @return
     */
    const char* what() const noexcept override { return _description.c_str(); }

   private:
    std::string _description;
  };
};

/**
 * Handles an exception that is defined using the input string
 * @param e the string to describe the exception
 */
template <>
void ExceptionHandler::exception(const std::string e);  // NOLINT

/**
 * Handles an exception that is defined using the input string
 * @param e the string to describe the exception
 */
template <>
void ExceptionHandler::exception(const char* const e);  // NOLINT
}  // namespace utils
}  // namespace autopas