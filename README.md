# os_exp
 my os_exp in XJTU


我尝试了文件索引系统与一个基于Rust的的网卡驱动设计

# Rust for Linux网卡驱动设计
## 环境配置：
- 拉取rust for linux代码
- 安装rust环境
- 安装clang，bindgen等相关包
- 获取代码仓库

编译密码：
```
make LLVM=1 menuconfig
make LLVM=1 -j$(nproc)
```
## 实现内容：
- 一个简单的HelloWorld驱动，用于验证开发流程；
- 一个文件查看驱动实现；
  操作内容：
  ```
  echo "Hello" > /dev/cicv
  cat /dev/cicv
  ```
- 网卡驱动实现；
  操作内容：
```
insmod r4l_e1000_demo.ko
ip link set eth0 up
ip addr add broadcast 10.0.2.255 dev eth0
ip addr add 10.0.2.15/255.255.255.0 dev eth0 
ip route add default via 10.0.2.1
ping 10.0.2.2
```
# 文件系统设计实验
