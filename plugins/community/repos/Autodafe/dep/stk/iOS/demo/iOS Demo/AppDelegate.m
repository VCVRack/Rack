//
//  AppDelegate.m
//  iOS Demo
//
//  Created by Ariel Elkin on 03/03/2014.
//

#import "AppDelegate.h"
#import "ViewController.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    
    ViewController *vc = [[ViewController alloc] initWithNibName:nil bundle:nil];
    [self.window setRootViewController:vc];
    
    [self.window makeKeyAndVisible];
    
    return YES;
}

@end
