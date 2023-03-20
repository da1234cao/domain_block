import shutil
import os
import subprocess


def copy_files_and_dirs(src_dir, dst_dir, items):
    results = []
    for item in items:
        src_path = os.path.join(src_dir, item)
        dst_path = os.path.join(dst_dir, item)
        if os.path.isfile(src_path):
            try:
                if (os.path.exists(os.path.dirname(dst_path)) != True):
                    os.makedirs(os.path.dirname(dst_path))
                shutil.copy(src_path, dst_path)
                results.append((item, True, ''))
            except Exception as e:
                results.append((item, False, str(e)))
        elif os.path.isdir(src_path):
            try:
                shutil.copytree(src_path, dst_path)
                results.append((item, True, ''))
            except Exception as e:
                results.append((item, False, str(e)))
        else:
            print(f'unknow : {src_path}')
            continue

    for item, success, reason in results:
        if success:
            print(f'{item} 复制成功')
        else:
            print(f'{item} 复制失败，原因：{reason}')


if __name__ == '__main__':
    # 拷贝需要的二进制文件
    bin_dir = "../../UI/build-domain_block_client-Desktop_Qt_6_4_2_MSVC2019_64bit-Release"
    dst_dir = "ui-output"
    items = [
        'domain_block_client.exe'
    ]
    copy_files_and_dirs(bin_dir, dst_dir, items)

    # 拷贝需要的资源文件
    res_dir = "../../UI/domain_block_client"
    items = [
        'domain_block.ini'
    ]
    copy_files_and_dirs(res_dir, dst_dir, items)

    # 进入output目录
    os.chdir(dst_dir)
    result = subprocess.run(
        ["windeployqt", "domain_block_client.exe"], capture_output=True)
    # 打印标准输出和标准错误输出
    print("STDOUT:")
    print(result.stdout.decode())
    print("STDERR:")
    print(result.stderr.decode())
    # 打印命令的返回值
    print("Return Code:")
    print(result.returncode)
