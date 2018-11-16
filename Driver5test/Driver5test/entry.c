#include "Driver.h"
NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status;
	//判断CPU类型
	CPUType();
	//枚举
	EnumeratePCI();
	//EnumPCI();
	KdPrint(("Enter DriverEntry\n"));
	//DbgBreakPoint();
	//设置卸载函数
	pDriverObject->DriverUnload =DDKUnload;
	//设置派遣函数
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = DDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_READ] = DDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = DDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = DDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = DDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = DDKDispatchRoutin;

	//创建驱动设备对象
	status = CreateDevice(pDriverObject);

	KdPrint(("Leave DriverEntry\n"));
	return status;
}

NTSTATUS CreateDevice(
	IN PDRIVER_OBJECT	pDriverObject)
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;

	//创建设备名称
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyDDKDevice");

	//创建设备
	status = IoCreateDevice(pDriverObject,
		sizeof(DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0, TRUE,
		&pDevObj);
	if (!NT_SUCCESS(status))
		return status;

	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDevice = pDevObj;
	pDevExt->ustrDeviceName = devName;
	//创建符号链接
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\TESTDDK");
	pDevExt->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
VOID DDKUnload(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT	pNextObj;
	KdPrint(("Enter DriverUnload\n"));
	
	pNextObj = pDriverObject->DeviceObject;
	while (pNextObj != NULL)
	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
			pNextObj->DeviceExtension;

		//删除符号链接
		UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
		IoDeleteSymbolicLink(&pLinkName);
		pNextObj = pNextObj->NextDevice;
		IoDeleteDevice(pDevExt->pDevice);
	}
}


#pragma PAGEDCODE
NTSTATUS DDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	KdPrint(("Enter HelloDDKDispatchRoutin\n"));

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	//建立一个字符串数组与IRP类型对应起来
	static char* irpname[] =
	{
		"IRP_MJ_CREATE",
		"IRP_MJ_CREATE_NAMED_PIPE",
		"IRP_MJ_CLOSE",
		"IRP_MJ_READ",
		"IRP_MJ_WRITE",
		"IRP_MJ_QUERY_INFORMATION",
		"IRP_MJ_SET_INFORMATION",
		"IRP_MJ_QUERY_EA",
		"IRP_MJ_SET_EA",
		"IRP_MJ_FLUSH_BUFFERS",
		"IRP_MJ_QUERY_VOLUME_INFORMATION",
		"IRP_MJ_SET_VOLUME_INFORMATION",
		"IRP_MJ_DIRECTORY_CONTROL",
		"IRP_MJ_FILE_SYSTEM_CONTROL",
		"IRP_MJ_DEVICE_CONTROL",
		"IRP_MJ_INTERNAL_DEVICE_CONTROL",
		"IRP_MJ_SHUTDOWN",
		"IRP_MJ_LOCK_CONTROL",
		"IRP_MJ_CLEANUP",
		"IRP_MJ_CREATE_MAILSLOT",
		"IRP_MJ_QUERY_SECURITY",
		"IRP_MJ_SET_SECURITY",
		"IRP_MJ_POWER",
		"IRP_MJ_SYSTEM_CONTROL",
		"IRP_MJ_DEVICE_CHANGE",
		"IRP_MJ_QUERY_QUOTA",
		"IRP_MJ_SET_QUOTA",
		"IRP_MJ_PNP",
	};

	UCHAR type = stack->MajorFunction;
	if (type >= arraysize(irpname))
		KdPrint((" - Unknown IRP, major type %X\n", type));
	else
		KdPrint(("\t%s\n", irpname[type]));


	//对一般IRP的简单操作，后面会介绍对IRP更复杂的操作
	NTSTATUS status = STATUS_SUCCESS;
	// 完成IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	KdPrint(("Leave DDKDispatchRoutin\n"));

	return status;
}

