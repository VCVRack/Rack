This file contains instructions for integrating the STK in Xcode projects and solutions to common integration issues. 

## Setup

### If you have [Cocoapods](http://cocoapods.org/)

1. Add `pod 'STK', '~> 4.5'` to your Podfile.

1. Run `pod install`

### If you don't have Cocoapods

1. Clone or [download][download_link] the STK into your project's directory.

1. Open the **STK for iOS** folder, and drag and drop **STK.xcodeproj** into your Xcode project.

1. Open your project's settings, open the *Build Phases* tab. In the *Link Binary with Libraries* section, add **libSTK.a**. 
![][linking_libSTK_screenshot]

1. In your project's settings, open the *Build Settings* tab. In the *Search Paths* section, double click on the field to the right of *Header Search Paths*, and add the path to the STK's **include** directory relative to your Xcode project's directory.
![][header_search_paths_screenshot]


## Usage

1. Import the STK classes in the source files you require. 
  * E.g. `#import "SineWave.h"`

1. Change the extension of Objective-C files that import STK files to **.mm**. 
  * E.g. **ViewController.m** —> **ViewController.mm**

You can also look at the [iOS Demo project](..projects/demo/iOS%20Demo) for a sample usage. 


## Troubleshooting

### 'FileName.h' file not found

If you get this error when `#import`ing an STK header, you have added the wrong header search path for the STK in your project's settings (see Step 4 in Setup)

The STK's header search path you need to add is the path to the STK's **include** directory relative to your project's directory (as if you were `cd`ing into it). For example, it is `stk/include/` if the stk directory is inside your project's directory, but it is `../stk/include/` if both share the same directory. 

If this problem doesn't go away:

1. Delete **STK.xcodeproj** from your Xcode project
1. Move the STK directory within your project's directory. 
1. Follow step 1 from **Setup**, add `stk/include` to the *Header Search Paths*.

If that doesn't solve it:
Install Cocoapods and use it to install the STK. It takes one minute and will make your life easier. Visit the [Cocoapods website](http://cocoapods.org/) for installation instructions. 

### FileRead::open: could not open or find file (../../rawwaves/filename.raw)!

If you use a class that makes use of raw waves (such as `Mandolin`, `Wurley`, or `Rhodey`) you need to make sure that the STK's raw wave files are copied into your bundle and that the STK knows where they are. You'll know you need to if you get this runtime error:
`FileRead::open: could not open or find file (../../rawwaves/filename.raw)!`

#### If you're using Cocoapods

Add this code before using a class that needs the raw waves: 
```objective-c
stk::Stk::setRawwavePath([[[NSBundle mainBundle] pathForResource:@"rawwaves" ofType:@"bundle"] UTF8String]);
```

#### If you're not using Cocoapods

1. Open your project's settings, open the *Build Phases* tab. 
1. In the *Copy Bundle Resources*, drag and drop **rawwaves.bundle** (it's located in **STK.xcodeproj**'s **Helpers** folder). 
1. Then add this code before using a class that needs the raw waves: 

```objective-c
NSBundle *rawwaveBundle = [NSBundle bundleWithURL:[[NSBundle mainBundle] URLForResource:@"rawwaves" withExtension:@"bundle"]];
stk::Stk::setRawwavePath([[rawwaveBundle resourcePath] UTF8String]);
```


### rawwaves.bundle: No such file or directory

This means that **rawwaves.bundle** hasn't been copied to the build folder, so you'll need to do it manually:

Select the rawwaves scheme:

![][rawwaves_scheme_screenshot]
  
Build it (⌘+B)  then build your project's main scheme. 

### Apple Mach-O Linker Error

This means that **STKLib.a** isn't being linked to your binary. Follow step 2 above in [Setup](#setup). 


[download_link]: https://github.com/thestk/stk/archive/master.zip
[linking_libSTK_screenshot]: http://i.imgur.com/cLbGrtq.png
[header_search_paths_screenshot]: http://i.imgur.com/iBTC06h.png
[rawwaves_scheme_screenshot]: http://i.imgur.com/PKd7epf.png
