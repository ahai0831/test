//
//  main.cpp
//  xianshi_load
//
//  Created by zhaozt@corp.21cn.com on 2020/1/2.
//  Copyright © 2020 zhaozt@corp.21cn.com. All rights reserved.
//

//显式加载
//显式链接
//显式链接则是利用API函数实现加载和卸载共享库，获取带调用函数地址，获取错误信息等功能。(动态加载共享库)
#include <iostream>
#include <dlfcn.h>//// 显式加载需要用到的头文件
#include <pthread.h>
#include <unistd.h>
#include <future>


typedef void (*PStart)(const char *);

typedef void (*PCallback)(const char *);

typedef void (*AProcess)(const char *,PStart,PCallback);


void  start(const char *start){
    printf("start\nthe uuid is %s\n",start);
}
void callback(const char *callback){
    printf("%s",callback);
}

void past(const char *,PStart,PCallback)
{
    
}

int main(int argc, const char * argv[]) {
    
    
//需要引入头文件 dlfcn.h
//第一个参数：字符串形式的共享库文件名
//第二个参数：标志 RTLD_LAZY -懒加载 只适用于函数。只有在函数被执行的时候，才确定函数的地址。函数不执行，不加载。RTLD_NOW -立即加载       在dlopen返回之前，动态库的符号就已经确定了地址
//返回值：通用类型指针, 成功返回句柄，暂时理解为首地址,失败返回NULL
//函数功能：主要用于打开和加载共享库文件
    AProcess ast = &past;
    PStart pstart = &start;
    PCallback pcallback = &callback;

    void *handle = dlopen("/Users/zhaoztcorp.21cn.com/Desktop/fake_dll/fake_dll/fake.dylib", RTLD_NOW);//加载共享库
    if (handle == NULL) {
        char *str = dlerror();
        if (str != NULL) {
            printf("出现问题:%s",str);
        }
        return -1;
    }

//dlerror 函数功能：主要用于获取dlopen等函数调用过程发生的最近一个错误的详细信息,返回NULL则表示没有错误发生


//    第一个参数：句柄，也就是dlopen函数的返回值
//第二个参数：字符串形式的符号,表示函数名
//返回值：成功返回函数在内存中的地址,失败返回NULL
//函数功能： 主要用于根据句柄和函数名获取在内存中的地址
  *(void **) (&ast) = dlsym(handle, "AstProcess");//定位动态库中的函数
    if(ast == NULL) {
            char * str = dlerror();
//     char *dlerror(void);   函数功能：主要用于获取dlopen等函数调用过程发生的最近一个错误的详细信息,返回NULL则表示没有错误发生
            if(str!=NULL) {
                printf("出现问题：%s",str);
            }
            dlclose(handle);
            return -1;
        }
    

        ast("111",pstart,pcallback);
    
        sleep(5);
    
//    函数功能： 主要用于关闭参数handle所指定的共享库,成功返回0，失败返回非0，当共享库不再被任何程序使用时，则回收共享库所占用的内存空间
        dlclose(handle);
        return 0;

}

