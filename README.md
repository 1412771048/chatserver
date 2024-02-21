## 环境
ubuntu22.04
## 1. 准备工作
### 安装boost库
```
sudo apt update 
wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz
tar xf boost_1_84_0.tar.gz && cd boost_1_84_0/ && ./bootstrap.sh 
./b2
sudo ./b2 install
```
### 安装muduo库
```
git clone git@github.com:chenshuo/muduo.git
cd muduo && sed -i '/^  option/s/^  /#  /' CMakeLists.txt && ./build.sh
./build.sh install
cd ../build/release-install-cpp11/include/ && sudo mv muduo/ /usr/include/ && sudo mv ../lib/* /usr/local/lib
```
### 安装mysql8.0及其开发包
```
sudo apt install mysql-server libmysqlclient-dev -y
sudo cat /etc/mysql/debian.cnf 
使用初始用户密码登录：musql -uxx -pxx
use mysql;
ALTER USER 'root'@'localhost' IDENTIFIED WITH caching_sha2_password BY '123456';
FLUSH PRIVILEGES;
exit;
mysql -uroot -p123456 <./third/chat.sql
```
至此，mysql的库表创建完成
### 安装nginx
```
sudo apt install libpcre3-dev zlib1g openssl -y
wget https://nginx.org/download/nginx-1.24.0.tar.gz
tar xf nginx-1.24.0.tar.gz && cd nginx-1.24.0/ && sudo ./configure --with-stream
sudo make && sudo make install 
cd .. && rm -rf nginx-1.24.0/
sudo vim /usr/local/nginx/conf/nginx.conf 
追加以下内容
stream {
    upstream MyServer {
        #后续在这里加配服务器即可
        server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s; #weight权重(涉及负载均衡策略)，max_fails心跳次数
        server 127.0.0.1:6001 weight=1 max_fails=3 fail_timeout=30s;
    }
    server {
        proxy_connect_timeout 1s; 
        listen 8000;
        proxy_pass MyServer;
        tcp_nodelay on;
    }
}
启动：sudo /usr/local/nginx/sbin/nginx

```
### 安装redis及其开发包
```
sudo apt install redis-server -y
git clone https://github.com/redis/hiredis.git
cd hiredis/ && sudo make && sudo make install
sudo ldconfig /usr/local/lib && cd .. && rm -rf hiredis/
```
## 2. 项目编译
```
./build.sh
```
在bin目录下生成chatclient和chatserver
## 3. 运行
```
cd bin/
以配置2台服务器为例：
./chatserver 127.0.0.1 6000 
./chatserver 127.0.0.1 6001
客户端连接nginx:
./chatclient 127.0.0.1 8000
```

