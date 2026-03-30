//
//  rb_logger.h
//
//  Created by Roger Boesch on Mar. 30, 2026.
//  "You can do whatever you like with it"
//

#ifndef rb_logger_h
#define rb_logger_h

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RB_LOG_LEVEL_DEBUG = 0,
    RB_LOG_LEVEL_INFO = 1,
    RB_LOG_LEVEL_ERROR = 2
} RBLogLevel;

// Set the minimum log level (messages below this level won't be printed)
void rb_log_set_level(RBLogLevel level);

// Get current log level
RBLogLevel rb_log_get_level(void);

// Logging functions with file/line info
void rb_log_debug_impl(const char *file, int line, const char *format, ...);
void rb_log_info_impl(const char *file, int line, const char *format, ...);
void rb_log_error_impl(const char *file, int line, const char *format, ...);

// Generic logging function with explicit level
void rb_log_impl(RBLogLevel level, const char *file, int line, const char *format, ...);

// Convenience macros that automatically capture file and line
#define rb_log_debug(...) rb_log_debug_impl(__FILE__, __LINE__, __VA_ARGS__)
#define rb_log_info(...) rb_log_info_impl(__FILE__, __LINE__, __VA_ARGS__)
#define rb_log_error(...) rb_log_error_impl(__FILE__, __LINE__, __VA_ARGS__)
#define rb_log(level, ...) rb_log_impl(level, __FILE__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* rb_logger_h */
