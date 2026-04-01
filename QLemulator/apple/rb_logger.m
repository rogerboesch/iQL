//
//  rb_logger.m
//
//  Created by Roger Boesch on Mar. 30, 2026.
//  "You can do whatever you like with it"
//

#include "rb_logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

static RBLogLevel s_current_log_level = RB_LOG_LEVEL_INFO;

void rb_log_set_level(RBLogLevel level) {
    s_current_log_level = level;
}

RBLogLevel rb_log_get_level(void) {
    return s_current_log_level;
}

static const char* rb_log_level_icon(RBLogLevel level) {
    switch (level) {
        case RB_LOG_LEVEL_DEBUG: return "🔧";
        case RB_LOG_LEVEL_INFO:  return "ℹ️";
        case RB_LOG_LEVEL_ERROR: return "❌";
        default: return "❓";
    }
}

static void rb_log_timestamp(char *buffer, size_t size) {
    struct timeval tv;
    struct tm *tm_info;
    
    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);
    
    snprintf(buffer, size, "%02d:%02d:%02d.%03d",
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec,
             (int)(tv.tv_usec / 1000));
}

static const char* extract_filename(const char *path) {
    const char *filename = strrchr(path, '/');
    if (filename) {
        return filename + 1;
    }
    return path;
}

void rb_log_impl(RBLogLevel level, const char *file, int line, const char *format, ...) {
    if (level < s_current_log_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    char timestamp[32];
    rb_log_timestamp(timestamp, sizeof(timestamp));
    
    // Print prefix: [HH:MM:SS.mmm] icon (file:line)
    printf("[%s] %s (%s:%d) ", timestamp, rb_log_level_icon(level), extract_filename(file), line);
    
    // Print the actual message
    vprintf(format, args);
    
    // Ensure newline at end if not present
    size_t len = strlen(format);
    if (len == 0 || format[len - 1] != '\n') {
        printf("\n");
    }
    
    va_end(args);
    fflush(stdout);
}

void rb_log_debug_impl(const char *file, int line, const char *format, ...) {
    if (RB_LOG_LEVEL_DEBUG < s_current_log_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    char timestamp[32];
    rb_log_timestamp(timestamp, sizeof(timestamp));
    
    printf("[%s] 🔧 (%s:%d) ", timestamp, extract_filename(file), line);
    vprintf(format, args);
    
    size_t len = strlen(format);
    if (len == 0 || format[len - 1] != '\n') {
        printf("\n");
    }
    
    va_end(args);
    fflush(stdout);
}

void rb_log_info_impl(const char *file, int line, const char *format, ...) {
    if (RB_LOG_LEVEL_INFO < s_current_log_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    char timestamp[32];
    rb_log_timestamp(timestamp, sizeof(timestamp));
    
    printf("[%s] ℹ️ (%s:%d) ", timestamp, extract_filename(file), line);
    vprintf(format, args);
    
    size_t len = strlen(format);
    if (len == 0 || format[len - 1] != '\n') {
        printf("\n");
    }
    
    va_end(args);
    fflush(stdout);
}

void rb_log_error_impl(const char *file, int line, const char *format, ...) {
    if (RB_LOG_LEVEL_ERROR < s_current_log_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    char timestamp[32];
    rb_log_timestamp(timestamp, sizeof(timestamp));
    
    fprintf(stderr, "❌ [%s, %s:%d] ", timestamp, extract_filename(file), line);
    vfprintf(stderr, format, args);
    
    size_t len = strlen(format);
    if (len == 0 || format[len - 1] != '\n') {
        fprintf(stderr, "\n");
    }
    
    va_end(args);
    fflush(stderr);
}
void rb_log_dbginfo(const char *file, int line, const char *format, ...) {
    if (RB_LOG_LEVEL_DEBUG < s_current_log_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    char timestamp[32];
    rb_log_timestamp(timestamp, sizeof(timestamp));
    
    // Log the message
    printf("🔧 [%s, %s:%d] ", timestamp, extract_filename(file), line);
    vprintf(format, args);
    
    size_t len = strlen(format);
    if (len == 0 || format[len - 1] != '\n') {
        printf("\n");
    }
    
    va_end(args);
    
    // Also call the register dump
    extern void rb_log_register_dump(void);
    rb_log_register_dump();
    
    fflush(stdout);
}

