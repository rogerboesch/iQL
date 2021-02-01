# iQL
 
iQL is a Sinclair QL emulator based on uQlx with the focus on mobile devices.
The current implementation works fine for:
 
- iOS
- padOS
- macOS
- watchOS (For fun only and not yet finished)
- Android (coming soon)
 
If you search for alternatives running on Linux go with the original uQlx.
To make it work on sandboxed systems (as Android and iOS are) the following changes and enhancements are made:
 
- The QL filesystem is created at startup by the app itself and doesn't need any ini file
- All temporary files are handled by iQL and no longer by the OS (Unix) as in uQlx
- Filesystem was changed to support sandboxed folder names (changes at startup)
- Some Unix specific feature removed and/or refactored that are not supported on iOS or Android


The second area of changes is the entire user interface and user input:
 
- No use of any external library or system (as Xwindow, SDL, GLEW or similar)
- Rendering is purely done by using only the pixel buffer 
- Emulated user input through virtual keyboards
- Touch support (not yet finished)
- Platform enhancements to look "good" on iOS and macOS
 
It is currently very much work in progress and still needs much improvement.
 
 
## Build iQL
 
iQL has cmake support (creates a console app that runs on macos).
To create real macOS, iOS, padOS and Android citizens go with XCode resp. Android Studio at the moment.
 
    mkdir build
    cd build
    cmake -GXcode ..
 
 
## uQlx Emulator
 
While iQL uses still most of the underlying original files from uQlx, I have massively changed things which are no longer needed for non Xwindows systems.
 
Originally the code was written for gcc.
To create iOS apps it's necessary to use XCode which resulted in a lot of errors and warnings.
All of the errors are removed. The warnings are work in progress.
Therefore the original [documents](/QLemulator/docs/) are still good as a reference, but are in many parts no longer valid for iQL
 
 
## Copyright
 
For copyright of the base emulator files (original uQlx) see and **respect** the copyright of the author: [Copyright information](/QLemulator/docs/COPYRIGHT)
 
**All my changes are free to use and explicitly without any copyrights or limitations.**
 
*"Do whatever you like with it"* ... Although it would be nice to mention iQL ;)
 
 
*January 2021, Roger Boesch*
 
 