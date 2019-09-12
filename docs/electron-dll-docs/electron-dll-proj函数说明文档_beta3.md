### 函数说明
------------
- 生成Ast对象 
  
  `CreateAst <int64_t(char*)>`
  - 输入：session字符串
  - 返回值：Ast对象句柄
    > 即int64_t 类型对象
  
- 开始下载
  
  `DoDownload <char*(char*, OnNext, OnComplete, int64_t)>`
  - 输入：1. 文件信息字符串，2. OnNext回调，3. OnComplete回调，4. ast对象句柄
  - 返回值：文件下载生成的uuid字符串

- 启动分片下载流程
  
  `StartSliceDownload <char*(char*, char*, int64_t)>`
  - 输入：1. 下载链接，2. 下载路径, 3. Ast对象句柄
    > 必须保证Ast对象句柄有效（当前进程，且未被销毁），输入的字符串的编码应为utf-8编码
  - 返回值：此下载流程的uuid字符串

- 注册下载回调【DoDownload流程，无需再次调用此ABI】
  
  `RegisterSliceDownloadSubscription <char*(OnNext, OnComplete, char*, int64_t)>`
  - 输入：1. OnNext回调，2. OnComplete回调，3. 下载流程的uuid字符串，4. Ast对象句柄
  - 返回值：此订阅的uuid字符串

- 取消回调订阅【DoDownload流程，不适合调用此ABI】
  
  `CancelSubscription <bool(char*, int64_t)>`
  > 取消回调订阅之后该任务对应的registerid会被释放
  - 输入：1. 订阅的uuid字符串，2. Ast对象句柄
  - 返回值：取消是否成功
 
- 停止下载流程
  
  `StopDownload <void(char*, int64_t)>`
  - 输入：1. 下载流程的uuid字符串，2. Ast对象句柄
  - 返回值：空

- 获取下载结束状态
  
  `GetFinalResult <void(OnFinal，char*, int64_t)>`
  - 输入：1. OnFinal回调，2. 下载uuid字符串， 3. Ast对象句柄
  - 返回值：空

- 校验下载文件完整性
  
  `CheckDLFile <void (char*, OnCheck)>`
  - 输入：1.下载文件信息json字符串， 2.OnCheck回调
  - 返回值：空
 
- 销毁Ast对象
  
  `DestroyAst <void(int64_t)>`
  - 输入：Ast对象句柄
  - 返回值：空

- 推送流程信息，每秒一次
  > Node.js侧回调

  `OnNext <void(char*)>`
   - 入参为json字符串，含有进度、速度等信息
   - 返回值：空
  
- 下载流程结束
  > Node.js侧回调

  `OnComplete <void(void)>`
  > （未主动取消订阅的情况下）此回调必被调用且仅被调用一次；此回调后，C++侧不会再次调用此订阅对应的回调方法

- 获取下载结束状态回调
  > node.js侧回调

  `OnFinal <void(char*)>`
  - 输入：最终状态json字符串
  - 返回值：空

- 校验下载文件完整性回调
  > node.js侧回调

  `OnCheck <void(char*)>`
  > 如果校验不成功，但是文件大小一致，需要重新下载
  - 输入：文件校验信息json字符串
  - 返回值：空

 # session字符串
   | **字段**  | **类型**   | **详细描述**   |
| ---------------------------------- | ------------------------------ | ---------------------------------------------- |
|sessionKey |string |获取的session key |
|sessionSecret |string |获取的session secret |
|veision |string |版本信息, e.g.: version:2.2.0.0 |

 # 下载文件信息字符串
  | **字段**  | **类型**   | **详细描述**   |
| ---------------------------------- | ------------------------------ | ---------------------------------------------- |
|fileId |long |文件id|
|dt     |long |下载类型<br>**1** - 企业空间文件<br>**2** - 协作文件夹内文件<br>**3** - 工作空间文件|
| corpId       | Long   | 企业ID    |
| coshareId    | Long   | 共享ID   dt 为2时要传 |
| isPreview    | Long     | 是否用于预览<br>**0或空** - 下载功能<br>**1** - 预览功能 |
|downloadFilePath_temp | string |文件临时下载路径 |
|slicedownload_state |string |分片下载暂停状态（json字符串） |

 # onnext 下载回调json字符串
 | **字段**  | **类型**   | **详细描述**   |
