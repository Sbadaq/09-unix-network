

void *product(void *arg)
{
    int semid = (int)arg;
    struct sembuf sops[2];
   
}

void *consumer(void *arg)
{

}
/*
1. **创建信号量集合**  
   ```c
   int semid = semget(key, nsems, flags);
   ```
2. **PV操作**  
   ```c
   semop(semid, &sembuf, nsops);
   ```
3. **初始化和删除**  
   ```c
   semctl(semid, semnum, SETVAL, arg);
   semctl(semid, 0, IPC_RMID);
   ```

*/

int main(int argc,char **argv)
{
  key_t key = ftok("/tmp/semfile", 'a');  // 生成键值
  int semid = semget(key, 1, IPC_CREAT | 0666); // 创建包含1个信号量的集合
  if (semid == -1) {
      perror("semget error");
      exit(1);
  } 

  pthread_t  product, consumer;
  pthread_create(&product, NULL, product_thread, (void *)semid);
  pthread_create(&consumer, NULL, consumer_thread, (void *)semid);
  pthread_join(product, NULL);
  pthread_join(consumer, NULL);

  semctl(semid, 0, IPC_RMID); // 删除信号量集合
  return 0;

}
