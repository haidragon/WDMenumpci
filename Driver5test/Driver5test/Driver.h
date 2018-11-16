#include <ntifs.h>
#include <NTDDK.h>
#include <string.h>
#include <ntstrsafe.h>
#define PAGEDCODE code_seg("PAGE")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("INIT")
#define PDI_BUS_MAX     0xFF
#define PDI_DEVICE_MAX     0x1F
#define PDI_FUNCTION_MAX     0x7 
#define PAGEDDATA data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("INIT")
#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

#define DELAY_ONE_MICRSECOND (-10)
#define  DELAY_ONE_MILLISECOND (1000 * DELAY_ONE_MICRSECOND)
typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDevice;
	UNICODE_STRING ustrDeviceName;	//Éè±¸Ãû³Æ
	UNICODE_STRING ustrSymLinkName;	//·ûºÅÁ´½ÓÃû
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
typedef struct MyStruct
{
	//Intel
	BOOLEAN bIntelCPU;
	//AMD
	BOOLEAN bAMDCPU;
}stRet;
INT signature_;  // raw form of type, family, model, and stepping
INT type_;  // process type
INT family_;  // family of the processor
INT model_;  // model of processor
INT stepping_;  // processor revision number
INT ext_model_;
INT ext_family_;
BOOLEAN has_mmx_;
BOOLEAN has_sse_;
BOOLEAN has_sse2_;
BOOLEAN has_sse3_;
BOOLEAN has_ssse3_;
BOOLEAN has_sse41_;
BOOLEAN has_sse42_;
BOOLEAN has_avx_;
BOOLEAN has_avx2_;
BOOLEAN has_aesni_;
BOOLEAN has_non_stop_time_stamp_counter_;
ULONG CPUInfo[4] = { -1 };
// º¯ÊýÉùÃ÷
stRet CPUType();
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject);
VOID DDKUnload(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS DDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
//	×Ö·û´®³¤¶È
DWORD lstrlen(PCHAR pSrc);
//	×Ö·û¿½±´
VOID lstrcpy(PCHAR pDst, PCHAR pSrc, DWORD len);

//////////////////////////////////////////////////////////////
/* PCIÅäÖÃ¿Õ¼ä¼Ä´æÆ÷ */
#define PCI_CONFIG_ADDRESS      0xCF8
#define PCI_CONFIG_DATA         0xCFC


#define PCI_TYPE0_ADDRESSES             6
#define PCI_TYPE1_ADDRESSES             2
#define PCI_TYPE2_ADDRESSES             5

typedef struct _MYPCI_COMMON_CONFIG {
	USHORT  VendorID;                   // (ro)
	USHORT  DeviceID;                   // (ro)
	USHORT  Command;                    // Device control
	USHORT  Status;
	UCHAR   RevisionID;                 // (ro)
	UCHAR   ProgIf;                     // (ro)
	UCHAR   SubClass;                   // (ro)
	UCHAR   BaseClass;                  // (ro)
	UCHAR   CacheLineSize;              // (ro+)
	UCHAR   LatencyTimer;               // (ro+)
	UCHAR   HeaderType;                 // (ro)
	UCHAR   BIST;                       // Built in self test

	union {
		struct _MYPCI_HEADER_TYPE_0 {
			ULONG   BaseAddresses[PCI_TYPE0_ADDRESSES];
			ULONG   CIS;
			USHORT  SubVendorID;
			USHORT  SubSystemID;
			ULONG   ROMBaseAddress;
			UCHAR   CapabilitiesPtr;
			UCHAR   Reserved1[3];
			ULONG   Reserved2;
			UCHAR   InterruptLine;      //
			UCHAR   InterruptPin;       // (ro)
			UCHAR   MinimumGrant;       // (ro)
			UCHAR   MaximumLatency;     // (ro)
		} type0;

		// end_wdm end_ntminiport end_ntndis

		//
		// PCI to PCI Bridge
		//

		struct _MYPCI_HEADER_TYPE_1 {
			ULONG   BaseAddresses[PCI_TYPE1_ADDRESSES];
			UCHAR   PrimaryBus;
			UCHAR   SecondaryBus;
			UCHAR   SubordinateBus;
			UCHAR   SecondaryLatency;
			UCHAR   IOBase;
			UCHAR   IOLimit;
			USHORT  SecondaryStatus;
			USHORT  MemoryBase;
			USHORT  MemoryLimit;
			USHORT  PrefetchBase;
			USHORT  PrefetchLimit;
			ULONG   PrefetchBaseUpper32;
			ULONG   PrefetchLimitUpper32;
			USHORT  IOBaseUpper16;
			USHORT  IOLimitUpper16;
			UCHAR   CapabilitiesPtr;
			UCHAR   Reserved1[3];
			ULONG   ROMBaseAddress;
			UCHAR   InterruptLine;
			UCHAR   InterruptPin;
			USHORT  BridgeControl;
		} type1;

		//
		// PCI to CARDBUS Bridge
		//

		struct _MYPCI_HEADER_TYPE_2 {
			ULONG   SocketRegistersBaseAddress;
			UCHAR   CapabilitiesPtr;
			UCHAR   Reserved;
			USHORT  SecondaryStatus;
			UCHAR   PrimaryBus;
			UCHAR   SecondaryBus;
			UCHAR   SubordinateBus;
			UCHAR   SecondaryLatency;
			struct {
				ULONG   Base;
				ULONG   Limit;
			}       Range[PCI_TYPE2_ADDRESSES - 1];
			UCHAR   InterruptLine;
			UCHAR   InterruptPin;
			USHORT  BridgeControl;
		} type2;

		// begin_wdm begin_ntminiport begin_ntndis

	} u;

	UCHAR   DeviceSpecific[192];

} MYPCI_COMMON_CONFIG, *MYPPCI_COMMON_CONFIG;

typedef struct _MYPCI_SLOT_NUMBER {
	union {
		struct {
			ULONG   FunctionNumber : 3;
			ULONG   DeviceNumber : 5;
			ULONG   Reserved : 24;
		} bits;
		ULONG   AsULONG;
	} u;
} MYPCI_SLOT_NUMBER, *MYPPCI_SLOT_NUMBER;
VOID EnumeratePCI();

VOID EnumPCI();
VOID MySleep(LONG msec);