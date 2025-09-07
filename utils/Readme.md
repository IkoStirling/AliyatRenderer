-ResourceManager
模块职责：管理硬盘与内存的资源交互


1、Resource 基础资源类
2、Persistence 资源持久化
	当使用到资源时，如果资源是持久化的，则从该类获取、
	该单例维护一个持久化资源map
2、CacheManager 缓存管理
3、MemoryManager 内存管理
	用于处理大小不超过512K，且频繁创建销毁的对象
4、FileHandler 文件管理

1