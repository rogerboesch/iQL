//
//  rb_ghost.h
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import "TargetConditionals.h"

#if TARGET_OS_IOS
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

@interface RBGhost : NSObject

+ (void)typeIn:(NSString *)command;
+ (void)loadPasteFile;
+ (void)savePasteFile;

@end