| ---------------------------------- | ------------------------------ | ---------------------------------------------- |
| total_length    | uint64_t  | 待传输文件总长度（字节）  |
| completed_length  | uint64_t   | 已完成长度（字节）  |
| smooth_speed   | uint64_t   | 传输速度（经过平滑，字节/秒）  |
| in_progress  | bool   | 传输进行中(true或false)  |

  # onfinal 下载结束状态回调json字符串
  | **字段**  | **类型**   | **详细描述**   |
| ---------------------------------- | ------------------------------ | ---------------------------------------------- |
|succeed |bool |下载是否成功 |
|error_code |string |返回错误码 |
|slicedownload_state |string |分片下载暂停状态（json字符串） |
|pdf_md5 |string |水印下载文件添加水印后的md5 |


  # 待校验下载文件信息json字符串
  | **字段**  | **类型**   | **详细描述**   |
| ---------------------------------- | ------------------------------ | ---------------------------------------------- |
|md5 |string |该文件云端md5值 |
|downloadFilePath_temp |string |临时下载路径 |
|downloadFilePath |string |用户设置文件下载路径 |

  # oncheck 校验结果回调json字符串
  | **字段**  | **类型**   | **详细描述**   |
| ---------------------------------- | ------------------------------ | ---------------------------------------------- |
|succeed |bool |文件校验是否成功 |
|downloadFilePath |string |重命名后的路径 |
|file_length |uint64_t |文件大小 |

 # 错误码
 
 | **错误码**                         | **错误信息**                   | **详细描述**                                   |
