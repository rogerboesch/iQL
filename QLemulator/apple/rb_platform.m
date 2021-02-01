//
//  rb_platform.m
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import "rb_platform.h"

char documentPath[256];
char temporaryPath[256];
char resourcePath[256];

@interface RBPlatform()

@end

@implementation RBPlatform

+ (void)copyFile:(NSString*)fromPath to:(NSString*)toPath overwrite:(BOOL)flag {
    NSURL* fromURL = [NSURL fileURLWithPath:fromPath];
    NSURL* toURL = [NSURL fileURLWithPath:toPath];

    NSError *error;
    
    if (flag) {
        [[NSFileManager defaultManager] removeItemAtURL:toURL error:&error];
    }
    
    if (![[NSFileManager defaultManager] copyItemAtURL:fromURL toURL:toURL error:&error]) {
       NSLog(@"Error occurred: %@", error);
    }
}

+ (void)deleteFile:(NSString*)atPath {
    NSURL* url = [NSURL fileURLWithPath:atPath];

    NSError *error;
    [[NSFileManager defaultManager] removeItemAtURL:url error:&error];
}

+ (void)createFolder:(NSString*)name {
    NSURL* url = [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject];
    NSString* path = [url path];
    path = [NSString stringWithFormat:@"%@/%@/", path, name];
    
    NSError *error;
    [[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:YES attributes:NULL error:&error];
}

+ (NSString *)getDocumentPath {
    NSURL* url = [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject];
    NSString* path = [url path];

#if TARGET_OS_IOS
    path = [NSString stringWithFormat:@"%@/", path];
#else
    path = [NSString stringWithFormat:@"%@/iQLmac/", path];
#endif

    return path;
}

+ (NSString *)getTemporaryPath {
    NSString* path = [RBPlatform getDocumentPath];

    path = [NSString stringWithFormat:@"%@temp/", path];

    return path;
}

+ (NSString *)getResourcePath {
    NSString *path = [[NSBundle mainBundle] resourcePath];
    path = [NSString stringWithFormat:@"%@/", path];

    return path;
}

+ (BOOL)saveContent:(NSString *)str to:(NSString *)path {
    NSError *error;
    [str writeToFile:path atomically:NO encoding:NSUTF8StringEncoding error:&error];
    
    return error == NULL ? YES : NO;
}

+ (NSString *)loadContentFromPath:(NSString *)path {
    NSError *error;
    NSString* content = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:&error];
    
    return content;
}

@end

// Root path for all system files (as drives, roms, etc.)
char* rb_get_platform_system_path() {
    strcpy(documentPath, (char *)[[RBPlatform getDocumentPath] UTF8String]);
    return documentPath;
}

// Temporary path
char* rb_get_temporary_path() {
    strcpy(temporaryPath, (char *)[[RBPlatform getTemporaryPath] UTF8String]);
    return temporaryPath;
}

// Root path for all bundle resources
char* rb_get_platform_resource_path() {
    strcpy(resourcePath, (char *)[[RBPlatform getResourcePath] UTF8String]);
    return resourcePath;
}

