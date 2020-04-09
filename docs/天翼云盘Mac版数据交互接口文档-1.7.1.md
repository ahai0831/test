# 天翼云盘Mac版数据交互接口文档

**版权所有  不得翻印  内部资料**

**Version 0.2**

**修订历史：**

| **版本号** | **时间**  | **作者** | **内容**                                                                            |
| ---------- | --------- | -------- | ----------------------------------------------------------------------------------- |
| 1.0        | 2020.1.7  | 田由     | 创建                                                                                |
| 1.1        | 2020.1.7  | 田由     | 增加AstProcess函数说明和Cloud189-DoUpload相关业务字段说明                           |
| 1.1.1      | 2020.1.10 | 田由     | 完善AstProcess函数说明和修改Cloud189-DoUpload相关业务字段说明中的是否必传选项和备注 |
| 1.2        | 2020.1.15 | 周启鹏   | 完全增加1.2模块：增加AstConfig函数说明和相关业务字段说明                          |
| 1.2.1      | 2020.1.17 | 周启鹏   | 1.1.6和1.1.8小节相关字段按是否必传分类，便于联调                       |
| 1.2.2      | 2020.1.19 | 周启鹏   | 原1.2.4的预置条件冗余，已清理,1.2.5和1.2.6统一所有字段名为小写字母加下划线的形式，1.1.7小节修改start_result字段为int32类型|
| 1.3        | 2020.2.10 | 周启鹏   | 整合AstProcess原1.1.2-1.1.5到1.1.1中，原1.1.6改为1.1.2，增加AstProcess下1.1.3（DoFolderUpload）相关业务字段说明       |
| 1.3.1      | 2020.2.19 | 周启鹏   | 修正1.1.3.3中parent_folder_id为string类型、增加is_complete为true时parent_folder_id、local_folder_path、sub_file_data字段的说明       |
| 1.4        | 2020.2.19 | 周启鹏   | 完全增加1.1.4模块,增加Cloud189-DoDownload相关业务字段说明       |
| 1.5        | 2020.2.27 | 周启鹏   | 完全增加1.1.5模块,增加Cloud189-DoFolderDownload相关业务字段说明       |
| 1.5.1      | 2020.3.9  | 周启鹏    | 补充所有路径相关字段的备注说明：以“/”结尾      |
| 1.5.2      | 2020.3.13 | 周启鹏    | 删除1.1.4.3中的冗余字段：int32_error_code、succeed和error_code，添加download_folder_path的备注说明  |
| 1.6        | 2020.3.18 | 周启鹏    | 补充完善1.1.1模块对AstProcess通信函数的功能及各个参数的详细说明及流程示例；增加1.1.6暂停模块（UserCancel）相关业务字段说明；版本号修订 |
| 1.6.1      | 2020.3.23 | 周启鹏    |1.1.4.1中增加last_download_breakpoint_data字段及说明；1.1.4.3中增加download_breakpoint_data和int32_error_code字段及说明 |
| 1.7        | 2020.3.28 | 周启鹏    |1.1.2.3和1.1.4.3中增加average_speed字段及其备注说明 ；1.1.2.3中增加commit_file_id的备注|
| 1.7.1       | 2020.3.30 | 周启鹏   |1.1.4.3中增加rename_result,md5_result,stat_result及其备注说明|



# 0 规范介绍

  本文档对天翼云盘Mac版客户端、前端与后端数据交互接口进行定义，包括接口定义，请求参数、响应结果和错误码定义。适用于客户端与平台数据交互、门户数据接口研发和前端UI交互研发人员。

# 1 函数说明

## 1.1 AstProcess

### 1.1.1 概述

+ 此函数是一个混合开发接口通信函数，是文件上传、下载，文件夹上传、下载及暂停各个流程的通用接口函数。

+ 此函数有一个json字符串（process_info），两个回调（on_start和on_callback）共计三个参数。

+ process_info为各个流程需传入的相关业务字段，其中domain和operation为各个流程的必传字段，domain字段区分家庭云和个人云，operation字段区分文件（夹）上传、下载等流程。

