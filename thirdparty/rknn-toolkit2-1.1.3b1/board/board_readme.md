## Toolkit2 连板功能
Toolkit2的连板功能一般需要更新板端的 rknn_server 和 librknnrt.so，并且手动启动 rknn_server 才能正常工作。

rknn_server: 是一个运行在板子上的后台代理服务，用于接收PC通过USB传输过来的协议，然后执行板端runtime对应的接口，并返回结果给PC。

librknnrt.so: 是一个板端的runtime库。

## 存放目录
### Android平台
```
Android
└── arm64-v8a
    └── vendor
        ├── bin
        │   └── rknn_server
        └── lib
            └── librknnrt.so
```

### Linux平台
```
Linux
└── aarch64
    └── usr
        ├── bin
        │   ├── restart_rknn.sh
        │   ├── rknn_server
        │   └── start_rknn.sh
        └── lib
            └── librknnrt.so
```

## 启动步骤
### Android平台
1. adb root && adb remount
2. adb push Android/${BOARD_ARCH}/vendor/bin/rknn_server /vendor/bin/
3. adb push Android/${BOARD_ARCH}/vendor/lib/librknnrt.so 到 /vendor/lib64（64位系统特有）或 /vendor/lib 目录
4. 进入板子的串口终端，执行：
```
su
chmod +x /vendor/bin/rknn_server
setenforce 0
sync
reboot
```
5. 重新进入板子的串口终端，执行 `ps -A|grep rknn_server`, 看是否有 rknn_server 的进程（较新的固件开机会自动启动rknn_server），
   如果不存在，则手动执行 
```
su
setenforce 0
/vendor/bin/rknn_server &
```

### Linux平台
1. adb push Linux/${BOARD_ARCH}/usr/bin/下的所有文件到/usr/bin目录
2. adb push Linux/${BOARD_ARCH}/usr/lib/librknnrt.so到/usr/lib目录
3. 进入板子的串口终端，执行：
```
chmod +x /usr/bin/rknn_server
chmod +x /usr/bin/start_rknn.sh
chmod +x /usr/bin/restart_rknn.sh
./restart_rknn.sh
```

### 串口查看rknn_server详细日志
查看详细的rknn_server日志,需要设置日志等级,并在python端执行推理后获取,步骤如下:
#### Android平台
1. 进入串口终端,设置日志等级
```
su
setenforce 0
export RKNN_SERVER_LOGLEVEL=5
```
2. 杀死rknn_server进程
```
kill -9 `pgrep rknn_server`
```
3. 重启rknn_server进程(若固件没有自启动rknn_server)
```
/vendor/bin/rknn_server &
logcat
```
#### Linux平台
1. 进入串口终端,设置日志等级
```
export RKNN_SERVER_LOGLEVEL=5
```
2. 重启rknn_server进程(若固件没有自启动rknn_server)
```
restart_rknn.sh
```