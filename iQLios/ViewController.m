//
//  ViewController.m
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import "ViewController.h"
#import "RB_renderview.h"
#import "RBKeyboardSupportField.h"

extern int QLStart(void);
extern void QLRestart(void);
extern void QLSetSpeed(int time);
extern void QLTimer(void);

static float EMULATOR_SPEED = 0.02f;
static MacRenderView* s_renderView = NULL;

@interface ViewController ()
@property (nonatomic, retain) MacRenderView* renderView;
@property (nonatomic, retain) RBKeyboardSupportField* keyboardSupportField;
@property (nonatomic, retain) NSTimer* fireTimer;
@property (nonatomic) RBEvent lastKeyEvent;
@end

@implementation ViewController

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

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    
    self.renderView.frame = self.view.bounds;
}

- (void)viewWillLayoutSubviews {
    [super viewWillLayoutSubviews];
    
    self.renderView.frame = self.view.bounds;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    self.renderView = [[MacRenderView alloc] initWithFrame:CGRectZero];
    [self.view addSubview:self.renderView];
    
    s_renderView = self.renderView;

    self.keyboardSupportField = [[RBKeyboardSupportField alloc] initWithFrame:CGRectMake(2400, 2400, 100, 20)];
    self.keyboardSupportField.keyboardAppearance = UIKeyboardAppearanceDark;
    [self.view addSubview:self.keyboardSupportField];

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
    
    self.fireTimer = [NSTimer timerWithTimeInterval:EMULATOR_SPEED target:self selector:@selector(triggerQL) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:self.fireTimer forMode:NSDefaultRunLoopMode];

    [self performSelectorInBackground:@selector(runQL) withObject:nil];
    
    [self.keyboardSupportField becomeFirstResponder];
}

@end

void platform_init(void) {}
void ql_set_title(char* title) {}
void ql_render_screen(void* buffer) {
    dispatch_async(dispatch_get_main_queue(), ^{
        [s_renderView render:buffer];
    });
}