+ on_start是为了避免前端和客户端两种语言（javascript和C/C++）的桥接限制导致潜在的资源泄露及野对象或空对象问题而引入的同步回调，起到返回值的作用。

+ on_callback是合并了OnNext和OnComplete两种事件，以“秒”或“目录层级”为单位进行数据流推送的异步回调

### 1.1.1.1 函数原型

```cpp
  void AstProcess(const char *process_info, OnProcessStart on_start,OnProcessCallback on_callback)
```

### 1.1.1.2 参数列表
| **字段名称** | **类型** | **描述**                 | **备注** |
| ------------ | -------- | ------------------------ | -------- |
| process_info | char*    | 传入业务信息的json字符串 |          |
| on_start     | char*    | Start同步回调            |          |
| on_callback  | char*    | callback异步回调         |          |

### 1.1.1.3 process_info必传字段

| **字段名称** | **类型** | **是否必传** | **描述**                                                                                  |
| ------------ | -------- | ------------ | ----------------------------------------------------------------------------------------- |
| domain       | string   | Y            | 以下值之一："Cloud189", "FamilyCloud", "EnterpriseCloud"                                  |
| operation    | string   | Y            | 以下值之一："DoUpload", "UserCancelUpload", "DoDownload", "UserCancelDownload","DoFolderUpload","UserCancelFolderUpload","DoFolderDownload", "UserCancelFolderDownload"           |


### 1.1.1.4 on_start详细说明

+ 由于前端和客户端两种语言(javascript和C/C++)桥接的限制，故通过on_start回调返回一个代表结果的字符串以避免资源泄露及出现野对象或空对象的使用导致潜在问题

+ on_start回调中的domain字段为域标识，区分家庭云（FamilyCloud）和个人云（Cloud189）；
+ on_start回调中的operation字段为操作流程类型标识。共计有8种类型：文件上传、文件夹上传、文件下载、文件夹下载、暂停文件上传、暂停文件夹上传、暂停文件下载、暂停文件夹下载
+ on_start回调中的uuid为native侧自行生成，用来唯一标识各个流程的字段。

### 1.1.1.5 on_callback详细说明

+ OnCallback是合并了OnNext和OnComplete两种事件的异步回调。

+ 文件传输时（上传和下载）OnCallback回调以“秒”为单位每秒推送一次数据流。

+ 文件夹传输时（上传和下载）OnCallback回调以“目录层级”为单位每一级目录推送一次数据流

### 1.1.1.6 流程举例（文件上传）

+ 1——js侧参数传入：1）process_info中operation字段传入“DoUpload”；2）传入其它业务字段（详见1.1.2.1）

+ 2——native侧根据传入参数创建上传流程

+ 3——js侧监听on_start回调，若on_start回调中start_result字段返回0，则表明native侧创建上传流程成功，js侧监听on_callback回调，直到on_callback结束。若on_start回调中start_result字段不为0，表明native侧创建上传流程失败，js侧停止监听on_callback回调。

+ 4——若创建上传流程成功后创建取消上传流程，则仅仅是“请求取消”，不代表真正的“取消成功”。所以依然还需要监听原有上传流程的OnCallback回调

### 1.1.1.7 目录结构说明
| **标号** | **描述**|
| --------------| -------- | ------------ | 
| 1.1.2         | 文件上传（DoUpload）相关业务字段说明|
| 1.1.3         | 文件夹上传（DoFolderUpload）相关业务字段说明|
| 1.1.4         | 文件下载（DoDownload）相关业务字段说明|
| 1.1.5         | 文件夹下载（DoFolderDownload）相关业务字段说明|
| 1.1.6         | 取消文件上传（UserCancelUpload）相关业务字段说明|


### 1.1.2 Cloud189-DoUpload相关业务字段
### 1.1.2.1 DoUpload-process_info

