//
//  AppDelegate.m
//  CellMotus
//
//  Created by user on 23.12.14.
//  Copyright (c) 2014 TimeFlask. All rights reserved.
//

#import "AppDelegate.h"
#include "sen.h"


#define KEEP(x) unused.p[0]=x

extern const void* signal_enterBackground;
extern const void* signal_enterForeground;

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
  _logfi("===================== didFinishLaunchingWithOptions ======================");
  return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
  _logfi("=====================  (void)applicationWillResignActive =================");
//  unused.p[5] = sen_scheduler;
  //sen_platform_dpi(); sen_exit();
  if (signal_enterBackground) {

    sen_signal_emit( signal_enterBackground, NULL );
  }
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
  _logfi("=====================  applicationDidEnterBackground     =================");

}

- (void)applicationWillEnterForeground:(UIApplication *)application {
  _logfi("=====================  applicationWillEnterForeground    =================");
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
  _logfi("=====================  applicationDidBecomeActive        =================");
  if (signal_enterForeground) {
    sen_signal_emit( signal_enterForeground, NULL );
  }
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
  _logfi("=====================  applicationWillTerminate          =================");
}

@end
