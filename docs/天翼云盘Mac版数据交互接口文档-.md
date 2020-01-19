# 天翼云盘Mac版数据交互接口文档

**版权所有  不得翻印  内部资料**

**Version 0.2**

**修订历史：**

| **版本号** | **时间**  | **作者** | **内容**                                                                            |
| ---------- | --------- | -------- | ----------------------------------------------------------------------------------- |
| 0.1        | 2020.1.7  | 田由     | 创建                                                                                |
| 0.1        | 2020.1.7  | 田由     | 增加AstProcess函数说明和Cloud189-DoUpload相关业务字段说明                           |
| 0.1.1      | 2020.1.10 | 田由     | 完善AstProcess函数说明和修改Cloud189-DoUpload相关业务字段说明中的是否必传选项和备注 |
| 0.2        | 2020.1.15 | 周启鹏   | 完全增加1.2模块：增加AstConfig函数说明和相关业务字段说明                          |
| 0.2.1      | 2020.1.17 | 周启鹏   | 1.1.6和1.1.8小节相关字段按是否必传分类，便于联调                       |
| 0.2.2      | 2020.1.19 | 周启鹏   | 原1.2.4的预置条件冗余，已清理,1.2.5和1.2.6统一所有字段名为小写字母加下划线的形式，1.1.7小节修改start_result字段为int32类型|

# 0 规范介绍

  本文档对天翼云盘Mac版客户端、前端与后端数据交互接口进行定义，包括接口定义，请求参数、响应结果和错误码定义。适用于客户端与平台数据交互、门户数据接口研发和前端UI交互研发人员。

# 1 函数说明

## 1.1 AstProcess

### 1.1.1 函数原型

```cpp
  void AstProcess(const char *process_info, OnProcessStart on_start,OnProcessCallback on_callback)
```

### 1.1.2 功能描述

  提供一个混合开发接口通信函数
  
### 1.1.3 预置条件

| **字段名称** | **类型** | **是否必传** | **描述**                                                                                  |
| ------------ | -------- | ------------ | ----------------------------------------------------------------------------------------- |
| domain       | string   | Y            | 以下值之一："Cloud189", "FamilyCloud", "EnterpriseCloud"                                  |
| operation    | string   | Y            | 以下值之一："DoUpload", "UserCancelUpload", "DoDownload", "UserCancelDownload"            |
| uuid         | string   | N            | 可以不传，此字段若不传，或传了一个重复的视为无效，Start回调内的uuid字段将是自行生成的uuid |

+ 注意json风格，字段名全部小写加下划线，特例是部分从“业务流程”复制过来的字段，与原字段名保持一致即可
+ 同样注意前后一致，有些字段跨流程出现，应使用“一致”的字段名，避免来来回回产生歧义

### 1.1.4 返回值

```cpp
  void
```

### 1.1.5 参数列表

| **字段名称** | **类型** | **描述**                 | **备注** |
| ------------ | -------- | ------------------------ | -------- |
| process_info | char*    | 传入业务信息的json字符串 |          |
| on_start     | char*    | Start同步回调            |          |
| on_callback  | char*    | callback异步回调         |          |

### 1.1.6 Cloud189-DoUpload-process_info相关业务字段

| **字段名称**     | **类型** | **是否必传** | **描述**                                                               | **备注** |
| ---------------- | -------- | ------------ | ---------------------------------------------------------------------- | -------- |
| domain           | string   | Y            | 以下值之一："Cloud189", "FamilyCloud", "EnterpriseCloud"                                  |
| operation        | string   | Y            | 以下值之一："DoUpload", "UserCancelUpload", "DoDownload", "UserCancelDownload"            |
| local_path       | string   | Y            | 表明上传文件的本地全路径                                               |          |
| oper_type        | int32    | Y            | 表明上传后操作方式                                                     |          |
|                  |          |              | 1-遇到相同文件名(只检查文件名)，执行重命名操作，                       |          |
|                  |          |              | 3-遇到相同文件名（只检查文件名），执行覆盖原文件                       |          |
| is_log           | int32    | Y            | 表明是否为客户端日志上传                                               |          |
|                  |          |              | 1–客户端日志文件上传至指定账户，                                       |          |
|                  |          |              | 0-非客户端日志文件上传                                                 |          |
| parent_folder_id | string   | Y            | 表明上传文件的父文件夹id                                               |          |
| uuid             | string   | N            | 可以不传，此字段若不传，或传了一个重复的视为无效，Start回调内的uuid字段将是自行生成的uuid |
| last_md5         | string   | N            | 表明上传文件的md5，如果为续传则必须传入，如果为创建新的上传可为空      |          |
| last_upload_id   | string   | N            | 表明上传文件的id，如果为续传则必须传入且有效，如果为创建新的上传可为空 |          |
| x_request_id     | string   | N            | 表明请求的x_request_id，如果传入为空则生成                             |          |