| **字段名称**     | **类型** | **是否必传** | **描述**                                                               | **备注** |
| ---------------- | -------- | ------------ | ---------------------------------------------------------------------- | -------- |
| domain           | string   | Y            | 以下值："Cloud189"                                |
| operation        | string   | Y            | 以下值："DoUpload"            |
| local_path       | string   | Y            | 表明上传文件的本地全路径                                               |          |
| oper_type        | int32    | Y            | 表明上传后操作方式                                                     |          |
|                  |          |              | 1-遇到相同文件名(只检查文件名)，执行重命名操作，                       |          |
|                  |          |              | 3-遇到相同文件名（只检查文件名），执行覆盖原文件                       |          |
| is_log           | int32    | Y            | 表明是否为客户端日志上传                                               |          |
|                  |          |              | 1–客户端日志文件上传至指定账户，                                       |          |
|                  |          |              | 0-非客户端日志文件上传                                                 |          |
| parent_folder_id | string   | Y            | 表明上传文件的父文件夹id                                               |          |
| last_md5         | string   | N            | 表明上传文件的md5，如果为续传则必须传入，如果为创建新的上传可为空      |          |
| last_upload_id   | string   | N            | 表明上传文件的id，如果为续传则必须传入且有效，如果为创建新的上传可为空 |          |
| x_request_id     | string   | N            | 表明请求的x_request_id，如果传入为空则生成                             |          |


### 1.1.2.2 DoUpload-on_start

+ 此处回调为传给JS侧的信息

| **字段名称** | **类型** | **是否必传** | **描述**                                                                                          | **备注**                          |
| ------------ | -------- | ------------ | ------------------------------------------------------------------------------------------------- | --------------------------------- |
| uuid         | string   | Y            | native侧自行生成的流程标识                                           |
| domain       | string   | Y            | 与1.1.2.1传入的一致，以下值："Cloud189"                       |
| operation    | string   | Y            | 与1.1.2.1传入的一致，以下值："DoUpload" |
| start_result | int32    | Y            | 指定的创建流程是否成功                                                                            | 0：成功   非0：失败 |

### 1.1.2.3 DoUpload-on_callback

+ 此处回调为传给JS侧的信息

| **字段名称**       | **类型** | **是否必传** | **描述**                                                                                                                   | **备注**                             |
| ------------------ | -------- | ------------ | -------------------------------------------------------------------------------------------------------------------------- | ------------------------------- |
| uuid               | string   | N            | 与1.1.2.2完全一致                                                    |
| domain             | string   | Y            | 与1.1.2.2完全一致                                         |
| operation          | string   | Y            | 与1.1.2.2完全一致                      |
| x_request_id       | string   | Y            | 对上传总控进行初始化时指定，若不传则和流程中的uuid保持一致                                                         |
| file_size          | int64    | Y            | 文件大小                                                                      | 单位是Bytes                   |
| transferred_size   | int64    | Y            | 已传递数据量；在续传记录有效的情况下，是续传偏移位置+本次有效传输数据量之和         | 单位是Bytes                  
| stage              | int32    | Y            | 上传流程各态，-1，初态；5，终态；0-4上传中状态；0，计算MD5；1，创建上传；2，获取上传状态；3，上传文件数据；4，确认上传完成 |
| md5                | string   | Y            | 表明上传文件的md5，且完成文件MD5计算后，才有此值；计算完成前，传递空字符串                                             |
| upload_id          | string   | Y            | 表明上传文件的上传记录id，如果为续传则完成文件MD5计算后，才有此值；如果是新建续传记录，则建完后才有此值；无则空           |
| is_complete        | bool     | N            | 是否已完成                                                                 | 要么无此字段，若有此字段必为true     |
| int32_error_code   | int32    | N            | 错误码，仅在is_complete传递时，才传递；若已完成且成功，则为0；若非0，则代表最后一次网络请求的errorcode| 总控“完成”（含成功和失败），传此字段 |
| speed              | int64    | N            | 3s内的平滑传输速度|仅在is_complete未传时，才传递；若已完成，则无意义（事实上，在stage==4那一步，就不应显示速度了） | 单位是 X Bytes/s |
| average_speed      | int64    | N            | 全阶段平滑传输速度                               |   仅在stage==3（真正的数据传输阶段）时，才会回调此字段;界面展示的速度必须在speed和这个字段之间，显示较大的值;在stage  != 3的情况下，不应展示xxxKB/s 的文案|
| data_exist         | bool     | N            | 是否秒传；在不同的后端实现中，仅仅此值为ture时，代表文件可秒传，直接提交                                         |
| file_upload_url    | string   | N            | 文件上传url，用于url域名列表                                                                    |
| commit_file_id     | string   | N            | 上传完成后的文件id                                                                    | 上传完成后传;此字段可以辅助检查文件是否上传成功。若commit_id为空，则文件一定在云端文件列表看不到             |
| commit_name        | string   | N            | 上传完成后的云端的文件名                                                               | 上传完成后传          |
| commit_size        | string   | N            | 上传完成后的文件大小                                                                 | 上传完成后传          |
| commit_md5         | string   | N            | 上传完成后的文件md5                                                                 | 上传完成后传            |
| commit_create_date | string   | N            | 上传完成后的文件创建时间                                                            | 上传完成后传            |
| commit_rev         | string   | N            | 上传完成后文件版本号                                                                | 上传完成后传         |
| commit_user_id     | string   | N            | 上传完成后的user_id                                                                 | 上传完成后传                         |

