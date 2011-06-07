#ifndef SENSE_UTIL_UTIL_HPP
#define SENSE_UTIL_UTIL_HPP

// Please don't put anything in here unless it really is a utility macro/inline.
// I'm serious.

#define STRINGIFICATION(X) #X
#define STRINGIFICATE(X) STRINGIFICATION(X)
#define FILE_LINE " @ " __FILE__ ":" STRINGIFICATE(__LINE__)

#endif // SENSE_UTIL_UTIL_HPP
