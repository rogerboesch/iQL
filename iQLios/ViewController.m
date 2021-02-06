//
//  ViewController.m
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import "ViewController.h"
#import "rb_renderview.h"
#import "rb_platform.h"
#import "rb_ghost.h"
#import "RBKeyboardSupportField.h"

extern int QLStart(void);
extern void QLRestart(void);
extern void QLSetSpeed(int time);
extern void QLTimer(void);

static float EMULATOR_SPEED = 0.02f;
static MacRenderView* s_renderView = NULL;
static ViewController* s_viewController = NULL;

@interface ViewController ()
@property (nonatomic, retain) MacRenderView* renderView;
@property (nonatomic, retain) RBKeyboardSupportField* keyboardSupportField;
@property (nonatomic, retain) NSTimer* fireTimer;
@property (nonatomic) RBEvent lastKeyEvent;
@property (nonatomic, retain) NSString* mountPath;
@end

@implementation ViewController

// MARK: - iCloud files

- (NSString *)fixFilename:(NSString *)filename {
    filename = [filename lowercaseString];
    NSCharacterSet* unwanted = [[NSCharacterSet characterSetWithCharactersInString:@"abcdefghijklmnopqrstuvwxyz."] invertedSet];
    NSString *QLfilename = [[filename componentsSeparatedByCharactersInSet: unwanted] componentsJoinedByString: @""];
    QLfilename = [QLfilename stringByReplacingOccurrencesOfString:@"." withString:@"_"];

    return QLfilename;
}

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentAtURL:(NSURL *)url {
    if (controller.documentPickerMode == UIDocumentPickerModeImport) {
        NSString* sourcePath = [url path];
        NSString* filename = [sourcePath lastPathComponent];
        filename = [self fixFilename:filename];
        NSString* destPath = [NSString stringWithFormat:@"%@%@", self.mountPath, filename];
        [RBPlatform copyFile:sourcePath to:destPath overwrite:YES];
    }
    
    self.mountPath = @"";
}

- (void)loadFromFilesToPath:(NSString *)path {
    if (self.mountPath.length != 0) {
        // Already active
        return;
    }
    
    self.mountPath = path;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        UIDocumentPickerViewController *documentPicker = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:@[@"public.item"] inMode:UIDocumentPickerModeImport];
        documentPicker.delegate = self;

        documentPicker.modalPresentationStyle = UIModalPresentationFormSheet;
        [self presentViewController:documentPicker animated:YES completion:nil];
    });
}

// MARK: - Super Basic

- (void)copySuperBasicCode:(id)sender {
    [RBGhost savePasteFile];
}

- (void)pasteSuperBasicCode:(id)sender {
    NSString* str = [[UIPasteboard generalPasteboard] string];
    NSString* path = [NSString stringWithFormat:@"%@%@%@", [RBPlatform getDocumentPath], @"mdv1/", @"_paste"];
    [RBPlatform saveContent:str to:path];
    [RBGhost loadPasteFile];
}

// MARK: - QL integration

- (void)restartQL {
    QLRestart();
}

- (void)runQL {
    QLStart();
}

- (void)triggerQL {
    QLTimer();
}

- (void)sendKeyDownEvent:(RBEvent)event {
    [self.renderView sendEvent:event];
}

- (void)sendKeyUpEvent {
    [self.renderView sendEvent:self.lastKeyEvent];
}

- (void)viewWillLayoutSubviews {
    [super viewWillLayoutSubviews];

    self.renderView.frame = self.view.bounds;
}

// MARK: - Menu interaction

