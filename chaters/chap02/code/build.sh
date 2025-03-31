
# 编译监控守护进程
gcc -g -o monitor monitor.c -lrt

# 编译日志守护进程（如果有）
gcc -g -o logger logger.c -lrt