### 1.1.3 Cloud189-DoFolderUpload相关业务字段
### 1.1.3.1 DoFolderUpload-process_info
| **字段名称**     | **类型** | **是否必传** | **描述**                                                               | **备注** |
| ---------------- | -------- | ------------ | ---------------------------------------------------------------------- | -------- |
| domain           | string   | Y            | 以下值："Cloud189"                                |
| operation        | string   | Y            | 以下值："DoFolderUpload"         |
| parent_folder_id | string     | Y            | 表明上传到云端的所在目录                               | 举例，代表用户个人云盘根目录的值，"-11"                                     | 
| local_folder_path       | string   | Y            | 表明上传文件夹的本地全路径 |以"/"结尾|  
| server_folder_path      | string   | Y            | 表明上传到云端目录的路径，以`/`分割且必须以`/`终止  |举例，代表用户个人云盘根目录的值，"/"；代表用户同步盘目录的值，"同步盘/"|

### 1.1.3.2 DoFolderUpload-on_start
+ 此处回调为传给JS侧的信息

| **字段名称** | **类型** | **是否必传** | **描述**                                                                                          | **备注**                          |
| ------------ | -------- | ------------ | ------------------------------------------------------------------------------------------------- | --------------------------------- |
| uuid         | string   | Y            | native侧自行生成的流程标识                                            |
| domain       | string   | Y            | 与1.1.3.1传入的一致                   |
| operation    | string   | Y            | 与1.1.3.1传入的一致 |
| start_result | int32    | Y            | 指定的创建流程是否成功                                                                            | 0：成功   非0：失败 |

### 1.1.3.3 DoFolderUpload-on_callback

+ 此处回调为传给JS侧的信息

| **字段名称**       | **类型** | **是否必传** | **描述**                                                                                                                   | **备注**                             |
| ------------------ | -------- | ------------ | -------------------------------------------------------------------------------------------------------------------------- | ------------------------------- |
| uuid               | string   | N            | 与1.1.3.2完全一致                                                    |
| domain             | string   | Y            | 与1.1.3.2完全一致                                         |
| operation          | string   | Y            | 与1.1.3.2完全一致                      |   
| parent_folder_id   | string   | Y            | 父文件夹id                                                 |is_complete为true时此字段为空（Json::Value::null）                                      | 
| local_folder_path  | string   | Y            | 表明上传文件夹及子目录的本地全路径       |is_complete为true时此字段为空（Json::Value::null）;以"/"结尾|              
| sub_file_data      | json数组 | Y            | 存放文件夹下各个子文件的信息                 |空文件夹传入NULL，数组长度可变，随子文件个数变化 ;is_complete为true时此字段为空（Json::Value::null）            |
| is_complete        | bool     | N            | 是否已完成                                                                 | 要么无此字段，若有此字段必为true     |
| int32_error_code   | int32    | N            | 错误码，仅在is_complete传递时，才传递；若已完成且成功，则为0| 总控“完成”（含成功和失败），传此字段 |

