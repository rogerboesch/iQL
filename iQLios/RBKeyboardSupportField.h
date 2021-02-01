//
//  RBKeyboardSupportField.h
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import <UIKit/UIKit.h>
#include "rb_virtual_keys.h"

typedef void (^RBKeyboardSupportFieldCallback)(char, BOOL);

@interface RBKeyboardSupportField : UITextField <UITextFieldDelegate>

@property (nonatomic, copy) RBKeyboardSupportFieldCallback callback;

@end
