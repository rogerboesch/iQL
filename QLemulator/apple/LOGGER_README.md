# Logger Implementation Summary

## Overview
A new logging system has been implemented to replace direct `printf`, `fprintf`, and `perror` calls throughout the codebase. This provides consistent logging with timestamps and different severity levels.

## New Files Created

### rb_logger.h
Header file defining the logger interface with:
- **Log Levels**: `RB_LOG_LEVEL_DEBUG`, `RB_LOG_LEVEL_INFO`, `RB_LOG_LEVEL_ERROR`
- **Functions**:
  - `rb_log_set_level(RBLogLevel level)` - Set minimum log level
  - `rb_log_get_level()` - Get current log level
  - `rb_log_debug(const char *format, ...)` - Log debug messages
  - `rb_log_info(const char *format, ...)` - Log info messages
  - `rb_log_error(const char *format, ...)` - Log error messages
  - `rb_log(RBLogLevel level, const char *format, ...)` - Generic logging

### rb_logger.m
Implementation file providing:
- Timestamp formatting (yyyy-MM-dd HH:mm:ss.SSS)
- Automatic newline handling
- Different output streams (stdout for debug/info, stderr for errors)
- Configurable log level filtering

## Files Modified

### base_pty.c
- Added `#include "rb_logger.h"`
- Replaced 4 print statements:
  - `printf` for exec error → `rb_log_error`
  - `fprintf(stderr)` for pty info → `rb_log_info`
  - `printf` for file descriptor debug → `rb_log_debug`

### base_uxfile.c
- Added `#include "rb_logger.h"`
- Replaced 7 print statements:
  - File reopening info → `rb_log_info`
  - XTcc debug info → `rb_log_debug`
  - `perror` for stat errors → `rb_log_error`
  - Directory and mount point errors → `rb_log_error`

### QL_emulator.c
- Added `#include "rb_logger.h"`
- Replaced 21 print statements:
  - Signal processing debug → `rb_log_debug`
  - Fatal errors → `rb_log_error`
  - ROM loading warnings → `rb_log_info` / `rb_log_error`
  - Debug information → `rb_log_debug`
  - Memory allocation errors → `rb_log_error`

## Usage Examples

```c
// Debug messages (only shown when log level is DEBUG)
rb_log_debug("Processing frame %d at position %d", frame_num, pos);

// Info messages (shown when log level is DEBUG or INFO)
rb_log_info("System initialized successfully");
rb_log_info("Loaded ROM from %s", rom_path);

// Error messages (always shown unless log level is explicitly disabled)
rb_log_error("Failed to open file: %s", filename);
rb_log_error("Memory allocation failed for %d bytes", size);

// Setting log level (typically done at startup)
rb_log_set_level(RB_LOG_LEVEL_INFO);  // Only show INFO and ERROR
rb_log_set_level(RB_LOG_LEVEL_DEBUG); // Show all messages
```

## Output Format

The logger includes time, icon, file location, and message:

```
[13:49:32.249] ℹ️ (base_cfg.c:395) Using Config: ql.ini
[13:49:32.350] ℹ️ (QL_config.c:235) using Minerva ROM
[13:49:32.451] ℹ️ (QL_config.c:213) Patching Boot Device mdv1 at 0x83ca
[13:49:32.562] 🔧 (base_dummies.c:17) warning: calling dummy function: InstallSerial
[13:49:32.673] ❌ (base_cfg.c:393) Failed to open file: config.dat
```

### Format: `[HH:MM:SS.mmm] icon (filename:line) message`

- **Time** - Hours:minutes:seconds.milliseconds
- **Icon** - Emoji showing severity level
- **Location** - Filename and line number in parentheses
- **Message** - The actual log message

### Severity Icons

- **🔧** - Debug/diagnostic information for developers
- **ℹ️** - General informational messages
- **❌** - Error messages that require attention

The file location makes it easy to find exactly where each log message originated!

## Benefits

1. **Consistent Format**: All log messages have timestamps and severity levels
2. **Filtering**: Can adjust verbosity at runtime by changing log level
3. **Proper Streams**: Errors go to stderr, other messages to stdout
4. **Better Debugging**: Timestamps help track timing issues
5. **Maintainability**: Centralized logging makes it easier to add features (e.g., file logging)
6. **Cross-platform**: Works on all Apple platforms (iOS, macOS, etc.)

## Integration

To use the logger in any C/Objective-C file:

```c
#include "rb_logger.h"

// Then use rb_log_debug(), rb_log_info(), rb_log_error()
```

## Future Enhancements

Possible improvements:
- File logging support
- Log rotation
- Colored output for terminal
- Performance metrics
- Thread-safe logging for concurrent operations
