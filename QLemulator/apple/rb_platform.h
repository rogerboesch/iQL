//
//  rb_platform.h
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import <Foundation/Foundation.h>

@interface RBPlatform : NSObject

+ (void)createFolder:(NSString*)name;
+ (void)copyFile:(NSString*)fromPath to:(NSString*)toPath overwrite:(BOOL)flag;
+ (void)deleteFile:(NSString*)atPath;

+ (NSString *)getDocumentPath;
+ (NSString *)getTemporaryPath;
+ (NSString *)getResourcePath;

+ (BOOL)saveContent:(NSString *)str to:(NSString *)path;
+ (NSString *)loadContentFromPath:(NSString *)path;

@end
