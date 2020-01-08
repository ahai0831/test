# coding=UTF-8
import hashlib
import sys
import os


def CalcSha1(filepath):
  with open(filepath, 'rb') as f:
    sha1obj = hashlib.sha1()
    sha1obj.update(f.read())
    hash = sha1obj.hexdigest()
    print(hash)
    return hash


if __name__ == '__main__':
  import os
  import sys
  import platform
  import csv

  if (platform.system() == 'Windows' or platform.system() == 'Darwin'):
    # abs_module_path = os.path.abspath(__file__)
    # abs_module_dir = abs_module_path[:abs_module_path.rfind("\\")] + "\\"
    # target_bat_file = os.path.join(
    #     abs_module_dir, '..\\..\\vcproj\\batch-scripts\\package_get.bat')

    # csv_path = ''
    # if (len(sys.argv) > 1):
    #   csv_path = str(sys.argv[1])
    #   cmd_line = 'call cmd /C \"' + target_bat_file + ' ' + csv_path + '\"'
    #   print(cmd_line)
    #   a = os.popen(cmd_line)
    #   text = a.read()
    #   print(text)
    # elif (platform.system() == 'Darwin'):
    abs_module_path = os.path.abspath(__file__)
    abs_module_dir = abs_module_path[:abs_module_path.rfind("\\")] + "\\"
    print(abs_module_dir)
    if (len(sys.argv) > 1):
      csv_path = str(sys.argv[1])
      print('csv_path:' + csv_path)
      # 保存路径为上层级
      save_path = csv_path[:csv_path.rfind("/")] + "/"
      print('save_path:' + save_path)
      if (os.path.exists(csv_path)):
        with open(csv_path, 'r') as csvfile:
          render = csv.reader(csvfile)
          for line in render:
            file_path = save_path + line[0]
            url = line[1]
            hash_value = line[2]
            print(url)
            # 只读取包含下载链接
            if (url.rfind('http') != -1):
              if (os.path.exists(file_path) and
                  CalcSha1(file_path) == hash_value):
                break
              with os.popen('aria2c' + ' -d ' + save_path + ' ' + url) as a:
                message = a.read()
                print(message)
                last_line = message[-1]
              # 判断是否成功，如果不成功就执行下行（TODO）
              if (last_line.rfind('OK') != -1 and
                  CalcSha1(file_path) == hash_value):
                print('下载完成')
                break
              pass
            pass
        pass
      pass
    print('MacOS')
  else:
    print('其他')
