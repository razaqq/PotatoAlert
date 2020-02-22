from .._utils import PLATFORM, PLATFORM_MACOS, PLATFORM_WINDOWS

if PLATFORM in PLATFORM_WINDOWS:
    from .win32 import BorderlessWindow
elif PLATFORM in PLATFORM_MACOS:
    from .darwin import BorderlessWindow
else:
    from .dummy import BorderlessWindow
