//
//  AppDelegate.m
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import "AppDelegate.h"
#import "rb_platform.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)copyFile:(NSString*)filename toFolder:(NSString*)folderName {
    NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
    NSString* fromPath = [NSString stringWithFormat:@"%@/%@", resourcePath, filename];

    NSString* documentPath = [RBPlatform getDocumentPath];
    NSString* toPath = [NSString stringWithFormat:@"%@/%@/%@", documentPath, folderName, filename];

    if (folderName.length == 0) {
        toPath = [NSString stringWithFormat:@"%@/%@", documentPath, filename];
    }
    
    [RBPlatform copyFile:fromPath to:toPath overwrite:YES];
}

- (void)createQLSystem {
    [RBPlatform createFolder:@"roms"];
    [RBPlatform createFolder:@"mdv1"];
    [RBPlatform createFolder:@"mdv2"];
    [RBPlatform createFolder:@"flp1"];
    [RBPlatform createFolder:@"temp"];
    
    [self copyFile:@"ql.ini" toFolder:@""];
    [self copyFile:@"min198.rom" toFolder:@"roms"];
    [self copyFile:@"ql_jm.rom" toFolder:@"roms"];
    [self copyFile:@"ql_js.rom" toFolder:@"roms"];
    [self copyFile:@"tk232.rom" toFolder:@"roms"];
    [self copyFile:@"BOOT" toFolder:@"mdv1"];
    [self copyFile:@"invaders_bas" toFolder:@"mdv2"];
    [self copyFile:@"snake_bas" toFolder:@"mdv2"];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    [self createQLSystem];
    
    return YES;
}

#pragma mark - UISceneSession lifecycle

- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options {
    return [[UISceneConfiguration alloc] initWithName:@"Default Configuration" sessionRole:connectingSceneSession.role];
}

- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions {
}

@end