stRet CPUType()
{
	stRet st_Ret = {FALSE,FALSE}; 
	ULONG CPUInfo[4] = { -1 };
	CHAR cpu_string[48];
	__cpuid(CPUInfo, 0);
	INT num_ids = CPUInfo[0];
	//GenuineIntel是Intel的CPU 
	//0x47, 0x65, 0x6E, 0x75,   0x69, 0x6E, 0x65, 0x49,   0x6E, 0x74, 0x65, 0x6C,   0x00, 0x00, 0x00, 0x00 GenuineIntel
	if ((CPUInfo[1] == 0x756e6547) && (CPUInfo[2] == 0x6c65746e) && (CPUInfo[3] == 0x49656e69))
	{
		st_Ret.bIntelCPU=TRUE;
		KdPrint(("GenuineIntel\n"));
	}
	//AuthenticAMD 是AMD的CPU
	//0x41, 0x75, 0x74, 0x68,   0x65, 0x6E, 0x74, 0x69,   0x63, 0x41, 0x4D, 0x44,   0x00, 0x00, 0x00, 0x00  AuthenticAMD
	if ((CPUInfo[1] == 0x68747541) && (CPUInfo[2] == 0x444d4163) && (CPUInfo[3] == 0x69746e65))
	{
		st_Ret.bIntelCPU = TRUE;
		KdPrint(("AuthenticAMD\n"));
	}
	INT cpu_info[4] = { -1 };
	__cpuid(cpu_info, 0);
	// Interpret CPU feature information.
	if (num_ids > 0) {
		int cpu_info7[4] = { 0 };
		__cpuid(cpu_info, 1);
		if (num_ids >= 7) {
			__cpuid(cpu_info7, 7);
		}
		signature_ = cpu_info[0];
		stepping_ = cpu_info[0] & 0xf;
		model_ = ((cpu_info[0] >> 4) & 0xf) + ((cpu_info[0] >> 12) & 0xf0);
		family_ = (cpu_info[0] >> 8) & 0xf;
		type_ = (cpu_info[0] >> 12) & 0x3;
		ext_model_ = (cpu_info[0] >> 16) & 0xf;
		ext_family_ = (cpu_info[0] >> 20) & 0xff;
		has_mmx_ = (cpu_info[3] & 0x00800000) != 0;
		has_sse_ = (cpu_info[3] & 0x02000000) != 0;
		has_sse2_ = (cpu_info[3] & 0x04000000) != 0;
		has_sse3_ = (cpu_info[2] & 0x00000001) != 0;
		has_ssse3_ = (cpu_info[2] & 0x00000200) != 0;
		has_sse41_ = (cpu_info[2] & 0x00080000) != 0;
		has_sse42_ = (cpu_info[2] & 0x00100000) != 0;
		has_aesni_ = (cpu_info[2] & 0x02000000) != 0;
		has_avx2_ = has_avx_ && (cpu_info7[1] & 0x00000020) != 0;
	}

	// Get the brand string of the cpu.
	__cpuid(cpu_info, 0x80000000);
	const int parameter_end = 0x80000004;
	int max_parameter = cpu_info[0];

	//CHAR temp[40] = { -1 };
	if (cpu_info[0] >= parameter_end) {
		PCHAR cpu_string_ptr = cpu_string;
		for (int parameter = 0x80000002; parameter <= parameter_end &&
			cpu_string_ptr < &cpu_string[lstrlen(cpu_string)]; parameter++) {
			__cpuid(cpu_info, parameter);
			lstrcpy(cpu_string_ptr, cpu_info, lstrlen(cpu_info));
			cpu_string_ptr += lstrlen(cpu_info);
		}
		//lstrcpy(g_cpu_string, cpu_string_ptr-34, 32);
		KdPrint(("cpu_string:%s cpu_string:%x\n", cpu_string, cpu_string));
		//DbgBreakPoint(); 
	}

	const int parameter_containing_non_stop_time_stamp_counter = 0x80000007;
	if (max_parameter >= parameter_containing_non_stop_time_stamp_counter) {
		__cpuid(cpu_info, parameter_containing_non_stop_time_stamp_counter);
		has_non_stop_time_stamp_counter_ = (cpu_info[3] & (1 << 8)) != 0;
	} 
	return st_Ret;
}
//	字符串长度
DWORD lstrlen(PCHAR pSrc)
{
	DWORD	dwRet = 0;

	while (*pSrc != 0)
	{
		dwRet++;
		pSrc++;
	}
	return dwRet;
}
//	字符拷贝
VOID lstrcpy(PCHAR pDst, PCHAR pSrc, DWORD len)
{
	//while (1)
	//{
	//	//	复制字符，直到零结尾
	//	*pDst = *pSrc;
	//	if (*pSrc == 0)
	//		break;

	//	//	下一个字符
	//	pSrc++;
	//	pDst++;
	//}
	for (DWORD i = 0; i < len; i++) {
			//	复制字符 
			*pDst = *pSrc;
			if (*pSrc == 0)
				break;
			//	下一个字符
			pSrc++;
			pDst++;
	}
}