### 1.1.7 Cloud189-DoUpload-on_start相关业务字段

+ 此处回调为传给JS侧的信息

| **字段名称** | **类型** | **是否必传** | **描述**                                                                                          | **备注**                          |
| ------------ | -------- | ------------ | ------------------------------------------------------------------------------------------------- | --------------------------------- |
| uuid         | string   | Y            | 与1.1.5传入的一致，若传入的为空或无效，将是自行生成的uuid                                           |
| domain       | string   | Y            | 与1.1.5传入的一致，以下值之一："Cloud189", "FamilyCloud", "EnterpriseCloud"                       |
| operation    | string   | Y            | 与1.1.5传入的一致，以下值之一："DoUpload", "UserCancelUpload", "DoDownload", "UserCancelDownload" |
| start_result | int32    | Y            | 指定的创建流程是否成功                                                                            | 在传入字段非法情况下，可能为false |

### 1.1.8 Cloud189-DoUpload-on_callback相关业务字段

+ 此处回调为传给JS侧的信息

| **字段名称**       | **类型** | **是否必传** | **描述**                                                                                                                   | **备注**                             |
| ------------------ | -------- | ------------ | -------------------------------------------------------------------------------------------------------------------------- | ------------------------------- |
| uuid               | string   | N            | 与1.1.5传入的一致，若传入的为空或无效，将是自行生成的uuid                                                    |
| domain             | string   | Y            | 与1.1.5传入的一致，以下值之一："Cloud189", "FamilyCloud", "EnterpriseCloud"                                      |
| operation          | string   | Y            | 与1.1.5传入的一致，以下值之一："DoUpload", "UserCancelUpload", "DoDownload", "UserCancelDownload"                   |
| x_request_id       | string   | Y            | 对上传总控进行初始化时指定，若不传则和流程中的uuid保持一致                                                         |
| file_size          | int64    | Y            | 文件大小                                                                      | 单位是Bytes                   |
| transferred_size   | int64    | Y            | 已传递数据量；在续传记录有效的情况下，是续传偏移位置+本次有效传输数据量之和         | 单位是Bytes                  
| stage              | int32    | Y            | 上传流程各态，-1，初态；5，终态；0-4上传中状态；0，计算MD5；1，创建上传；2，获取上传状态；3，上传文件数据；4，确认上传完成 |
| md5                | string   | Y            | 表明上传文件的md5，且完成文件MD5计算后，才有此值；计算完成前，传递空字符串                                             |
| upload_id          | string   | Y            | 表明上传文件的上传记录id，如果为续传则完成文件MD5计算后，才有此值；如果是新建续传记录，则建完后才有此值；无则空           |
| is_complete        | bool     | N            | 是否已完成                                                                 | 要么无此字段，若有此字段必为true     |
| int32_error_code   | int32    | N            | 错误码，仅在is_complete传递时，才传递；若已完成且成功，则为0；若非0，则代表最后一次网络请求的errorcode| 总控“完成”（含成功和失败），传此字段 |
| speed              | int64    | N            | 速度，仅在is_complete未传时，才传递；若已完成，则无意义（事实上，在stage==4那一步，就不应显示速度了） | 单位是 X Bytes/s |
| data_exist         | bool     | N            | 是否秒传；在不同的后端实现中，仅仅此值为ture时，代表文件可秒传，直接提交                                         |
| file_upload_url    | string   | N            | 文件上传url，用于url域名列表                                                                    |
| commit_file_id     | string   | N            | 上传完成后的文件id                                                                    | 上传完成后传              |
| commit_name        | string   | N            | 上传完成后的云端的文件名                                                               | 上传完成后传          |
| commit_size        | string   | N            | 上传完成后的文件大小                                                                 | 上传完成后传          |
| commit_md5         | string   | N            | 上传完成后的文件md5                                                                 | 上传完成后传            |
| commit_create_date | string   | N            | 上传完成后的文件创建时间                                                            | 上传完成后传            |
| commit_rev         | string   | N            | 上传完成后文件版本号                                                                | 上传完成后传         |
| commit_user_id     | string   | N            | 上传完成后的user_id                                                                 | 上传完成后传                         |

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

### 1.2.5 Cloud189-DoUpload-json_str相关业务字段

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
### 1.2.6 Cloud189-DoUpload-on_config_finished相关业务字段
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