### 1.1.3.3.1 DoFolderUpload-on_callback-sub_file_data相关业务字段
| **字段名称**                  | **类型** | **是否必传** | **描述**                                                     | **备注**                             |
| -----------------------------    | -------- | ------------ | --------------------------------------------------------- | ------------------------------------ |
| local_path                        | string   | Y            | 子文件的路径                                                |以"/"结尾                  |

+ 注意sub_file_data字段为一个json数组，从第1到n个元素为各个子文件的路径，数组长度随子文件个数变化。<br /><br /><br />


### 1.1.4 Cloud189-DoDownload相关业务字段
### 1.1.4.1 DoDownload-process_info
| **字段名称**     | **类型** | **是否必传** | **描述**                                                               | **备注** |
| ---------------- | -------- | ------------ | ---------------------------------------------------------------------- | -------- |
| domain           | string   | Y            | 以下值："Cloud189"                   |
| operation        | string   | Y            | 以下值："DoDownload"                 |
| file_id          | string   | Y            | 文件id                            |                                      
| file_name        | string   | Y            | 文件名                           |
| md5              | string   | Y            | 文件md5值             |用于文件下载成功后，进行MD5校验|
| last_download_breakpoint_data| string   | N | 上次文件下载中断时保存的数据           |续传时必须传入此字段;如果是创建新的下载可为空；此字段是将md5和file_id用算法拼接的base64字符串|
| download_folder_path| string  | Y            | 文件下载到本地的全路径      | 以"/"结尾;传入前保证文件夹是已经存在的，如果不存在，则此次调用必然失败|


### 1.1.4.2 DoDownload-on_start
+ 此处回调为传给JS侧的信息

| **字段名称** | **类型** | **是否必传** | **描述**                                                                                          | **备注**                          |
| ------------ | -------- | ------------ | ------------------------------------------------------------------------------------------------- | --------------------------------- |
| uuid         | string   | Y            | native侧自行生成的流程标识                                            |
| domain       | string   | Y            | 与1.1.4.1传入的一致                   |
| operation    | string   | Y            | 与1.1.4.1传入的一致 |
| start_result | int32    | Y            | 指定的创建流程是否成功                                                                            | 0：成功   非0：失败 |

### 1.1.4.3 DoDownload-on_callback

+ 此处回调为传给JS侧的信息

| **字段名称**       | **类型** | **是否必传** | **描述**                                                                                                                   | **备注**                             |
| ------------------ | -------- | ------------ | -------------------------------------------------------------------------------------------------------------------------- | ------------------------------- |
| uuid               | string   | N            | 与1.1.4.2完全一致                      |
| domain             | string   | Y            | 与1.1.4.2完全一致                      |
| operation          | string   | Y            | 与1.1.4.2完全一致                      |   
| file_size          | int64   | Y            | 文件大小                       |    byte          |         
| transferred_size   | int64    | Y            | 已传递数据量；在续传记录有效的情况下，是续传偏移位置+本次有效传输数据量之和         | 单位是Bytes   
| speed              | int64   | Y            | 3s内的平滑传输速度                               |   经过平滑（B/s）
| average_speed      | int64   | N            | 全阶段平滑传输速度                               |   仅在stage==3（真正的数据传输阶段）时，才会回调此字段;界面展示的速度必须在speed和这个字段之间，显示较大的值;在stage  != 3的情况下，不应展示xxxKB/s 的文案|
| is_complete        | bool     | N            | 是否已完成                             | 要么无此字段，若有此字段必为true     |
| int32_error_code   | int32    | N            | 错误码，仅在is_complete传递时，才传递；若已完成且成功，则为0；若非0，则代表最后一次网络请求的errorcode| 总控“完成”（含成功和失败），传此字段 |
| download_file_path | string   | N            | 重命名后的路径                       |仅在is_complete为true时，才传递|
| stat_result        | int32    | N            | 检测临时文件的路径，是否为有效文件                       |仅在md5校验失败时返回此字段用于问题排查；返回0x8000代表是有效文件；外部不可尝试通过此字段，进行任何逻辑判断|
| rename_result      | bool     | N            | 校验对文件的重命名操作是否成功                           |仅在md5校验失败时返回此字段用于问题排查；重命名失败返回false；外部不可尝试通过此字段，进行任何逻辑判断|
| md5_result         | string   | N            | 对临时文件进行MD5计算的结果                             |仅在md5校验失败时返回此字段用于问题排查；返回文件的md5与传入的md5对照；外部不可尝试通过此字段，进行任何逻辑判断|
| download_breakpoint_data| string | N | 保存文件下载中断时的数据           |此字段是将md5和file_id用算法拼接的base64字符串，下载中断时返回此字段用于下次续传 |
### 1.1.5 Cloud189-DoFolderDownload相关业务字段
### 1.1.5.1 DoFolderDownload-process_info
| **字段名称**     | **类型** | **是否必传** | **描述**                                                               | **备注** |
| ---------------- | -------- | ------------ | ---------------------------------------------------------------------- | -------- |
| domain           | string   | Y            | 以下值："Cloud189"                                |
| operation        | string   | Y            | 以下值："DoFolderDownload"         |
| folder_id        | string   | Y            | 云端文件夹id                    |  必传字段且不可直接下载云端的根目录（即不能为：-11）|             | 
| folder_path      | string   | Y            | 云端文件夹路径                  |  必传字段且不可直接下载云端的根目录（即不能为："/")|  
| download_path    | string   | Y            | 文件夹下载到本地的全路径              |  不包括远端文件夹的名称， 以"/"结尾 ;传入前保证文件夹是已经存在的，如果不存在，则此次调用必然失败     |          |        


