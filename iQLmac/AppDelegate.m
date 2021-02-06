//
//  AppDelegate.m
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import "AppDelegate.h"
#import "rb_platform.h"
#import "rb_ghost.h"

extern int QLStart(void);
extern void QLRestart(void);
extern void QLSetSpeed(int time);
extern void QLTimer(void);

static float EMULATOR_SPEED = 0.02f;
static AppDelegate* s_appDelegate;

@interface AppWindow : NSWindow
@end

@interface AppWindowDelegate : NSObject<NSWindowDelegate>
@end

@interface AppDelegate ()

@property (nonatomic, retain) AppWindow *window;
@property (nonatomic, retain) AppWindowDelegate *windowDelegate;
@property (nonatomic, retain) NSTimer* fireTimer;
@property (nonatomic, retain) NSString *title;
@property (nonatomic, retain) NSMenu *menu;
@property (nonatomic, retain) RBToolbar *toolbar;
@property (nonatomic, retain) MacRenderView* renderView;

@end

@implementation AppDelegate

- (void)copySuperBasicCode:(id)sender {
    [RBGhost savePasteFile];
}

- (void)createAppMenu {
    self.menu = [[NSMenu alloc] initWithTitle:@"MainMenu"];
    NSApp.mainMenu = self.menu;
    NSMenuItem *submenu = [self.menu addItemWithTitle:@"Application" action:nil keyEquivalent:@""];
    NSMenu *menu = [[NSMenu alloc] initWithTitle:@"Application"];
    [self.menu setSubmenu:menu forItem:submenu];

    NSString* title = @"Quit";
    NSMenuItem *item = [menu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];
    item.target = NSApp;
}

- (void)createEditMenu {
    NSMenuItem *submenu = [self.menu addItemWithTitle:@"Edit" action:nil keyEquivalent:@""];
    NSMenu *menu = [[NSMenu alloc] initWithTitle:@"Edit"];
    [self.menu setSubmenu:menu forItem:submenu];

    [menu addItemWithTitle:@"Copy SuperBasic code" action:@selector(copySuperBasicCode:) keyEquivalent:@"c"];
    [menu addItemWithTitle:@"Paste SuperBasic code" action:@selector(paste:) keyEquivalent:@"v"];
}

- (void)createToolbar {
    self.toolbar = [[RBToolbar alloc] initWithIdentifier:@"AppToolbar"];
    self.toolbar.callback = self;
    
    [self.toolbar addSegment:@"Speed" labels:@[@"Normal", @"Slow"] images:@[[NSImage imageNamed:@"SpeedNormal"], [NSImage imageNamed:@"SpeedSlow"]] width:100];
    
    [self.toolbar addItem:@"Reset" image:[NSImage imageNamed:NSImageNameSlideshowTemplate]];
    [self.toolbar setLabel:@"Reset" forItem:@"Reset"];
    [self.toolbar setImage:[NSImage imageNamed:NSImageNameSlideshowTemplate] forItem:@"Reset"];

    self.window.toolbar = self.toolbar;
}

- (void)setupWindow {
    NSRect rect = NSMakeRect(0, 0, 512*2, 256*2);
    const NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    self.window = [[AppWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:NO];

    self.window.releasedWhenClosed = NO; // this is necessary for proper cleanup in applicationWillTerminate
    self.window.title = @"";
    self.window.acceptsMouseMovedEvents = YES;
    self.window.restorable = YES;

    self.windowDelegate = [[AppWindowDelegate alloc] init];
    self.window.delegate = self.windowDelegate;
    
    self.renderView = [[MacRenderView alloc] initWithFrame:rect];
    [self.renderView updateTrackingAreas];

    self.window.contentView = self.renderView;

    [self.window center];
    [self.window makeKeyAndOrderFront:nil];
    [self.window makeFirstResponder:self.renderView];
}

- (void)setWindowTitle:(NSString *)title {
    self.title = title;
    
    if (self.window != NULL) {
        [self.window setTitle:self.title];
    }
}

- (void)toggleSpeeed {
    switch ([self.toolbar selectionForSegment:@"Speed"]) {
        case 0:
            QLSetSpeed(0);
            break;
        case 1:
            QLSetSpeed(4);
            break;
        default:
            break;
    }
}

- (void)toolbarItemPressed:(NSString *)name {
    if ([name isEqualTo:@"Reset"]) {
        QLRestart();
    }
    else if ([name isEqualTo:@"Speed"]) {
        [self toggleSpeeed];
    }
}

- (void)runQL {
    QLStart();
}

- (void)triggerQL {
    QLTimer();
}

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
    [RBPlatform createFolder:@"iQLmac"];

    [RBPlatform createFolder:@"iQLmac/roms"];
    [RBPlatform createFolder:@"iQLmac/mdv1"];
    [RBPlatform createFolder:@"iQLmac/mdv2"];
    [RBPlatform createFolder:@"iQLmac/flp1"];
    [RBPlatform createFolder:@"iQLmac/temp"];
    
    [self copyFile:@"ql.ini" toFolder:@""];
    [self copyFile:@"min198.rom" toFolder:@"roms"];
    [self copyFile:@"ql_jm.rom" toFolder:@"roms"];
    [self copyFile:@"ql_js.rom" toFolder:@"roms"];
    [self copyFile:@"tk232.rom" toFolder:@"roms"];
    [self copyFile:@"BOOT" toFolder:@"mdv1"];
    [self copyFile:@"invaders_bas" toFolder:@"mdv2"];
    [self copyFile:@"snake_bas" toFolder:@"mdv2"];
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    s_appDelegate = self;

    [self createQLSystem];

    [self setupWindow];
    [self createAppMenu];
    [self createEditMenu];
    [self createToolbar];

    self.fireTimer = [NSTimer timerWithTimeInterval:EMULATOR_SPEED target:self selector:@selector(triggerQL) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:self.fireTimer forMode:NSDefaultRunLoopMode];

    [self performSelectorInBackground:@selector(runQL) withObject:nil];

    [NSApp activateIgnoringOtherApps:YES];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
}

@end

@implementation AppWindow

- (instancetype)initWithContentRect:(NSRect)contentRect
                          styleMask:(NSWindowStyleMask)style
                            backing:(NSBackingStoreType)backingStoreType
                              defer:(BOOL)flag {
    if (self = [super initWithContentRect:contentRect styleMask:style backing:backingStoreType defer:flag]) {
        [self registerForDraggedTypes:[NSArray arrayWithObject:NSPasteboardTypeFileURL]];
    }
    
    return self;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
    return NSDragOperationCopy;
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
    return NSDragOperationCopy;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
    return NO;
}

@end

@implementation AppWindowDelegate

- (BOOL)windowShouldClose:(id)sender {
    return YES;
}

- (void)windowDidResize:(NSNotification*)notification {
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification {
}

- (void)windowDidExitFullScreen:(NSNotification*)notification {
}

@end

void rb_platform_load_file_from_cloud(char* mountPath) {
    // Just needed on iOS
}

void ql_set_title(char* title) {
    NSString* str = [[NSString alloc] initWithCString:title encoding:NSUTF8StringEncoding];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [s_appDelegate setWindowTitle:str];
    });
}

void ql_render_screen(void* buffer) {
    dispatch_async(dispatch_get_main_queue(), ^{
        [s_appDelegate.renderView render:buffer];
    });
}
