//
//  rb_virtual_keys.h
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#ifndef RB_VIRTUAL_KEY_H
#define RB_VIRTUAL_KEY_H

typedef enum _RBVirtualKey {
	RBVK_Unknown = -1, 		// Unhandled key
	RBVK_A = 0,        		// The A key
	RBVK_B,            		// The B key
	RBVK_C,            		// The C key
	RBVK_D,            		// The D key
	RBVK_E,            		// The E key
	RBVK_F,            		// The F key
	RBVK_G,            		// The G key
	RBVK_H,            		// The H key
	RBVK_I,            		// The I key
	RBVK_J,            		// The J key
	RBVK_K,            		// The K key
	RBVK_L,            		// The L key
	RBVK_M,            		// The M key
	RBVK_N,            		// The N key
	RBVK_O,            		// The O key
	RBVK_P,            		// The P key
	RBVK_Q,            		// The Q key
	RBVK_R,            		// The R key
	RBVK_S,            		// The S key
	RBVK_T,            		// The T key
	RBVK_U,            		// The U key
	RBVK_V,            		// The V key
	RBVK_W,            		// The W key
	RBVK_X,            		// The X key
	RBVK_Y,            		// The Y key
	RBVK_Z,            		// The Z key
	RBVK_Num0,         		// The 0 key (26)
	RBVK_Num1,         		// The 1 key
	RBVK_Num2,         		// The 2 key
	RBVK_Num3,         		// The 3 key
	RBVK_Num4,         		// The 4 key
	RBVK_Num5,         		// The 5 key
	RBVK_Num6,         		// The 6 key
	RBVK_Num7,         		// The 7 key
	RBVK_Num8,         		// The 8 key
	RBVK_Num9,         		// The 9 key (35)
	RBVK_LBracket,      	// The ( key
	RBVK_RBracket,     		// The ) key
	RBVK_LSquareBracket, 	// The [ key
	RBVK_RSquareBracket, 	// The ] key
    RBVK_LCurlyBracket,     // The { key
    RBVK_RCurlyBracket,     // The } key
	RBVK_Colon,    	  		// The : key
	RBVK_SemiColon,    		// The ; key
	RBVK_Comma,        		// The , key
	RBVK_Period,       		// The . key
    RBVK_Quote,             // The ' key
    RBVK_DoubleQuote,       // The " key
	RBVK_Slash,        		// The / key
	RBVK_BackSlash,    		// The \ key
	RBVK_Tilde,        		// The ~ key
	RBVK_Equal,        		// The = key
	RBVK_Dash,         		// The - key
    RBVK_Add,               // The + key
    RBVK_Subtract,          // The - key
    RBVK_Multiply,          // The * key
    RBVK_Divide,            // The / key
    RBVK_Less,              // The < key
    RBVK_Greater,           // The > key

    RBVK_Exlamation,        // ! (59)
    RBVK_Question,          // ?
    RBVK_At,                // @
    RBVK_Hash,              // #
    RBVK_Dollar,            // $
    RBVK_Percent,           // %
    RBVK_CircAccent,        // ^
    RBVK_Ampersand,         // &
    RBVK_VertLine,          // |
    RBVK_Underline,         // _
    RBVK_Grave,             // `
	RBVK_Space,        		// The Space key (69)
    RBVK_Tab,               // The Tabulation key
    RBVK_Return,            // The Return key
	RBVK_BackSpace,    		// The Backspace key
    RBVK_Escape,            // The Escape key

    // Probably dont need any character equivalent
    RBVK_Left,              // Left arrow (74)
    RBVK_Right,             // Right arrow
    RBVK_Up,                // Up arrow
    RBVK_Down,              // Down arrow
    RBVK_Insert,            // The Insert key
    RBVK_Delete,            // The Delete key
	RBVK_PageUp,       		// The Page up key
	RBVK_PageDown,     		// The Page down key
    RBVK_LControl,          // The left Control key
    RBVK_LShift,            // The left Shift key
    RBVK_LAlt,              // The left Alt key
    RBVK_LSystem,           // The left OS specific key: window (Windows and Linux), apple (MacOS X), ...
    RBVK_RControl,          // The right Control key
    RBVK_RShift,            // The right Shift key
    RBVK_RAlt,              // The right Alt key
    RBVK_RSystem,           // The right OS specific key: window (Windows and Linux), apple (MacOS X), ...
    RBVK_Menu,              // The Menu key
	RBVK_End,          		// The End key
	RBVK_Home,         		// The Home key
	RBVK_Numpad0,      		// The numpad 0 key (93)
	RBVK_Numpad1,      		// The numpad 1 key
	RBVK_Numpad2,      		// The numpad 2 key
	RBVK_Numpad3,      		// The numpad 3 key
	RBVK_Numpad4,      		// The numpad 4 key
	RBVK_Numpad5,      		// The numpad 5 key
	RBVK_Numpad6,      		// The numpad 6 key
	RBVK_Numpad7,      		// The numpad 7 key
	RBVK_Numpad8,      		// The numpad 8 key
	RBVK_Numpad9,      		// The numpad 9 key
	RBVK_F1,           		// The F1 key (103)
	RBVK_F2,           		// The F2 key
	RBVK_F3,           		// The F3 key
	RBVK_F4,           		// The F4 key
	RBVK_F5,           		// The F5 key
	RBVK_F6,           		// The F6 key
	RBVK_F7,           		// The F7 key
	RBVK_F8,           		// The F8 key
	RBVK_F9,           		// The F9 key
	RBVK_F10,          		// The F10 key
	RBVK_F11,          		// The F11 key
	RBVK_F12,          		// The F12 key
	RBVK_F13,          		// The F13 key
	RBVK_F14,          		// The F14 key
	RBVK_F15,          		// The F15 key
	RBVK_Pause,        		// The Pause key (118)
    RBVK_CapsLock,          // Caps Lock
    RBVK_LGUI,              // Aplple/Windows key etc.
    
    RBVK_CtrlA,             // Ctrl+Key Combinations (119)
    RBVK_CtrlE,
    RBVK_CtrlF,
    RBVK_CtrlO,
    RBVK_CtrlR,
    RBVK_CtrlQ,
    RBVK_CtrlS,
    RBVK_CtrlU,
    
	RBVK_KeyCount      		// Keep last -- the total number of keyboard keys
} RBVirtualKey;

