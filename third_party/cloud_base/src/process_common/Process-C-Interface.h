//
//  Process-C-Interface.h
//  ExampleMac
//
//  Created by cocoDevil on 2020/2/18.
//  Copyright © 2020 cocoDevil. All rights reserved.
//

#ifndef Process_C_Interface_h
#define Process_C_Interface_h
#include <string>
/*oc的桥接声明，当需要用到oc库的时候，都需要创建对应的桥接声明文件。再通过OcToC_bridge进行实现*/
bool c_get_appdata_path(std::string &tempPath);

/// 获取日志存放地址
bool c_get_log_path(std::string &logPath);

#endif /* Process_C_Interface_h */
