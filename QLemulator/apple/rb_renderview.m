//
//  rb_renderview.m
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import "rb_renderview.h"
#import "rb_platform.h"
#import "rb_ghost.h"

extern void QLRBSendEvent(RBEvent evt);

@interface MacRenderView ()

@property (nonatomic, retain) OSImageView* renderImageView;
@property (nonatomic) BOOL shiftKey;
@property (nonatomic) BOOL ctrlKey;
@property (nonatomic) BOOL altKey;

@end

@implementation MacRenderView

#if TARGET_OS_IOS

#pragma mark - Render

- (OSImage *)imageWithBuffer:(void *)buffer width:(int)width height:(int)height {
    int length = width*height*4;
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer, length, NULL);
    
    int bitsPerComponent = 8;
    int bitsPerPixel = 4 * bitsPerComponent;
    int bytesPerRow = 4*width;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGImageRef imageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    CGDataProviderRelease(provider);
    
    UIImage* myImage = [UIImage imageWithCGImage:imageRef];
    
    //CGDataProviderRelease(provider);
    //CGColorSpaceRelease(colorSpaceRef);
    // CGImageRelease(imageRef); // TODO: This results in a crash

    return myImage;
}

#else

- (OSImage *)imageWithBuffer:(void *)buffer width:(int)width height:(int)height {
    size_t bufferLength = width * height * 4;
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer, bufferLength, NULL);
    size_t bitsPerComponent = 8;
    size_t bitsPerPixel = 32;
    size_t bytesPerRow = 4 * width;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedLast;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;

    CGImageRef iref = CGImageCreate(width,
                                    height,
                                    bitsPerComponent,
                                    bitsPerPixel,
                                    bytesPerRow,
                                    colorSpaceRef,
                                    bitmapInfo,
                                    provider,   // data provider
                                    NULL,       // decode
                                    YES,        // should interpolate
                                    renderingIntent);

    OSImage* image =  [[OSImage alloc] initWithCGImage:iref size:CGSizeMake(width, height)];

    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpaceRef);
    CGImageRelease(iref);

    return image;
}

#endif

- (void)render:(unsigned char*)buffer {
    OSImage* image = [self imageWithBuffer:buffer width:512 height:256];
    self.renderImageView.image = image;
}

#pragma mark - UI events

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)becomeFirstResponder {
    return YES;
}

- (BOOL)resignFirstResponder {
    return YES;
}

#if TARGET_OS_IOS

- (void)sendEvent:(RBEvent)event {
    if (event.code == RBVK_Unknown) {
        // Test ascii characters
        if (isupper(event.ch))
            event.shift = 1;
        
        char ch = toupper(event.ch);

        if ((int)ch > 0) {
            int keyCode = character_to_vk(ch);
            int newKeyCode = ql_shift_key_fixes(ch, keyCode);
            event.code = newKeyCode;
            
            if (newKeyCode != keyCode) {
                event.shift = 1;
            }
        }
    }
    
    QLRBSendEvent(event);
}

#else

- (void)paste:(id)sender {
    NSString* str = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
    NSString* path = [NSString stringWithFormat:@"%@%@%@", [RBPlatform getDocumentPath], @"ram1/", @"paste"];
    [RBPlatform saveContent:str to:path];
    [RBGhost loadPasteFile];
}

- (void)mouseDown:(NSEvent*)theEvent {
    [self makeFirstResponder];
}

- (void)makeFirstResponder {
    [self.window makeFirstResponder:self];
}

