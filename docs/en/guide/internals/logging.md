---
description: logging in nidium
---

nidium provides various facilities for printing informations on screen (or in debug console).

## Controlling verbosity

The verbosity level range from `0` (only print errors) to `3` (debug)
debug build (`--debug`) set the default verbosity to `3` and release build set the default to `0`.

You can dynamically change the level using the `NIDIUM_VERBOSITY` environment variable. (`$ NIDIUM_VERBOSITY=3 ./nidium`).

## printing messages

Various macros are globally available (don't require an include file) in order to print informations.

```cpp
#define NDM_LOG_ERROR 0
#define NDM_LOG_WARN  1
#define NDM_LOG_INFO  2
#define NDM_LOG_DEBUG 3

#define ndm_logf(level, tag, format, ...) \
    APE_logf(level, tag, "(%s:%d) " format, __FILENAME__, __LINE__, ##__VA_ARGS__)

#define ndm_log(level, tag, data) \
    APE_logf(level, tag, "(%s:%d) %s", __FILENAME__, __LINE__, data)

#define ndm_printf(format, ...) \
    APE_logf(APE_LOG_INFO, nullptr, "(%s:%d) " format, __FILENAME__, __LINE__, ##__VA_ARGS__)

#define ndm_print(data) \
    APE_logf(APE_LOG_INFO, nullptr, "(%s:%d) %s", __FILENAME__, __LINE__, data)
```

## Examples

```cpp
// print 'Hello 42' with verbosity of level DEBUG (3).
ndm_logf(NDM_LOG_DEBUG, "a_tag", "Hello %d", 42);
// do not use the format version for uncontrolled and unformatted data.
ndm_log(NDM_LOG_DEBUG, "a_tag", some_message);

// Shortcut for level INFO without any tag
ndm_printf("Hello %d", 42);
ndm_print(some_message);
```

The `tag` (optional) is prepended to the printed message and is used for easier filtering.

Please note that a line feed (`\n`) is automatically appended.
