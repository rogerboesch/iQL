# iQL
 
iQL is a Sinclair QL emulator based on uQlx with the focus on mobile devices.
It will be used as part of the 
The current implementation works fine for:
 
- iOS
- padOS
- macOS
- watchOS (For fun only and not yet finished)
- *Android (coming soon)*
 
If you search for alternatives running on Linux or Windows go with the original uQlx emulator.

To make iQL work on sandboxed systems (as Android and iOS are) the following changes and enhancements are made:
 
- The QL filesystem is created at startup by the app itself and doesn't need any ini file
- All temporary files are handled by iQL and no longer by the OS (Unix) as in uQlx
- Filesystem was changed to support sandboxed folder names (changes at startup)
- Some Unix specific feature removed and/or refactored that are not supported on iOS or Android


The second area of changes made impact the entire user interface and user input:
 
- No use of any external library or system (as Xwindow, SDL, GLEW or similar)
- Rendering is purely done by using the pixel buffer only 
- Simulate user input through virtual keyboards
- Platform enhancements to look "good" on iOS and macOS
- *Touch support (not yet finished)*
 
It is currently very much work in progress and still needs much improvement.
 
 ## TODO
 
 - Copy/Paste code (to simplfy exchange of SuperBasic ocde)
 - iCloud/Google Drive integration (To add disc images and files to iQL)
 
 
## Build iQL
 
To create real macOS, iOS, padOS and Android citizens go with XCode (use iQL.xcodeproj) resp. Android Studio at the moment.
 
 
## uQlx Emulator
 
While iQL uses still most of the underlying original files from uQlx, I have massively changed things which are no longer needed for non Xwindows systems.
 
Originally the code was written for gcc.
To create iOS apps it's necessary to use XCode which resulted in a lot of errors and warnings.
All of the errors are removed. The warnings are work in progress.
Therefore the original [documents](/QLemulator/docs/) are still good as a reference, but are in many parts no longer valid for iQL
 
 
## Copyright
 
For copyright informations of the base emulator files (original uQlx) read and **respect** the [copyright](/QLemulator/docs/COPYRIGHT) of the author!


**All my changes are free to use and explicitly without any copyrights or limitations.**
 
*"Do whatever you like with it"* ... Although it would be nice to mention iQL ;)
 
 
*January 2021, Roger Boesch*
 
 
