# coding=UTF-8

# This is a temp impl. Should be imporved by xvrzh.
if __name__ == '__main__':
  import os
  import sys
  import platform
  import shutil

  if (platform.system() == 'Windows'):
    abs_module_path = os.path.abspath(__file__)
    abs_module_dir = abs_module_path[:abs_module_path.rfind("\\")] + "\\"
    target_bat_file = os.path.join(
        abs_module_dir, '..\\..\\vcproj\\batch-scripts\\decompression.bat')

    zip_path = ''
    zip_dec_path = ''
    if (len(sys.argv) > 2):
      zip_path = str(sys.argv[1])
      zip_dec_path = str(sys.argv[2])
      cmd_line = 'call cmd /C \"'+target_bat_file + \
          ' ' + zip_path + ' ' + zip_dec_path+'\"'
      print(cmd_line)
      a = os.popen(cmd_line)
      text = a.read()
      print(text)
  # Mac需要配置7z
  elif (platform.system() == 'Darwin'):
    abs_module_path = os.path.abspath(__file__)
    # abs_module_dir = abs_module_path[:abs_module_path.rfind("\\")] + "\\"
    print('CurrentPy_Path:' + abs_module_path)
    # print(abs_module_dir)
    # 待解压文件路径
    zip_path = ''
    # 解压到文件夹路径
    zip_dec_path = ''
    # force强制进行重新解压，不设置存在文件夹就不重新解压
    zip_type = ''
    # 命令行
    shell_line = ''
    # 7z解压模式
    zip_ao = ' -aos'
    # py脚本路径
    py_path = ''
    if platform.system() == 'Darwin':
      py_path = str(abs_module_path[:abs_module_path.rfind("/")])
      pass
    if platform.system() == 'Windows':
      py_path = str(abs_module_path[:abs_module_path.rfind("\\")])
      pass
    py_path_retval = str(os.getcwd())
    print('Work_Path:' + py_path_retval)
    # 将当前终端位置定位到py目录
    os.chdir(py_path)
    py_path_retval = str(os.getcwd())
    print('CurrentWork_Path:' + py_path_retval)
    if (len(sys.argv) > 2):
      zip_path = str(sys.argv[1])
      zip_dec_path = str(sys.argv[2])
      os.system('echo ' + 'fils:' + zip_path + 'dir:' + zip_dec_path)
      if (len(sys.argv) > 3):
        zip_type = sys.argv[3]
        pass
      if (os.path.exists(zip_path) == False):
        # 文件不存在会终止程序执行
        os.system('echo ' + 'Fail:WaitUzip empty')
        exit()
        pass
      #如果文件夹不存在，先创建文件夹
      if (os.path.exists(zip_dec_path) == False):
        os.makedirs(zip_dec_path)
        pass
      if (zip_type == 'force'):
        zip_ao = ' -aoa'
        pass

      # 使用python解压
      # if (zip_path.rfind('.tar.gz') != -1 or zip_path.rfind('.tar.xz') != -1):
      #   un_tar(zip_path, zip_dec_path)
      #   pass
      # else:
      #   un_zip(zip_path, zip_dec_path)
      #   pass
      # sys.exit(0)

      shell_line = '7z x ' + '"' + zip_path + '"' + zip_ao + ' -o' + '"' + zip_dec_path + '"'
      # .tar.gz和.tar.xz需要二次解压
      if (zip_path.rfind('.tar.gz') != -1 or zip_path.rfind('.tar.xz') != -1):
        shell_line = '7z x ' + '"' + zip_path + '"' + ' -so' + ' | ' + '7z x ' + '-si -ttar' + zip_ao + ' -o' + '"' + zip_dec_path + '"'
        pass
      print('UzipCommand:' + shell_line)
      os.chdir(py_path)
      with os.popen(shell_line) as a:
        text = a.read()
        print(text)
        if text.rfind('Everything is Ok') != -1:
          os.system('echo ' + 'Uzip successed')
        else:
          os.system('echo ' + 'Uzip failed')
          sys.exit(0)
      print('cmd order:' + shell_line)
      with os.popen(shell_line) as a:
        text = a.read()
        # if len(text) == 0:
        # os.system('echo ' + '解压失败')
        # for line in text:
        # os.system('echo ' + line)
        if text.rfind('Everything is Ok') != -1:
          os.system('echo ' + 'Extract success')
        else:
          os.system('echo ' + 'Extract failed')
    # print('MacOS uZip')
  else:
    print('Unsupport platform.')
