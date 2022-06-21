#pragma once

#ifndef WIN32_H
#define WIN32_H

#if !defined(_WIN32) && !defined(_WIN64)
#	error We're not on Windows...
#endif

#ifndef WIN32_FAT_AND_FRIENDLY
#	define WIN32_LEAN_AND_MEAN
#endif

// Atom Manager routines
#ifndef WIN32_ATOM
#	define NOATOM
#endif

// Clipboard routines
#ifndef WIN32_CLIPBOARD
#	define NOCLIPBOARD
#endif

// Screen colors
#ifndef WIN32_COLOR
#	define NOCOLOR
#endif

// COMM driver routines
#ifndef WIN32_COMM
#	define NOCOMM
#endif

// Control and Dialog routines
#ifndef WIN32_CTLMGR
#	define NOCTLMGR
#endif

// DeferWindowPos routines
#ifndef WIN32_DEFERWINDOWPOS
#	define NODEFERWINDOWPOS
#endif

// DrawText() and DT_*
#ifndef WIN32_DRAWTEXT
#	define NODRAWTEXT
#endif

// All GDI defines and routines
#ifndef WIN32_GDI
#	define NOGDI
#endif

// CC_* , LC_*, PC_*, CP_*, TC_*, RC_
#ifndef WIN32_GDICAPMASKS
#	define NOGDICAPMASKS
#endif

// Help engine interface.
#ifndef WIN32_HELP
#	define NOHELP
#endif

// IDI_*
#ifndef WIN32_ICONS
#	define NOICONS
#endif

// Kanji support stuff.
#ifndef WIN32_KANJI
#	define NOKANJI
#endif

// All KERNEL defines and routines
#ifndef WIN32_KERNEL
#	define NOKERNEL
#endif

// MK_*
#ifndef WIN32_KEYSTATES
#	define NOKEYSTATES
#endif

// MB_* and MessageBox()
#ifndef WIN32_MB
#	define NOMB
#endif

// Modem Configuration Extensions
#ifndef WIN32_MCX
#	define NOMCX
#endif

// GMEM_* , LMEM_*, GHND, LHND, associated routines
#ifndef WIN32_MEMMGR
#	define NOMEMMGR
#endif

// MF_*
#ifndef WIN32_MENUS
#	define NOMENUS
#endif

// typedef METAFILEPICT
#ifndef WIN32_METAFILE
#	define NOMETAFILE
#endif

// typedef MSG and associated routines
#ifndef WIN32_MSG
#	define NOMSG
#endif

// All NLS defines and routines
#ifndef WIN32_NLS
#	define NONLS
#endif

// OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#ifndef WIN32_OPENFILE
#	define NOOPENFILE
#endif

// Profiler interface.
#ifndef WIN32_PROFILER
#	define NOPROFILER
#endif

// Binary and Tertiary raster ops
#ifndef WIN32_RASTEROPS
#	define NORASTEROPS
#endif

// SB_* and scrolling routines
#ifndef WIN32_SCROLL
#	define NOSCROLL
#endif

// All Service Controller routines, SERVICE_ equates, etc.
#ifndef WIN32_SERVICE
#	define NOSERVICE
#endif

// SW_*
#ifndef WIN32_SHOWWINDOW
#	define NOSHOWWINDOW
#endif

// Sound driver routines
#ifndef WIN32_SOUND
#	define NOSOUND
#endif

// SC_*
#ifndef WIN32_SYSCOMMANDS
#	define NOSYSCOMMANDS
#endif

// SM_*
#ifndef WIN32_SYSMETRICS
#	define NOSYSMETRICS
#endif

// typedef TEXTMETRIC and associated routines
#ifndef WIN32_TEXTMETRIC
#	define NOTEXTMETRIC
#endif

// All USER defines and routines
#ifndef WIN32_USER
#	define NOUSER
#endif

// VK_*
#ifndef WIN32_VIRTUALKEYCODES
#	define NOVIRTUALKEYCODES
#endif

// SetWindowsHook and WH_*
#ifndef WIN32_WH
#	define NOWH
#endif

