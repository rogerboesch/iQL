//
//  rb_toolbar.h
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import <Cocoa/Cocoa.h>

@protocol RBToolbarDelegate

- (void)toolbarItemPressed:(NSString *)name;

@end

@interface RBToolbar : NSToolbar <NSToolbarDelegate> {
}

@property (nonatomic, assign) id<RBToolbarDelegate> callback;

- (void)setLabel:(NSString *)label forItem:(NSString *)name;
- (void)setImage:(NSImage *)image forItem:(NSString *)name;

- (void)addItem:(NSString *)name image:(NSImage *)image;
- (void)addSegment:(NSString *)name labels:(NSArray *)labels images:(NSArray *)images width:(CGFloat)width;
- (void)enableSegment:(NSString *)name enable:(BOOL)enable;

- (int)selectionForSegment:(NSString *)name;

@end


