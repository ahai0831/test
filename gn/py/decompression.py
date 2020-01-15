# coding=UTF-8
import socket
import time
import sys
import zipfile
import tarfile


# 解压zip
def un_zip(file_name, des_dir):
  """unzip zip file"""
  zip_file = zipfile.ZipFile(file_name)
  for names in zip_file.namelist():
    zip_file.extract(names, des_dir + '/')
  zip_file.close()


# 解压tar
def un_tar(file_name, des_dir):
  # untar zip file"""
  tar = tarfile.open(file_name)
  names = tar.getnames()
  for name in names:
    tar.extract(name, des_dir + '/')
  tar.close()


# 脚本单端口检查
# def ApplicationInstance():
#   try:
#     global s
#     s = socket.socket()
#     host = socket.gethostname()
#     print(host)
#     s.bind((host, 78888))
#   except:
#     print('instance is running...')

if __name__ == '__main__':
  import os
  import sys
  import platform
  import shutil
  # ApplicationInstance()
  # while True:
  if (platform.system() == 'Windows' or platform.system() == 'Darwin'):
    # Mac需要配置7z
    # elif (platform.system() == 'Darwin'):
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
      # zip_path = str(sys.argv[1]).replace('../..', py_path_retval)
      # zip_dec_path = str(sys.argv[2]).replace('../..', py_path_retval)
      # if platform.system() == 'Windows':
      #   zip_path = zip_path.replace('/', '\\')
      #   zip_dec_path = zip_dec_path.replace('/', '\\')
      #   pass
      print('PyFolder_path:' + py_path)
      os.system('echo ' + 'WaitUzipPath:' + zip_path + ' UzipToPath:' +
                zip_dec_path)
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
      if os.system('7z') != 0:
        print('Without 7z command')
        # windows 7z失效就走回原来的方式
        if platform.system() == 'Windows':
          abs_module_path = os.path.abspath(__file__)
          abs_module_dir = abs_module_path[:abs_module_path.rfind("\\")] + "\\"
          target_bat_file = os.path.join(
              abs_module_dir,
              '..\\..\\vcproj\\batch-scripts\\decompression.bat')
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
        pass
      else:
        with os.popen(shell_line) as a:
          text = a.read()
          print(text)
          if text.rfind('Everything is Ok') != -1:
            os.system('echo ' + 'Uzip successed')
          else:
            os.system('echo ' + 'Uzip failed')
          sys.exit(0)
        pass
    else:
      os.system('echo ' + 'Fail:Args less')
      sys.exit(0)
      pass
  else:
    print('Other system')
    sys.exit(0)
else:
  sys.exit(0)