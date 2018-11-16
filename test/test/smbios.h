#ifndef __SM_BIOS_H__
#define __SM_BIOS_H__

#define TRUE 1
#define FALSE 0
#define ERROR -1
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned double QWORD;


typedef struct
{
	BYTE Type;
	BYTE Length;
	WORD Handle;
}SMBStructsHeader;

typedef struct
{
	BYTE        AnchorString[4];
	BYTE        Checksum;
	BYTE        EntryPointLength;
	BYTE        SMBMajorVer;
	BYTE        SMBMinorVer;
	WORD    MaxStructSize;
	BYTE        EntryPointRev;
	BYTE        FormattedArea[5];
	BYTE        InterAnchorStr[5];
	BYTE        InterChecksum;
	WORD    StructTableLength;
	DWORD StructTableAddr;
	WORD    NumOfSMBStructs;
	BYTE        SMBBCDRev;
}SMBStructTableEntryPoint;

typedef struct
{
	SMBStructsHeader Header;
	BYTE        Vendor;
	BYTE        Version;
	WORD    StartingAddrSeg;
	BYTE        ReleaseDate;
	BYTE        ROMSize;
	BYTE        Characteristics[8];
	WORD    Reserved;
	BYTE        MajorRelease;
	BYTE        MinorRelease;
	BYTE        FirmwareMajorRelease;
	BYTE        FirmwareMinorRelease;
}SMBBIOSInfo;

typedef struct
{
	DWORD time_low;
	WORD    time_mid;
	WORD    time_hi_and_ver;
	BYTE        clock_seq_hi_and_res;
	BYTE        clock_seq_low;
	BYTE        Node[6];
}UUIDStruct;

typedef struct
{
	SMBStructsHeader Header;
	BYTE Manufacturer;
	BYTE ProductName;
	BYTE Version;
	BYTE SerialNumber;
	UUIDStruct UUID;
	BYTE WakeupType;
	/* 2.4+ */
	BYTE SKUNumber;
	BYTE Family;
}SMBSysInfo;

typedef struct
{
	SMBStructsHeader Header;
	BYTE Manufacturer;
	BYTE Product;
	BYTE Version;
	BYTE SerialNumber;
	BYTE AssetTag;
	BYTE FeatureFlags;
	BYTE LocationInChassis;
	WORD ChassisHandle;
	BYTE BoardType;
	BYTE Resvd[2];
}SMBBaseBoardInfo;

typedef struct
{
	SMBStructsHeader Header;
	BYTE InterRefDesignator;
	BYTE InterConType;
	BYTE ExterRefDesignator;
	BYTE ExterConType;
	BYTE PortType;
}SMBPortInfo;

typedef struct
{
	SMBStructsHeader Header;
	BYTE Manufacturer;
	BYTE Type;
	BYTE Version;
	BYTE SerialNumber;
	BYTE AssetTagNum;
	BYTE BootupState;
	BYTE PowerSupplyState;
	BYTE ThermalState;
	BYTE SecurityStatus;
	DWORD OEMRes;
	/* 2.3+ */
	BYTE Height;
	BYTE NumOfPowerCords;
	BYTE Resved[3]; /* Contained Elements */
}SMBSysEnclosure;

typedef struct
{
	SMBStructsHeader Header;
	BYTE SocketDesignation;
	BYTE Type;
	BYTE Family;
	BYTE Manufacturer;
	BYTE ID[8];
	BYTE Version;
	BYTE Voltage;
	WORD ExternalClock;
	WORD MaxSpeed;
	WORD CurrentSpeed;
	BYTE Status;
	BYTE Upgrade;
	WORD L1CacheHandle;
	WORD L2CacheHandle;
	WORD L3CacheHandle;
	/* 2.3+ */
	BYTE SerialNumber;
	BYTE AssetTag;
	BYTE PartNumber;
	/* 2.5+ */
	BYTE CoreCount;
	BYTE CoreEnabled;
	BYTE ThreadCount;
	WORD Characterisitics;
	/* 2.6+ */
	WORD ProcessorFamily2;
}SMBProcessorInfo;

