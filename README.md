# uHTTP - a very lightweight portable HTTP client

## Synopsis

uHTTP is an attempt to build a very small, portable, HTTP client for embedding
into larger applications.

The primary goals are:
 * Targetting C++11
 * Linux, OSX, Win32 (including x64) support.
 * Synchronous Operation Only
 * No dependancies other than libraries standard to the native toolchain.
 * HTTP/1.0 and/or HTTP/1.1 only.
 * Header decoding and body seperation in library
 * Trivially embeddable via CMake.

SSL/TLS support and web proxy support are intentionally omitted for simplicity 
at this time.

## Current Progress

Woefully incomplete at this time.

 * Basic socket code for win32 complete.  posix to follow shortly.
 * URI parsing implemented but untested.

## License

When complete, uhttp will be available under a the 3 clause BSD license. 
(https://opensource.org/licenses/BSD-3-Clause)

## Contributions

Please feel free to contribute, but all submissions must adhere to the goals 
as stated above.

Other key notes about the library design:

 * Efforts to conform developer use of the library to be standards compliant is
   eschewed in favor of simplicity.  It is expected that developers will ensure
   their use of the client is compliant with any necessary standards.

 * The third party library usage rule is a strict rule - the whole reason I
   started writing uhttp is to evict curl out of a crossplatform project
   because it was too hard to get to reliably build over all platforms from
   cmake, and has far more features than the project needs.

 * Code must remain scrutable - clever solutions to problems are not clever if
   the result is only understood by a few people and becomes difficult to 
   maintain as a result.

 * Wherever feasible, the client must use platform services or use platform
   standard configuration.