typedef enum _RBEventType {
    RBEVT_None,
	RBEVT_Closed,                 // The window requested to be closed (no data)
	RBEVT_Resized,                // The window was resized (data in event.size)
	RBEVT_LostFocus,              // The window lost the focus (no data)
	RBEVT_GainedFocus,            // The window gained the focus (no data)
	RBEVT_TextEntered,            // A character was entered (data in event.text)
	RBEVT_KeyPressed,             // A key was pressed (data in event.key)
	RBEVT_KeyReleased,            // A key was released (data in event.key)
	RBEVT_MouseWheelMoved,        // The mouse wheel was scrolled (data in event.mouseWheel) (deprecated)
	RBEVT_MouseWheelScrolled,     // The mouse wheel was scrolled (data in event.mouseWheelScroll)
	RBEVT_MouseButtonPressed,     // A mouse button was pressed (data in event.mouseButton)
	RBEVT_MouseButtonReleased,    // A mouse button was released (data in event.mouseButton)
	RBEVT_MouseMoved,             // The mouse cursor moved (data in event.mouseMove)
	RBEVT_MouseEntered,           // The mouse cursor entered the area of the window (no data)
	RBEVT_MouseLeft,              // The mouse cursor left the area of the window (no data)
	RBEVT_JoystickButtonPressed,  // A joystick button was pressed (data in event.joystickButton)
	RBEVT_JoystickButtonReleased, // A joystick button was released (data in event.joystickButton)
	RBEVT_JoystickMoved,          // The joystick moved along an axis (data in event.joystickMove)
	RBEVT_JoystickConnected,      // A joystick was connected (data in event.joystickConnect)
	RBEVT_JoystickDisconnected,   // A joystick was disconnected (data in event.joystickConnect)
	RBEVT_TouchBegan,             // A touch event began (data in event.touch)
	RBEVT_TouchMoved,             // A touch moved (data in event.touch)
	RBEVT_TouchEnded,             // A touch event ended (data in event.touch)
	RBEVT_SensorChanged,          // A sensor value changed (data in event.sensor)

	RBEVT_Count                   // Keep last -- the total number of event types
} RBEventType;

typedef struct _RBEvent {
	RBEventType type; 	// Event type
	RBVirtualKey code;	// Code of the key that has been pressed
    int ch; 			// Character
    int alt;            // Is the Alt key pressed?
    int control; 		// Is the Control key pressed?
    int shift;   		// Is the Shift key pressed?
    int mouseBtn;       // Mouse Button
    int mouseX;         // Mouse x pos
    int mouseY;         // Mouse y pos
} RBEvent;

#endif
