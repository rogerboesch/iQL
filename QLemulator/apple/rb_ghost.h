//
//  rb_ghost.h
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import <Foundation/Foundation.h>

@interface RBGhost : NSObject

+ (void)typeIn:(NSString *)command;
+ (void)loadPasteFile;
+ (void)savePasteFile;

@end