### 1.1.5.2 DoFolderDownload-on_start
+ 此处回调为传给JS侧的信息

| **字段名称** | **类型** | **是否必传** | **描述**                                                                                          | **备注**                          |
| ------------ | -------- | ------------ | ------------------------------------------------------------------------------------------------- | --------------------------------- |
| uuid         | string   | Y            | native侧自行生成的流程标识                                            |
| domain       | string   | Y            | 与1.1.5.1传入的一致                   |
| operation    | string   | Y            | 与1.1.5.1传入的一致 |
| start_result | int32    | Y            | 指定的创建流程是否成功                                                                            | 0：成功   非0：失败 |

### 1.1.5.3 DoFolderDownload-on_callback

+ 此处回调为传给JS侧的信息

| **字段名称**       | **类型** | **是否必传** | **描述**                                                                                                                   | **备注**                             |
| ------------------ | -------- | ------------ | -------------------------------------------------------------------------------------------------------------------------- | ------------------------------- |
| uuid               | string   | N            | 与1.1.5.2完全一致                                                    |
| domain             | string   | Y            | 与1.1.5.2完全一致                                         |
| operation          | string   | Y            | 与1.1.5.2完全一致                      |         |
| download_folder_path| string  | Y            | 文件夹及子目录下载到本地的全路径      | 以"/"结尾;传入前保证文件夹是已经存在的，如果不存在，则此次调用必然失败|               
| sub_file_data      | json数组 | Y            | 存放云端文件夹下各个子文件的信息                 |空文件夹传入NULL，数组长度可变，随子文件个数变化 ;is_complete为true时此字段为空（Json::Value::null）            |
| is_complete        | bool     | N            | 是否已完成                                                                 | 要么无此字段，若有此字段必为true     |
| int32_error_code   | int32    | N            | 错误码，仅在is_complete传递时，才传递；若已完成且成功，则为0| 总控“完成”（含成功和失败），传此字段 |

### 1.1.5.3.1 DoFolderDownload-on_callback-sub_file_data相关业务字段
| **字段名称**                  | **类型** | **是否必传** | **描述**                                                     | **备注**                             |
| -----------------------------    | -------- | ------------ | --------------------------------------------------------- | ------------------------------------ |
| file_id          | string   | Y            | 文件id                            |                                      
| file_name        | string   | Y            | 文件名                           |
| md5              | string   | Y            | 文件md5值             |用于文件下载成功后，进行MD5校验|

### 1.1.6 Cloud189-UserCancel相关业务字段
### 1.1.6.1 UserCancel-process_info
| **字段名称**     | **类型** | **是否必传** | **描述**                                                               | **备注** |
| ---------------- | -------- | ------------ | ---------------------------------------------------------------------- | -------- |
| domain           | string   | Y            | 以下值："Cloud189"                   |
| operation        | string   | Y            | 暂停流程标识，以下值之一："UserCancelUpload", "UserCancelDownload","UserCancelFolderUpload","UserCancelFolderDownload"                    |
| uuid             | string   | Y            | 需暂停流程的标识 |必传字段|

