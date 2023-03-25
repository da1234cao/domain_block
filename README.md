


```json
{
    "name": "dacao",
    "description": "Block the following networks",
    "rules": [
        {
            "action": "block",
            "domain": "www.baidu.com",
            "ip": [
                "180.101.50.188",
                "180.101.50.242"
            ]
        }
    ]
}
```

## 编译与打包

### linux平台

```shell
mkdir build && cd build
cmake ..
make package
```

### windwos平台

windows下面的打包有点麻烦。

* qt程序没有打包成安装包，通过脚本执行windeployqt，生成一个免安装版本。

* 驱动程序，没有接入cmake管理，得使用vs打开进行编译。

* 上面程序编译通过后，使用inno setup进行打包

```shell
mkdir build && cd build
cmake  -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release

# 生成windows的qt免安装包
## 进入项目的\package\windows目录，运行
### 输出qt程序在ui-output目录
python .\ui.py

# 生成驱动
## 进入项目的windows\WFP\block目录，使用visiual studio打开项目
### 进行编译

## 当上面编译完成后
### 使用inno setup，打开\package\windows\service.iss
### 输出安装包在service-output目录
```