- (void)processEvent:(NSEvent *)event type:(int)eventType {
    int ctrlPressed = 0;
    int shiftPressed = 0;
    int altPressed = 0;
    RBVirtualKey keyCode = RBVK_Unknown;
    char ch = 0;
    
    // Test modifiers
    if (event.modifierFlags & NSEventModifierFlagControl) {
        ctrlPressed = 1;
    }
    
    if (event.modifierFlags & NSEventModifierFlagShift) {
        shiftPressed = 1;
    }
    
    if (event.modifierFlags & NSEventModifierFlagOption) {
        altPressed = 1;
    }
    
    // Test escape codes
    if (keyCode == RBVK_Unknown) {
        // Special chars
        const char* keytest = [event.characters UTF8String];

        if (strcmp(keytest, "\x7f") == 0) {
            keyCode = RBVK_BackSpace;
            ch = 8;
        }
        else if (strcmp(keytest, "\x1b") == 0) {
            keyCode = RBVK_Escape;
            ch = 8;
        }
    }
    
    // Test arrow keys
    if (keyCode == RBVK_Unknown) {
        // Parse for known special character
        int vk = event.keyCode;
        switch (vk) {
            case 122:    // F1
                keyCode = RBVK_F1;
                break;
            case 120:    // F2
                keyCode = RBVK_F2;
                break;
            case 99:    // F3
                keyCode = RBVK_F3;
                break;
            case 118:    // F4
                keyCode = RBVK_F4;
                break;
            case 96:    // F5
                keyCode = RBVK_F5;
                break;
            case 97:    // F6
                keyCode = RBVK_F6;
                break;
            case 98:    // F7
                keyCode = RBVK_F7;
                break;
            case 100:    // F8
                keyCode = RBVK_F8;
                break;
            case 101:    // F9
                keyCode = RBVK_F9;
                break;
            case 109:    // F10
                keyCode = RBVK_F10;
                break;
            case 110:    // F10
                keyCode = RBVK_F11;
                break;
            case 111:    // F12
                keyCode = RBVK_F12;
                break;

            case 123:   // arrrow (left)
                keyCode = RBVK_Left;
                break;
            case 124:   // arrrow (right)
                keyCode = RBVK_Right;
                break;
            case 125:   // arrrow (down)
                keyCode = RBVK_Down;
                break;
            case 126:   // arrrow (up)
                keyCode = RBVK_Up;
                break;
                
            case 115:   // Home
                keyCode = RBVK_Home;
                break;
            case 119:   // End
                keyCode = RBVK_End;
                break;
            case 116:   // PgUp
                keyCode = RBVK_PageUp;
                break;
            case 121:   // PgDown
                keyCode = RBVK_PageDown;
                break;
        }
    }

    // Test ascii characters
    if (event.characters.length == 1 && keyCode == RBVK_Unknown) {
        ch = [event.charactersIgnoringModifiers characterAtIndex:0];

        if ((int)ch > 0) {
            char ch2 = [[event.charactersIgnoringModifiers uppercaseString] characterAtIndex:0];
            keyCode = character_to_vk(ch2);
            keyCode = ql_shift_key_fixes(ch2, keyCode);
        }
    }
    
    RBEvent evt;
    evt.type = eventType;
    evt.control = ctrlPressed;
    evt.shift = shiftPressed;
    evt.alt = altPressed;
    evt.code = keyCode;
    evt.ch = ch;

    QLRBSendEvent(evt);
}

- (void)modifierKeyEvent:(int)code pressed:(BOOL)pressed {
    RBEvent evt;
    evt.type = pressed ? RBEVT_KeyPressed : RBEVT_KeyReleased;
    evt.control = false;
    evt.shift = false;
    evt.alt = false;
    evt.code = code;
    evt.ch = ' ';

    QLRBSendEvent(evt);
}

- (void)flagsChanged:(NSEvent *)event {
    // FIXME: Add capslock support
    if ([event modifierFlags] & NSEventModifierFlagOption) {
        if (!self.altKey) {
            [self modifierKeyEvent:RBVK_LAlt pressed: true];
        }
        self.altKey = true;
    }
    else if ([event modifierFlags] | NSEventModifierFlagOption) {
        if (self.altKey) {
            [self modifierKeyEvent:RBVK_LAlt pressed: false];
        }
        self.altKey = false;
    }

    if ([event modifierFlags] & NSEventModifierFlagShift) {
        if (!self.shiftKey) {
            [self modifierKeyEvent:RBVK_LShift pressed: true];
        }
        self.shiftKey = true;
    }
    else if ([event modifierFlags] | NSEventModifierFlagShift) {
        if (self.shiftKey) {
            [self modifierKeyEvent:RBVK_LShift pressed: false];
        }
        self.shiftKey = false;
    }

    if ([event modifierFlags] & NSEventModifierFlagControl) {
        if (!self.ctrlKey) {
            [self modifierKeyEvent:RBVK_LControl pressed: true];
        }
        self.ctrlKey = true;
    }
    else if ([event modifierFlags] | NSEventModifierFlagControl) {
        if (self.ctrlKey) {
            [self modifierKeyEvent:RBVK_LControl pressed: false];
        }
        self.ctrlKey = false;
    }
}

- (void)keyDown:(NSEvent *)event {
	[self processEvent:event type:RBEVT_KeyPressed];
}

- (void)keyUp:(NSEvent *)event {
	[self processEvent:event type:RBEVT_KeyReleased];
}

#endif

#pragma mark - View

- (BOOL)isFlipped {
    return true;
}

- (void)resizeSubviewsWithOldSize:(CGSize)oldSize {
	self.renderImageView.frame = self.bounds;
}

- (void)setFrame:(CGRect)frame {
	[super setFrame:frame];

	self.renderImageView.frame = self.bounds;
}

#pragma mark - Initialisation

- (id)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:frame];

#if TARGET_OS_IOS
    self.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

    self.renderImageView = [[UIImageView alloc] initWithFrame:CGRectZero];
    self.renderImageView.backgroundColor = UIColor.blackColor;
    self.renderImageView.contentMode = UIViewContentModeScaleAspectFit;
    self.renderImageView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self addSubview:self.renderImageView];
#else
    self.renderImageView = [[OSImageView alloc] initWithFrame:NSZeroRect];
    self.renderImageView.imageScaling = NSImageScaleProportionallyUpOrDown;
    self.renderImageView.imageAlignment = NSImageAlignTop;
    self.renderImageView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [self addSubview:self.renderImageView];
#endif
    
    self.altKey = false;
    self.ctrlKey = false;
    self.shiftKey = false;

	return self;
}

@end
