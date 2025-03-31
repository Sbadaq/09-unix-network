
# 启动监控进程（临时添加sleep便于调试）
./monitor &
# 获取监控进程PID
MON_PID=$(ps aux | grep '[.]/monitor' | awk '{print $2}')
echo "监控进程PID: $MON_PID"