// WM_* , EM_*, LB_*, CB_*
#ifndef WIN32_WINMESSAGES
#	define NOWINMESSAGES
#endif

// GWL_* , GCL_*, associated routines
#ifndef WIN32_WINOFFSETS
#	define NOWINOFFSETS
#endif

// WS_* , CS_*, ES_*, LBS_*, SBS_*, CBS_*
#ifndef WIN32_WINSTYLES
#	define NOWINSTYLES
#endif

// OEM Resource values
#ifdef WIN32_OEMRESOURCE
#	define OEMRESOURCE
#endif

#define NOMINMAX

#include <Windows.h>

#undef NOMINMAX

#ifdef WIN32_LEAN_AND_MEAN
#	undef WIN32_LEAN_AND_MEAN
#endif

#ifndef WIN32_ATOM
#	undef NOATOM
#endif

#ifndef WIN32_CLIPBOARD
#	undef NOCLIPBOARD
#endif

#ifndef WIN32_COLOR
#	undef NOCOLOR
#endif

#ifndef WIN32_COMM
#	undef NOCOMM
#endif

#ifndef WIN32_CTLMGR
#	undef NOCTLMGR
#endif

#ifndef WIN32_DEFERWINDOWPOS
#	undef NODEFERWINDOWPOS
#endif

#ifndef WIN32_DRAWTEXT
#	undef NODRAWTEXT
#endif

#ifndef WIN32_GDI
#	undef NOGDI
#endif

#ifndef WIN32_GDICAPMASKS
#	undef NOGDICAPMASKS
#endif

#ifndef WIN32_HELP
#	undef NOHELP
#endif

#ifndef WIN32_ICONS
#	undef NOICONS
#endif

#ifndef WIN32_KANJI
#	undef NOKANJI
#endif

#ifndef WIN32_KERNEL
#	undef NOKERNEL
#endif

#ifndef WIN32_KEYSTATES
#	undef NOKEYSTATES
#endif

#ifndef WIN32_MB
#	undef NOMB
#endif

#ifndef WIN32_MCX
#	undef NOMCX
#endif

#ifndef WIN32_MEMMGR
#	undef NOMEMMGR
#endif

#ifndef WIN32_MENUS
#	undef NOMENUS
#endif

#ifndef WIN32_METAFILE
#	undef NOMETAFILE
#endif

#ifndef WIN32_MSG
#	undef NOMSG
#endif

#ifndef WIN32_NLS
#	undef NONLS
#endif

#ifndef WIN32_OPENFILE
#	undef NOOPENFILE
#endif

#ifndef WIN32_PROFILER
#	undef NOPROFILER
#endif

#ifndef WIN32_RASTEROPS
#	undef NORASTEROPS
#endif

#ifndef WIN32_SCROLL
#	undef NOSCROLL
#endif

#ifndef WIN32_SERVICE
#	undef NOSERVICE
#endif

#ifndef WIN32_SHOWWINDOW
#	undef NOSHOWWINDOW
#endif

#ifndef WIN32_SOUND
#	undef NOSOUND
#endif

#ifndef WIN32_SYSCOMMANDS
#	undef NOSYSCOMMANDS
#endif

#ifndef WIN32_SYSMETRICS
#	undef NOSYSMETRICS
#endif

#ifndef WIN32_TEXTMETRIC
#	undef NOTEXTMETRIC
#endif

#ifndef WIN32_USER
#	undef NOUSER
#endif

#ifndef WIN32_VIRTUALKEYCODES
#	undef NOVIRTUALKEYCODES
#endif

#ifndef WIN32_WH
#	undef NOWH
#endif

#ifndef WIN32_WINMESSAGES
#	undef NOWINMESSAGES
#endif

#ifndef WIN32_WINOFFSETS
#	undef NOWINOFFSETS
#endif

#ifndef WIN32_WINSTYLES
#	undef NOWINSTYLES
#endif

#ifdef WIN32_OEMRESOURCE
#	undef OEMRESOURCE
#endif

#endif // WIN32_H
