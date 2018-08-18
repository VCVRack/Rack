##iOS Demo Xcode project

This project briefly shows how to manually integrate the STK static library into an Xcode project. See the **README** file in the STK's `iOS` directory for precise instructions.

Currently, this project does not output sound, it only shows how to generate audio samples from the STK classes within an iOS project, and how to control STK objects via UI controls.

Note the following:

 * ViewController needs to be renamed with the **.mm** extension as it's importing STK files, which are C++.
 * The header search paths in the *Build Settings* of **iOS Demo.xcodeproj** point to `../../include/` because the STK's `include` directory is two directories up relative to it.