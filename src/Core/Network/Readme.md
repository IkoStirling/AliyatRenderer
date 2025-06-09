# 网络模块

关于目前采取的方案：

	三层架构：协议层，代理层，应用层
	消息类型：定长消息头+可变消息体
	协议支持：TCP、UDP、基于UDP的KCP、Websocket
	线程支持：独立网络线程+复用事件系统的回调处理

参考信息：

	消息类型：手动管理读取消息长度+asio::async_xxx_some

2025.06.03 更新：

	模块主类： AYNetworkManager
	协议路由： AYProtocolRouter -> TCP / UDP(KCP)

	TCP分化：	
		AYTcpSession			单个连接会话维护
		AYTcpServer				监听
		AYTcpPacketAssembler	Tcp包体处理（粘包拆包）

	UDP分化：
		AYUdpServer				维护UDP通信端点	双向
		AYReliableUdpSession	实现可靠UDP传输	双向
		AYUnreliableUdpSession	原始UDP通信	双向

	数据序列化：
		AYPacketSerializer		结构化数据↔二进制	双向转换
		AYCompression			数据压缩/解压	发送前/接收后

	安全处理：
		AYEncryption			端到端加密	发送前/接收后
		AYCheatDetection		异常行为分析	接收→分析