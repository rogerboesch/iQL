//
//  rb_renderview.h
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import "TargetConditionals.h"
#import "rb_virtual_keys.h"

#if TARGET_OS_IOS

#import <UIKit/UIKit.h>

@interface MacRenderView : UIView

- (void)sendEvent:(RBEvent)event;
- (void)render:(unsigned char *)buffer;

@end

typedef UIImageView OSImageView;
typedef UIImage OSImage;

#else

#import <Cocoa/Cocoa.h>

@interface MacRenderView : NSView

- (void)makeFirstResponder;
- (void)render:(unsigned char *)buffer;

@end

typedef NSImageView OSImageView;
typedef NSImage OSImage;

#endif

