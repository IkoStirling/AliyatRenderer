# AliyatRenderer
---
### 项目架构
	--Core
		--EngineCore
			逻辑循环控制
			系统模块整合
		--EventSystem
			事件处理
		--ResourceManager
			资源管理
		--Graphics
			渲染
		--Network
			网络
		--Physics
			物理
		--InputSystem
			输入
		--SceneManager
			用于管理游戏场景 //是否移出Core？
		--Logger
			Debug工具
		... 
		//some system header files
### 项目约束
1、关于项目命名统一
	
	//类名前缀

		AY 核心组件
		UT 工具组件
		I  接口/抽象类
		ST 结构体

	//代码风格

		类名（无前缀）采用大驼峰
		成员函数及变量采用小驼峰
		私有变量采用下划线前缀

2、关于代码性能的依据

	1ns:
		CPU访问寄存器
	1~10ns:
		L1~L2缓存访问速度
	10~100ns:
		L3缓存访问速度
	100~1000ns:
		系统调用陷入内核态消耗时间
	1~10us:
		线程上下文切换耗时
		内存拷贝64KB约耗时
	10~100us:
		网络代理处理HTTP请求
		顺序读取1MB
		SSD读取速度（8K约100us）
	100~1000us:
		SSD写入速度（8K约1ms）
		云服务商提供区域内的服务器通信延迟
		内存直接缓存，从客户端观测响应约1ms
	1~10ms:
		云计算az节点互访速度（5ms）
		机械硬盘寻道时间
	10~100ms:
		内存顺读1GB
		常见网络延迟
	100~1000ms:
		SSD顺读1GB
	1s:
		数据库操作
		跨云访问
		。。。

	通常来说，连续空间因为缓存的原因会更快，数据量不大的情况下推荐使用vector