//
//  OcToC_bridge.m
//  ExampleMac
//
//  Created by cocoDevil on 2020/2/18.
//  Copyright © 2020 cocoDevil. All rights reserved.
//

#import "OcToC_bridge.h"
#import "Process-C-Interface.h"
#include <string>
#include <dirent.h>
#include <sys/stat.h>

static OcToC_bridge *ocToC_bridge = nil;
@implementation OcToC_bridge
+(instancetype)shareInstance{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        ocToC_bridge = [[self alloc] init];
    });
    return ocToC_bridge;
}

bool c_get_appdata_path(std::string &tempPath) {
    if (!ocToC_bridge) {
        ocToC_bridge = [OcToC_bridge shareInstance];
    }
    NSString *tempString = [ocToC_bridge c_getAppTempPath];
    if (tempString.length > 0) {
        tempPath = [tempString UTF8String];
        return true;
    }else{
        tempPath = "";
        return false;
    }
}

-(NSString *)c_getAppTempPath {
    NSString *path = NSTemporaryDirectory();
    if (path.length > 0) {
        return path;
    }else{
        return @"";
    }
}

bool c_get_log_path(std::string &logPath){
    if (!ocToC_bridge) {
        ocToC_bridge = [OcToC_bridge shareInstance];
    }
    // NSString *homePath = NSHomeDirectory();
    NSString *homePath = [[NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0] stringByAppendingPathComponent:@"com.21cn.cloud-dist"];
         
    NSFileManager *fm = [NSFileManager defaultManager];
    BOOL isYes;
    isYes = [fm fileExistsAtPath:homePath];
    if (!isYes) {
        NSError *error;
        if(![fm createDirectoryAtPath:homePath withIntermediateDirectories:YES attributes:NULL error:&error]){
            NSLog(@"%@", error);
        }else{
            isYes = YES;
        }
    }

    if (homePath.length > 0 && isYes) {
        std::string logString = [homePath UTF8String];
        // if (opendir(logString.c_str()) == NULL) {
        //     mkdir(logString.c_str(), S_IRWXU);
        // }
        // logPath = [[NSString stringWithFormat:@"%@/cloud-log", homePath] UTF8String];
        logPath = logString.c_str();
        return true;
    }else{
        logPath = "";
        return false;
    }
}

-(NSString *)c_getAppLogPath{
    NSString *path = NSHomeDirectory();
    if (path.length > 0) {
        return path;
    }else{
        return @"";
    }
}

bool c_get_process_name(std::string &processName) {
    if (!ocToC_bridge) {
        ocToC_bridge = [OcToC_bridge shareInstance];
    }
    NSProcessInfo *pInfo = [NSProcessInfo processInfo];
    NSString *processNameStr = [pInfo processName];
    if (processNameStr.length > 0) {
        std::string nameString = [processNameStr UTF8String];
        processName = nameString.c_str();
        return true;
    }else{
        processName = "";
        return false;
    }
}

@end
