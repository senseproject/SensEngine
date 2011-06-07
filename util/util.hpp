#ifndef SENSE_UTIL_UTIL_HPP
#define SENSE_UTIL_UTIL_HPP

// Please don't put anything in here unless it really is a utility macro/inline.
// I'm serious.

// Make automatic exceptions easier to debug
#define STRINGIFICATION(X) #X
#define STRINGIFICATE(X) STRINGIFICATION(X)
#define FILE_LINE " @ " __FILE__ ":" STRINGIFICATE(__LINE__)

// Selectively disable warnings on GCC
#ifdef __GNUC__
#define GCC_DO_PRAGMA(prag) _Pragma(#prag)
#if __GNUC_MINOR__ >= 5
#define GCC_DIAGNOSTIC_PUSH  GCC_DO_PRAGMA(GCC diagnostic push)
#define GCC_DIAGNOSTIC_POP   GCC_DO_PRAGMA(GCC diagnostic pop)
#else
#define GCC_DIAGNOSTIC_PUSH
#define GCC_DIAGNOSTIC_POP
#endif
#define GCC_DISABLE_WARNING(warn) GCC_DO_PRAGMA(GCC diagnostic ignored warn)
#define MSC_DIAGNOSTIC_PUSH
#define MSC_DIAGNOSTIC_POP
#define MSC_DISABLE_WARNING(warn)
#endif

// Selectively disable warnings on MSC
#ifdef _MSC_VER
#define MSC_DIAGNOSTIC_PUSH __pragma warning(push)
#define MSC_DIAGNOSTIC_POP  __pragma warning(pop)
#define MSC_DISABLE_WARNING(warn) __pragma warning(disable: warn)
#define GCC_DIAGNOSTIC_PUSH
#define GCC_DIAGNOSTIC_POP
#define GCC_DISABLE_WARNING(warn)
#endif

#endif // SENSE_UTIL_UTIL_HPP