//VOID EnumeratePCI()
//{
//	ULONG bus;
//	ULONG dev;
//	ULONG func;
//
//	PCI_COMMON_CONFIG PciConfig;
//	PCI_SLOT_NUMBER SlotNumber;
//
//	KdPrint(("Bus\tDevice\tFunc\tVendor\tDevice\tBaseCls\tSubCls\tIRQ\tPIN\n"));
//
//	for (bus = 0; bus <= PDI_BUS_MAX; ++bus)
//	{
//		for (dev = 0; dev <= PDI_DEVICE_MAX; ++dev)
//		{
//			for (func = 0; func <= PDI_FUNCTION_MAX; ++func)
//			{
//				SlotNumber.u.AsULONG = 0;
//				SlotNumber.u.bits.DeviceNumber = dev;
//				SlotNumber.u.bits.FunctionNumber = func;
//
//				RtlZeroMemory(&PciConfig,
//					sizeof(PCI_COMMON_CONFIG));
//				ULONG Size = HalGetBusData(PCIConfiguration,
//					bus,
//					SlotNumber.u.AsULONG,
//					&PciConfig,
//					PCI_COMMON_HDR_LENGTH);
//				if (Size == PCI_COMMON_HDR_LENGTH)
//				{
//					KdPrint(("%02X\t%02X\t%x\t%x\t%x\t%02X\t%02X\t%d\t%d\n",
//						bus,
//						dev,
//						func,
//						PciConfig.VendorID,
//						PciConfig.DeviceID,
//						PciConfig.BaseClass,
//						PciConfig.SubClass,
//						PciConfig.u.type0.InterruptLine,
//						PciConfig.u.type0.InterruptPin));
//				}
//
//			}
//		}
//	}
//
//}
void Out_32(USHORT port, ULONG value)
{ 
	UCHAR method = 4; 
	//KdPrint(("port:%x\n", port));
	//KdPrint(("method:%x\n", method));
	//KdPrint(("value:%x\n", value));
	if (method == 1)//8位操作
	{
		WRITE_PORT_UCHAR((PUCHAR)port, (UCHAR)value);
	}
	else if (method == 2)//16位操作
	{
		WRITE_PORT_USHORT((PUSHORT)port, (USHORT)value);
	}
	else if (method == 4)//32位操作
	{
		WRITE_PORT_ULONG((PULONG)port, (ULONG)value);
	}
}
DWORD In_32(USHORT port)
{ 
	UCHAR method = 4;
	//KdPrint(("port:%x\n", port));
	//KdPrint(("method:%x\n", method));
	if (method == 1)//8位操作
	{
		return READ_PORT_UCHAR((PUCHAR)port);
	}
	else if (method == 2)//16位操作
	{
		return READ_PORT_USHORT((PUSHORT)port);
	}
	else if (method == 4)//32位操作
	{
		return  READ_PORT_ULONG((PULONG)port);
	} 
	return 0;

}
void DisplayPCIConfiguation(int bus, int dev, int func)
{
	DWORD	dwAddr;
	DWORD	dwData;

	MYPCI_COMMON_CONFIG pci_config;
	MYPCI_SLOT_NUMBER SlotNumber;

	SlotNumber.u.AsULONG = 0;
	SlotNumber.u.bits.DeviceNumber = dev;
	SlotNumber.u.bits.FunctionNumber = func;

	dwAddr = 0x80000000 | (bus << 16) | (SlotNumber.u.AsULONG << 8);

	/* 256字节的PCI配置空间 */
	for (int i = 0; i < 0x100; i += 4)
	{
		/* Read */
		Out_32(PCI_CONFIG_ADDRESS, dwAddr | i);
		dwData = In_32(PCI_CONFIG_DATA);
		memcpy(((PUCHAR)&pci_config) + i, &dwData, 4);
	}
//bus:2 dev : 1 func : 0
//	VendorID : 15ad
//	DeviceID : 1977
//	Command : 6
//	Status : 10
//	RevisionID : 9
//	ProgIf : 0
//	SubClass : 3
//	BaseClass : 4
//	CacheLineSize : 40
//	LatencyTimer : 0
//	HeaderType : 0
//	BIST : 0
//	BaseAddresses[0] : 0XFC000004
//	BaseAddresses[1] : 0X00000000
//	BaseAddresses[2] : 0X00000000
//	BaseAddresses[3] : 0X00000000
//	BaseAddresses[4] : 0X00000000
//	BaseAddresses[5] : 0X00000000
//	InterruptLine : 19
//	InterruptPin : 1

	KdPrint(("bus:%d\tdev:%d\tfunc:%d\n", bus, dev, func));

	KdPrint(("VendorID:%x\n", pci_config.VendorID));
	KdPrint(("DeviceID:%x\n", pci_config.DeviceID));
	KdPrint(("Command:%x\n", pci_config.Command));
	KdPrint(("Status:%x\n", pci_config.Status));
	KdPrint(("RevisionID:%x\n", pci_config.RevisionID));
	KdPrint(("ProgIf:%x\n", pci_config.ProgIf));
	KdPrint(("SubClass:%x\n", pci_config.SubClass));
	KdPrint(("BaseClass:%x\n", pci_config.BaseClass));
	KdPrint(("CacheLineSize:%x\n", pci_config.CacheLineSize));
	KdPrint(("LatencyTimer:%x\n", pci_config.LatencyTimer));
	KdPrint(("HeaderType:%x\n", pci_config.HeaderType));
	KdPrint(("BIST:%x\n", pci_config.BIST));
	for (int i = 0; i < 6; i++)
	{
		KdPrint(("BaseAddresses[%d]:0X%08X\n", i, pci_config.u.type0.BaseAddresses[i]));
	}
	KdPrint(("InterruptLine:%d\n", pci_config.u.type0.InterruptLine));
	KdPrint(("InterruptPin:%d\n", pci_config.u.type0.InterruptPin));
}

