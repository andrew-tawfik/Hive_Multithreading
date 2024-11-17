# Hive Multithreaded Task Processing Program
### Description
The Hive program is a multithreaded c++ application that demonstrates the power of concurrency and multi-threading programming using worker threads. Inspired by the analogy of a queen bee and her worker bees, the program represents a task management system where:
- The queen (main thread) assigns tasks to a shared task queue. 
- The worker bees (threads) process these tasks in order and notify the queen after their completion