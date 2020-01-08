# coding=UTF-8
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
    abs_module_dir = abs_module_path[:abs_module_path.rfind("\\")] + "\\"
    print(abs_module_dir)
    # 待解压文件路径
    zip_path = ''
    # 解压到文件夹路径
    zip_dec_path = ''
    # force强制进行重新解压，不设置存在文件夹就不重新解压（是否需要检查文件机制？）
    zip_type = ''
    # 命令行
    shell_line = ''
    if (len(sys.argv) > 2):
      zip_path = str(sys.argv[1])
      zip_dec_path = str(sys.argv[2])
      os.system('echo ' + '待解压文件:' + zip_path + '解压到文件夹:' + zip_dec_path)
      if (len(sys.argv) > 3):
        zip_type = sys.argv[3]
        pass
      if (zip_type == 'force'):
        if (os.path.exists(zip_dec_path)):
          shutil.rmtree(zip_dec_path)
          pass
        os.makedirs(zip_dec_path)
        shell_line = '7z x ' + zip_path + ' -o' + zip_dec_path
      else:
        if (os.path.exists(zip_dec_path) == False):
          os.makedirs(zip_dec_path)
          shell_line = '7z x ' + zip_path + ' -o' + zip_dec_path
      # 失败重试机制（TODO）
      os.system('echo ' + '解压命令:' + shell_line)
      with os.popen(shell_line) as a:
        text = a.read()
        if len(text) == 0:
          os.system('echo ' + '解压失败')
        for line in text:
          os.system('echo ' + line)
          if line.rfind('Everything is Ok') != -1:
            os.system('echo ' + '解压成功')
    print('MacOS uZip')
  else:
    print('其他')