VOID EnumeratePCI() {
	ULONG ulbus = 0;   //总线号
	ULONG uldev = 0;   //设备号
	ULONG ulfunc = 0;    //功能号
	//枚举总线号
	//for (ulbus = 0; ulbus < PDI_BUS_MAX; ulbus++)
	//{
	//	//枚举设备号  
	//	for (uldev = 0; uldev <= PDI_DEVICE_MAX; uldev++)
	//	{
	//		//枚举功能号 
	//		for (ulfunc = 0; ulfunc <= PDI_FUNCTION_MAX; ulfunc++)
	//		{
	//			DisplayPCIConfiguation(ulbus, uldev, ulfunc);
	//			MySleep(1000);
	//		}
	//	}
	//}
	DisplayPCIConfiguation(0, 21, 0);
	DisplayPCIConfiguation(0, 15, 0);
}
VOID MySleep(LONG msec)
{
	LARGE_INTEGER my_interval;
	my_interval.QuadPart = DELAY_ONE_MILLISECOND;
	my_interval.QuadPart *= msec;
	KeDelayExecutionThread(KernelMode, 0, &my_interval);
}
VOID EnumPCI()
{
	ULONG ulbus = 0;   //总线号
	ULONG uldev = 0;   //设备号
	ULONG func = 0;    //功能号
	wchar_t *pwcHardwareId = NULL;
	//LPVIDEO_INFO      pHardWareInfo = NULL;
	PCI_COMMON_HEADER tgaConfigInfo;
	BOOLEAN           bHaveNvAMD = FALSE;       //是否有独立显卡
	MYPCI_SLOT_NUMBER SlotNumber;
	do
	{
		pwcHardwareId = ExAllocatePoolWithTag(NonPagedPool, 4096, 'root');
		if (NULL == pwcHardwareId)
		{
			break;
		}

		//枚举总线号
		for (ulbus = 0; ulbus < PDI_BUS_MAX; ulbus++)
		{
			//枚举设备号  
			for (uldev = 0; uldev <= PDI_DEVICE_MAX; uldev++) 
			{
				for (func = 0; func <= PDI_FUNCTION_MAX; func++)
				{
				    //bus:2 dev : 1 func : 0
					if ((ulbus==2)&& (uldev==1)&&(func==0))
					{
						RtlZeroMemory(&tgaConfigInfo, sizeof(tgaConfigInfo));
						/*ULONG HalGetBusData(
						IN BUS_DATA_TYPE BusDataType,
						IN ULONG BusNumber,
						IN ULONG SlotNumber,
						IN PVOID Buffer,
						IN ULONG Length
						);
						ULONG GetSetDeviceData(
						PVOID Context,
						ULONG DataType,
						PVOID Buffer,
						ULONG Offset,
						ULONG Length
						)
						*/
						SlotNumber.u.AsULONG = 0;
						SlotNumber.u.bits.DeviceNumber = uldev;
						SlotNumber.u.bits.FunctionNumber = func;
						//取得PCI配置空间
						//HalGetBusDataByOffset(PCIConfiguration, ulbus, SlotNumber.u.AsULONG, &tgaConfigInfo, 0, sizeof(tgaConfigInfo));
						HalGetBusDataByOffset(PCIConfiguration, ulbus, uldev, &tgaConfigInfo, 0, sizeof(tgaConfigInfo));
						//判断获取到的是否是显卡
						if (tgaConfigInfo.BaseClass != 3)
						{
							//continue;
						}
						if ((DWORD)0x8086 == tgaConfigInfo.VendorID)
						{
							//屏蔽Intel核显
							//break;
						}
						//组合硬件ID号
						swprintf(pwcHardwareId,
							L"VEN_%X&DEV_%04X&SUBSYS_%04X%04X&REV_%02X",
							tgaConfigInfo.VendorID,
							tgaConfigInfo.DeviceID,
							tgaConfigInfo.u.type0.SubSystemID,
							tgaConfigInfo.u.type0.SubVendorID,
							tgaConfigInfo.RevisionID);
						KdPrint(("bus:%d\tdev:%d\tfunc:%d\n", ulbus, uldev, func));

						KdPrint(("VendorID:%x\n", tgaConfigInfo.VendorID));
						KdPrint(("DeviceID:%x\n", tgaConfigInfo.DeviceID));
						KdPrint(("Command:%x\n", tgaConfigInfo.Command));
						KdPrint(("Status:%x\n", tgaConfigInfo.Status));
						KdPrint(("RevisionID:%x\n", tgaConfigInfo.RevisionID));
						KdPrint(("ProgIf:%x\n", tgaConfigInfo.ProgIf));
						KdPrint(("SubClass:%x\n", tgaConfigInfo.SubClass));
						KdPrint(("BaseClass:%x\n", tgaConfigInfo.BaseClass));
						KdPrint(("CacheLineSize:%x\n", tgaConfigInfo.CacheLineSize));
						KdPrint(("LatencyTimer:%x\n", tgaConfigInfo.LatencyTimer));
						KdPrint(("HeaderType:%x\n", tgaConfigInfo.HeaderType));
						KdPrint(("BIST:%x\n", tgaConfigInfo.BIST));
						for (int i = 0; i < 6; i++)
						{
							KdPrint(("BaseAddresses[%d]:0X%08X\n", i, tgaConfigInfo.u.type0.BaseAddresses[i]));
						}
						KdPrint(("InterruptLine:%d\n", tgaConfigInfo.u.type0.InterruptLine));
						KdPrint(("InterruptPin:%d\n", tgaConfigInfo.u.type0.InterruptPin));
						//pHardWareInfo = ExAllocatePoolWithTag(NonPagedPool, sizeof(VIDEO_INFO), 'xxx');
						//if (NULL == pHardWareInfo)
						//{
						//	continue;
						//}
						//bHaveNvAMD = TRUE;              //有独立显卡
						//RtlZeroMemory(pHardWareInfo, sizeof(VIDEO_INFO));
						//pHardWareInfo->dwVendorID = tgaConfigInfo.VendorID;
						//pHardWareInfo->dwDeviceID = tgaConfigInfo.DeviceID;
						//pHardWareInfo->wSubSystemID = tgaConfigInfo.u.type0.SubSystemID;
						//pHardWareInfo->wSubVendorID = tgaConfigInfo.u.type0.SubVendorID;
						//pHardWareInfo->wRevisionID = tgaConfigInfo.RevisionID;
						//wcscat(pHardWareInfo->wsHardwareID, pwcHardwareId);
						//InsertTailList(&g_DisPlayInfo.VideoList, &pHardWareInfo->Next);
						//MsgLog(L"枚举PCI成功");
						break;
					}
				
				}
			}
		}        
		if (NULL != pwcHardwareId)
		{
			ExFreePool(pwcHardwareId);
			pwcHardwareId = NULL;
		}
	} while (FALSE);
}