typedef struct
{
	SMBStructsHeader Header;
	BYTE SocketDesignation;
	WORD Configuration;
	WORD MaxCacheSize;
	WORD InstalledSize;
	WORD SupportedSRAMType;
	WORD CurrentSRAMType;
	BYTE Speed;
	BYTE ErrCorrectionType;
	BYTE SysCacheType;
	BYTE Associativity;
}SMBCacheInfo;

typedef struct
{
	SMBStructsHeader Header;
	BYTE Designation;
	BYTE Type;
	BYTE DataBusWidth;
	BYTE CurrentUsage;
	BYTE Length;
	WORD ID;
	BYTE Characteristics1;
	BYTE Characteristics2;
	/* 2.6+ */
	WORD SegGroupNum;
	BYTE BusNumber;
	BYTE Dev_FuncNum;
}SMBSystemSlots;

typedef struct
{
	SMBStructsHeader Header;
	BYTE Location;
	BYTE Use;
	BYTE MemErrCorrection;
	DWORD MaxCapacity;
	WORD MemErrInfoHandle;
	WORD NumOfMemDevices;
}SMBPhyMemArray;

typedef struct
{
	SMBStructsHeader Header;
	WORD PhyMemArrayHandle;
	WORD MemErrInfoHandle;
	WORD TotalWidth;
	WORD DataWidth;
	WORD Size;
	BYTE FormFactor;
	BYTE DeviceSet;
	BYTE DeviceLocator;
	BYTE BankLocator;
	BYTE MemoryType;
	WORD TypeDetail;
	/* 2.3+ */
	WORD Speed;
	BYTE Manufacturer;
	BYTE SerialNumber;
	BYTE AssetTag;
	BYTE PartNumber;
	/* 2.6+ */
	BYTE Attributes;
}SMBMemDevice;

typedef struct
{
	SMBStructsHeader Header;
	DWORD StartingAddr;
	DWORD EndingAddr;
	WORD MemArrayHandle;
	BYTE PartitionWidth;
}MemArrayMappedAddr;

typedef struct
{
	SMBStructsHeader Header;
	BYTE DeviceType;
	BYTE DescriptionStr;
}OnBoardDeviceInfo;

DWORD GetFlagPos(BYTE *flag, DWORD start, DWORD end);
DWORD GetStruct(DWORD TableAddress, WORD StructCount, BYTE Type, BYTE n);
BYTE  *ReadStr(DWORD handle, BYTE n);
BYTE CheckBit(WORD value, BYTE bit);

/*BYTE *GenUUID(UUIDStruct uuid);*/
/*BYTE *GenWakeupType(BYTE value);*/
/*BYTE *GenEnclosureType(BYTE value);*/
/*BYTE *GenEnclosureStates(BYTE value);*/
BYTE *GenSecurityStatus(BYTE value);
BYTE *GenProcessorType(BYTE value);
BYTE *GenSRAMType(WORD value);
/*
BYTE *GenProcessorUpgrade(BYTE value);
BYTE *GenErrCorrectionType(BYTE value);
BYTE *GenSysCacheType(BYTE value);
BYTE *GenAssociativity(BYTE value);
*/
BYTE *GenSlotType(BYTE value);
BYTE *GenSlotDataBusWidth(BYTE value);
BYTE *GenCurrentUsage(BYTE value);
/*BYTE *GenSlotLength(BYTE value);*/
/*BYTE *GenFormFactor(BYTE value);*/
BYTE *GenMemoryType(BYTE value);
/*BYTE *GenMemTypeDetail(WORD value);*/
BYTE *GenPortConType(BYTE value);
BYTE *GenPortType(BYTE value);
BYTE *GenDeviceType(BYTE value);

WORD CalCacheSize(WORD value);

DWORD *GetSupportedTypeHandles(DWORD TableAddress, WORD StructCount);
void ShowInfo(DWORD *types, WORD StructCount);
void BIOSInfo(DWORD handle);
void SystemInfo(DWORD handle);
void EnclosureInfo(DWORD handle);
void ProcessorInfo(DWORD handle);
void CacheInfo(DWORD handle);
void SystemSlots(DWORD handle);
void MemoryDevice(DWORD handle);
void BaseBoardInfo(DWORD handle);
void PortConInfo(DWORD handle);
void OnBoardDevInfo(DWORD handle);
#endif 