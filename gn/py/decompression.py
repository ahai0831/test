# coding=UTF-8
if __name__ == '__main__':
    import os
    import sys
    abs_module_path = os.path.abspath(__file__)
    abs_module_dir = abs_module_path[:abs_module_path.rfind("\\")]+"\\"
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
