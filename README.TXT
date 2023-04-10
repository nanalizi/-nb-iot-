器材:
	精英STM32F103开发板ZET6
	
目的:
	ds18b20+PH传感器+TDS传感器+BC26上传阿里云
	
硬件资源:
	1,BC26使用串口2通信，接线方式：
	vcc-5v；gnd-gnd；TXD-PA3;RXD-PA3.
	2,串口1(波特率:115200,PA9/PA10连接在板载USB转串口芯片CH340上面)用于打印数据至串口助手。
	3,浑浊度传感器使用ADC1接线,使用ADC(STM32内部ADC1,通道2,即:ADC1_CH2)
	VCC-5V;GND-GND;AO-PA2;DO-PA12.
	4,PH传感器使用ADC1接线，使用ADC(STM32内部ADC1,通道4,即:ADC1_CH4).
	VCC-5V;GND-GND;PO-PA2;T1-A0.PA8,PA11
	
现象:
	本项目通过串口2读取水温，PH，TDS等，通过串口4连接BC26将传感器数据上传至阿里云服务器。并通过阿里云公用版云智能APP下发数据，实现物联网的实时阈值控制。
	云端通信协议为MQTT协议。

三段码
ProductKey
a1pEJg7uaBv
DeviceName
lizi
DeviceSecret
694502a1956a3faf8be0975118e6af3e

