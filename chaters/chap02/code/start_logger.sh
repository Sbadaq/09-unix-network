
./logger &
# 获取日志进程PID
LOG_PID=$(ps aux | grep '[.]/logger' | awk '{print $2}')
echo "日志进程PID: $LOG_PID"