### 1.1.6.2 UserCancel-on_start
+ 此处回调为传给JS侧的信息

| **字段名称** | **类型** | **是否必传** | **描述**                                                                                          | **备注**                          |
| ------------ | -------- | ------------ | ------------------------------------------------------------------------------------------------- | --------------------------------- |
| uuid         | string   | Y            | 与1.1.6.1传入的一致                                  |
| domain       | string   | Y            | 与1.1.6.1传入的一致                   |
| operation    | string   | Y            | 与1.1.6.1传入的一致 |
| start_result | int32    | Y            | 创建指定流程是否成功  | 0：成功   非0：失败; 取消仅仅是“请求取消”，不代表真正的“取消成功”。所以仍需处理原有流程的OnCallback回调|
+ 注：1.1.6为暂停文件上传、暂停文件下载、暂停文件夹上传、暂停文件夹下载四个流程的整合，通过operation字段区分；暂停流程不需要传入on_callback回调。

## 1.2 AstConfig

### 1.2.1 函数原型

```cpp
  void AstConfig(const char *json_str,OnConfigFinished on_config_finished)
```

### 1.2.2 功能描述

  提供一个通用信息配置接口

### 1.2.3 返回值

```cpp
  void
```

### 1.2.4 参数列表

| **字段名称**       | **类型** | **描述**                     | **备注** |
| ------------------ | -------- | ---------------------------- | -------- |
| json_str           | char*    | 传入通用配置信息的json字符串   |          |
| on_config_finished | char*    | config_finished同步回调                 |          |

### 1.2.5 Cloud189-json_str相关业务字段

| **字段名称**                 | **类型** | **是否必传** | **描述**                                            | **备注**                         |
| ---------------------------- | -------- | ------------ | --------------------------------------------------- | -------------------------------- |
| save_cloud189_session        | json数组 | N            | 保存个人云cloud189_session_key、cloud189_session_secret、家庭云cloud189_session_key_family、cloud189_session_kecret_family       |
| clear_cloud189_session       | string   | N            | 要么无此字段：若有必须传值"logout"                                                       |
| save_enterprisecloud_session | json数组 | N            | 分别保存企业云enterprisecloud_session_key、enterprisecloud_session_secret                |
| clear_enterprisecloud_session| string   | N            | 要么无此字段：若有必须传值"logout_enterprise"                                             |
| proxy_info                   | string   | N            | 默认不走正向代理  |
+ 注意json风格，字段名全部小写加下划线，特例是部分从“业务流程”复制过来的字段，与原字段名保持一致即可
+ 同样注意前后一致，有些字段跨流程出现，应使用“一致”的字段名，避免来来回回产生歧义
+ 
### 1.2.6 Cloud189-on_config_finished相关业务字段
+ 此处回调为传给JS侧的信息

| **字段名称**                  | **类型** | **是否必传** | **描述**                                                                   | **备注**                             |
| -----------------------------    | -------- | ------------ | ---------------------------------------------------------------------- | ------------------------------------ |
| cloud189_session_key             | string   | N            | 个人云session_key                                                  |             |
| cloud189_session_secret          | string   | N            | 个人云session_secret                                               |             |
| cloud189_session_key_family      | string   | N            | 家庭云session_key_family                                           |             |
| cloud189_session_kecret_family   | string   | N            | 家庭云session_kecret_family                                        |             |
| enterprisecloud_session_key      | string   | N            | 企业云session_key                                                  |             |
| enterprisecloud_session_secret   | string   | N            | 企业云session_secret                                               |              |
| logout                           | string   | N            | 个人云和家庭云对当前用户登录信息进行清理的标识                        |                |
| logout_enterprise                | string   | N            | 企业云对当前用户登录信息进行清理的标识                               |               |
| proxy_info                       | string   | N            | 默认不走正向代理                                                   |               |     
