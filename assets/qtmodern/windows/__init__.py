from .._utils import PLATFORM, PLATFORM_MACOS, PLATFORM_WINDOWS

if PLATFORM in PLATFORM_WINDOWS:
    from .win32 import ModernWindow, ModernDialog
elif PLATFORM in PLATFORM_MACOS:
    from .darwin import ModernWindow
else:
    from .dummy import ModernWindow