- (UIContextMenuConfiguration *)contextMenuInteraction:(UIContextMenuInteraction *)interaction configurationForMenuAtLocation:(CGPoint)location {
    UIContextMenuConfiguration* config = [UIContextMenuConfiguration configurationWithIdentifier:nil
                                                                                  previewProvider:nil
                                                                                   actionProvider:^UIMenu* _Nullable(NSArray<UIMenuElement*>* _Nonnull suggestedActions) {
        NSMutableArray* actions = [[NSMutableArray alloc] init];
        
        UIAction* action = [UIAction actionWithTitle:@"Copy SuperBasic" image:[UIImage systemImageNamed:@"doc.on.doc"] identifier:nil handler:^(__kindof UIAction* _Nonnull action) {
            [self copySuperBasicCode:self];
        }];
        [actions addObject:action];

        action = [UIAction actionWithTitle:@"Paste SuperBasic" image:[UIImage systemImageNamed:@"doc.on.clipboard"] identifier:nil handler:^(__kindof UIAction* _Nonnull action) {
            [self pasteSuperBasicCode:self];
        }];
        [actions addObject:action];

        action = [UIAction actionWithTitle:@"Restart QL" image:[UIImage systemImageNamed:@"clear"] identifier:nil handler:^(__kindof UIAction* _Nonnull action) {
            [self restartQL];
        }];
        action.attributes = UIMenuElementAttributesDestructive;
        [actions addObject:action];

        action = [UIAction actionWithTitle:@"Reset QL" image:[UIImage systemImageNamed:@"trash.circle"] identifier:nil handler:^(__kindof UIAction* _Nonnull action) {
        }];
        action.attributes = UIMenuElementAttributesDestructive;
        [actions addObject:action];

        UIMenu* menu = [UIMenu menuWithTitle:@"" children:actions];
        return menu;
    }];

    return config;
}

// MARK: - View life cycle

- (void)viewDidLoad {
    [super viewDidLoad];

    self.renderView = [[MacRenderView alloc] initWithFrame:self.view.bounds];
    [self.view addSubview:self.renderView];

    s_viewController = self;
    s_renderView = self.renderView;

    self.keyboardSupportField = [[RBKeyboardSupportField alloc] initWithFrame:CGRectMake(2400, 2400, 100, 20)];
    self.keyboardSupportField.keyboardAppearance = UIKeyboardAppearanceDark;
    [self.view addSubview:self.keyboardSupportField];

    UIContextMenuInteraction* interaction = [[UIContextMenuInteraction alloc] initWithDelegate:self];
    [self.view addInteraction:interaction];
//    UILongPressGestureRecognizer* gesture = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(longPress:)];
  //  [self.view addGestureRecognizer:gesture];
    
    __weak __typeof(self) weakSelf = self;
    self.keyboardSupportField.callback = ^void (char ch, BOOL ctrlPressed) {
        RBEvent event;
        event.ch = ch;
        event.code = RBVK_Unknown;
        event.control = ctrlPressed;
        event.alt = 0;
        event.shift = 0;
        
        event.type = RBEVT_KeyPressed;
        
        switch (ch) {
            case '\r':
                event.code = RBVK_Return;
                break;
            case '\b':
                event.code = RBVK_BackSpace;
                break;
        }
        
        [weakSelf sendKeyDownEvent:event];
        
        event.type = RBEVT_KeyReleased;
        weakSelf.lastKeyEvent = event;
        [weakSelf performSelector:@selector(sendKeyUpEvent) withObject:nil afterDelay:0.1];
    };
    
    // Start
    self.fireTimer = [NSTimer timerWithTimeInterval:EMULATOR_SPEED target:self selector:@selector(triggerQL) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:self.fireTimer forMode:NSDefaultRunLoopMode];

    [self performSelectorInBackground:@selector(runQL) withObject:nil];
    
    [self.keyboardSupportField becomeFirstResponder];
}

@end

void rb_platform_load_file_from_cloud(char* mountPath) {
    NSString* path = [NSString stringWithCString:mountPath encoding:NSUTF8StringEncoding];
    [s_viewController loadFromFilesToPath:path];
}

void platform_init(void) {}
void ql_set_title(char* title) {}
void ql_render_screen(void* buffer) {
    dispatch_async(dispatch_get_main_queue(), ^{
        [s_renderView render:buffer];
    });
}