| ---------------------------------- | ------------------------------ | ---------------------------------------------- |
| InvalidSessionKey                  | 登录超时                       | 登录过期或已退出                               |
| InvalidArgument                    | 参数错误                       | 输入的参数错误                                 |
| FileAlreadyExists                  | 存在同名文件夹                 | 在同一目录下已存在同名文件夹                   |
| ParentNotFolder                    | 上一级目录已被删除             | 上一级目录已被删除                             |
| ShareFileNotEnoughSpaceError       | 剩余空间不足                   | 剩余空间不足                                   |
| ParentOrSubFileBOHasShareFileError | 文件夹已经属于一个群空间文件夹 | 此文件夹已经属于一个群空间文件夹，无法单独设置 |
| SubFileBOHasShareFileError         | 文件夹内已经包含了群空间文件夹 | 此文件夹内已经包含了群空间文件夹，无法设置     |
| ShareSpecialDirError               | 共享特殊目录错误               | 共享特殊目录错误                               |
| ShareFileOverLimitError            | 设置的群空间已达到上限         | 设置的群空间已达到上限                         |
| ShareFileUserOverLoadError         | 群空间成员数量超出限制         | 设置的群空间成员数量超出限制                   |
| PartialFailure                     | 部分失败                       | 部分操作失败                                   |
| ShareFileFindError                 | 群空间不存在                   | 群空间不存在                                   |
| ShareFileOverLimitError5           | 创建群空间超出套餐限制         | 创建群空间超出套餐限制                         |
| NoSuchUser                         | 无法获取用户信息               | 无法获取用户信息                               |
| TimeOut                            | 超时错误                       | 操作超时                                       |
| InternalError                      | 内部错误                       | 程序内部错误                                   |
| CorpUserNotBelongCorp              | 用户不属于该企业               | 用户还未加入企业或用户被删除                   |
| CorpAuthTimeExpire                 | 企业云盘服务已到期             | 企业云盘服务已到期                             |
| CorpSecuAuditNotPass               | 信安检查不通过                 | 信安检查不通过                                 |
| CorpCoshareIsExist                 | 共享文件夹已经存在             | 共享文件夹已经存在                             |
| CorpUserNotInCoshare               | 用户不在共享文件夹             | 用户不在共享文件夹                             |
| CorpAuthNoSetRR                    | 无设置权限                     | 无设置权限                                     |
| CorpAuthNoCopyR                    | 无复制权限                     | 无复制权限                                     |
| CorpAuthNoUploadR                  | 无上传权限                     | 无上传权限                                     |
| CorpAuthNoCreateFolderR            | 无创建文件夹权限               | 无创建文件夹权限                               |
| CorpAuthNoDownloadR                | 无下载权限                     | 无下载权限                                     |
| CorpAuthNoShareR                   | 无分享权限                     | 无分享权限                                     |
| CorpAuthNoSaveR                    | 无转存权限                     | 无转存权限                                     |
| CorpAuthNoMoveR                    | 无移动权限                     | 无移动权限                                     |
| CorpAuthNoRenameR                  | 无重命名权限                   | 无重命名权限                                   |
| CorpAuthNoDelR                     | 无删除权限                     | 无删除权限                                     |
| CorpAuthNoViewR                    | 无预览权限                     | 无预览权限                                     |
| CorpAuthNoOnlineR                  | 无在线编辑权限                 | 无在线编辑权限                                 |
| CorpFileNotInWork                  | 文件不在工作空间               | 文件不在工作空间                               |
| CorpWorkSpaceNotEnoughSpace        | 工作空间不足                   | 工作空间不足                                   |
| ShareFoldersAreNotSupport          | 不支持分享文件夹               | 不支持分享文件夹                               |
| CorpUserShareNotFound              | 外链不存在                     | 外链不存在                                     |
| CorpUserShareInvalid               | 外链已失效                     | 外链已失效                                     |
| CorpUserShareAuditUncheck          | 外链未审核                     | 外链未审核                                     |
| CorpUserShareAuditFail             | 外链审核失败                   | 外链审核失败                                   |
| CorpUserShareNoPermission          | 无外链权限                     | 无外链权限                                     |
| CorpUserShareFileNotFount          | 外链分享文件不存在             | 外链分享文件不存在                             |
| CorpUserShareCannotSaveFile        | 外链不允许转存                 | 外链不允许转存                                 |
| CorpUserShareAccessCodeWrong       | 外链访问码错误                 | 外链访问码错误                                 |
| CorpUserShareAccessCodeIlligal     | 外链访问码非法                 | 外链访问码非法                                 |
| CorpUserShareExpireDatePassed      | 外链有效期已过                 | 外链有效期已过                                 |
| CorpUserShareAccessTimeLimit       | 外链次数达到上限               | 外链次数达到上限                               |
| CorpUserShareCreateLimit           | 单位时间内创建外链次数达到上限 | 单位时间内创建外链次数达到上限                 |
| CorpUserShareCreateDayLimit        | 一天创建外链次数达到上限       | 一天创建外链次数达到上限                       |
| CorpUserHadLocked                  | 企业用户已被锁定               | 企业用户已被锁定                               |
| DownloadFileModified | 本地文件和云端不一致 | 文件下载大小校验不成功
| LocalDiskNotEnough | 本地磁盘不足 | 本地磁盘不足
| CreateFileError | 602 | 创建文件对象错误 |
| FileAlreadyExists| 400 | operType=2时，检查到文件已经存在 |
| FileMD5VerifyFailed | 602 | 文件MD5校验错误 |
| FileTooLarge | 400 | 单文件太大 |
| InfoSecurityErrorCode | 400 | 信安检查异常 |
| InputStreamReadError | 600 | 文件输入流错误 |
| InsufficientStorageSpace | 400 | 空间不足 |
| InvalidArgument | 400 | 错误 |
| InvalidParamError | 400 | 非法参数 |
| InvalidParentFolder | 400 | 上传父目录找不到或者错误 |
| InvalidSessionKey | 400 | 非法sessionKey |
| InvalidUploadFileStatus | 602 | 文件上传状态错误 |
| PermissionDenied | 400 | 拒绝访问 |
| UploadFileAccessViolation | 601 | 保护状态 |
| UploadFileIdVerifyFailed | 602 | 上传ID不存在获取错误 |
| UploadFileNotFound | 602 | 上传记录不存在 |
| UploadFileSaveFailed | 602 | 文件保存错误，文件大小和上传大小不一致 |
| UploadFileStatusVerifyFailed | 602 | 文件上传状态错误 |
| UploadFileVerifyFailed | 602 | 上传大小和实际大小不一致 |
| UploadToSwiftError | 400 | 写swift错误 |
| InfoSecurityErrorCode | 400 | MD5黑名单文件 |
| GetUploadUrlFailed | 400 | 获取上传地址错误 |
| CommonInvalidParam | 400 | 参数无效， 底层抛出 |
| CorpFileNotInCompanyFile | 400 | 不存在公司文件夹 |
| CorpCoshareNotFound | 400 | 不存在共享文件夹 |
| CorpUserNotBelongCorp | 400 | 用户不属于该企业 |
