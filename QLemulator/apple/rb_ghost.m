//
//  rb_ghost.m
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import <AppKit/AppKit.h>

#import "rb_ghost.h"
#import "rb_virtual_keys.h"
#import "rb_platform.h"

extern void QLRBSendEvent(RBEvent evt);

NSOperationQueue* s_ghostQueue = NULL;

@implementation RBGhost

+ (void)sendEvent:(RBEvent)evt {
    if (evt.type == RBEVT_KeyPressed || evt.type == RBEVT_KeyReleased) {
        // Test ascii characters
        if (isupper(evt.ch)) {
            evt.shift = 1;
        }
        
        char ch = toupper(evt.ch);

        if ((int)ch > 0) {
            int keyCode = character_to_vk(ch);
            int newKeyCode = ql_shift_key_fixes(ch, keyCode);
            evt.code = newKeyCode;
            evt.ch = ch;

            if (newKeyCode != keyCode) {
                evt.shift = 1;
            }
        }
    }
    
    QLRBSendEvent(evt);
    usleep(20*1000);
}

+ (void)typeInCharacter:(char)ch {
    [s_ghostQueue addOperationWithBlock:^{
        RBEvent evt;
        evt.type = RBEVT_KeyPressed;
        evt.code = RBVK_Unknown;
        evt.ch = ch;
        evt.control = 0;
        evt.shift = 0;
        evt.alt = 0;

        [RBGhost sendEvent:evt];
    }];
    
    [s_ghostQueue addOperationWithBlock:^{
        RBEvent evt;
        evt.type = RBEVT_KeyReleased;
        evt.ch = ch;
        
        [RBGhost sendEvent:evt];
    }];
}

+ (void)copyToPasteboard {
    [s_ghostQueue addOperationWithBlock:^{
        NSString* path = [NSString stringWithFormat:@"%@mdv1/_paste", [RBPlatform getDocumentPath]];
        NSString* content = [RBPlatform loadContentFromPath:path];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [[NSPasteboard generalPasteboard] clearContents];
            [[NSPasteboard generalPasteboard] setString:content forType:NSPasteboardTypeString];
        });
    }];
}

+ (void)deleteFile:(NSString *)path {
    [s_ghostQueue addOperationWithBlock:^{
        [RBPlatform deleteFile:path];
    }];
}

+ (void)typeIn:(NSString *)command {
    if (s_ghostQueue == NULL) {
        s_ghostQueue = [NSOperationQueue new];
        s_ghostQueue.maxConcurrentOperationCount = 1;
    }
    
    for (int i = 0; i < command.length; i++) {
        char ch = [command characterAtIndex:i];
        [RBGhost typeInCharacter:ch];
    }

    [RBGhost typeInCharacter:'\r'];
}

+ (void)loadPasteFile {
    [RBGhost typeIn:@"load mdv1__paste"];

    NSString* path = [NSString stringWithFormat:@"%@mdv1/_paste", [RBPlatform getDocumentPath]];
    [RBGhost deleteFile:path];
}

+ (void)savePasteFile {
    // To prevent that the QL ask for overwrite
    NSString* path = [NSString stringWithFormat:@"%@mdv1/_paste", [RBPlatform getDocumentPath]];
    [RBPlatform deleteFile:path];

    [RBGhost typeIn:@"save mdv1__paste"];
    [RBGhost copyToPasteboard];
    [RBGhost deleteFile:path];
}

@end
