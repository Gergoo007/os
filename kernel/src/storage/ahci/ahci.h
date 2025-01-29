#pragma once

#include <util/types.h>
#include <util/attrs.h>
#include <pci/pci.h>
#include <devmgr/devmgr.h>

enum {
	AHCI_FIS_REG_H2D	= 0x27,
	AHCI_FIS_REG_D2H	= 0x34,
	AHCI_FIS_DMA_ACT	= 0x39,
	AHCI_FIS_DMA_SETUP	= 0x41,
	AHCI_FIS_DATA		= 0x46,
	AHCI_FIS_BIST		= 0x58,
	AHCI_FIS_PIO_SETUP	= 0x5F,
	AHCI_FIS_DEV_BITS	= 0xA1,
};

enum {
	AHCI_SIG_SATA	= 0x00000101,
	AHCI_SIG_SATAPI	= 0xEB140101,
	AHCI_SIG_SEMB	= 0xC33C0101,	// Enclosure management bridge
	AHCI_SIG_PM		= 0x96690101,
};

enum {
	ATA_DEV_BUSY = 0x80,
	ATA_DEV_DRQ = 0x08,
};

// ATA parancsok
enum {
	ATA_CMD_IDENTIFY = 0xec,
	ATA_READ_DMA_EX = 0x25,
};

typedef volatile struct _attr_packed fis_reg_h2d {
	u8 type;

	u8 port_multi : 4;
	u8 : 3;
	u8 cmd_ctrl : 1;

	u8 cmd;
	u8 featurel;

	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 dev;

	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 featureh;

	u8 countl;
	u8 counth;

	u8 isochronous_complete;
	u8 ctrl;

	u32 : 32;
} fis_reg_h2d;

typedef volatile struct _attr_packed fis_reg_d2h {
	u8 type;

	u8 port_multi : 4;
	u8 : 2;
	u8 intr : 1;
	u8 : 1;

	u8 sts;
	u8 err;

	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 dev;

	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 : 8;

	u8 countl;
	u8 counth;

	u16 : 16;

	u32 : 32;
} fis_reg_d2h;

typedef volatile struct _attr_packed hba_port {
	u32 cmd_listl;
	u32 cmd_listu;

	u32 fisl;
	u32 fisu;

	union _attr_packed {
		u32 value;
		struct _attr_packed {
			u8 d2h : 1;
			u8 pio_setup : 1;
			u8 dma_setup : 1;
			u8 set_dev_bits : 1;
			u8 unknown_fis : 1;
			u8 prdt_finished : 1;
			u8 connection_change : 1;
			u8 mechanical_change : 1;
			u16 : 14;
			u8 phyrdy_change : 1;
			u8 incorrect_pm : 1;
			u8 overflow_sts : 1;
			u8 : 1;
			u8 if_non_fatal : 1;
			u8 if_fatal : 1;
			u8 host_bus_non_fatal : 1;
			u8 host_bus_fatal : 1;
			u8 task_file_error : 1;
			u8 cold_port_detect : 1;
		};
	} int_sts;
	u32 int_enable;

	union _attr_packed {
		u32 value;
		struct _attr_packed {
			u8 start : 1;
			u8 spin_up : 1;
			u8 power_on : 1;
			u8 cmd_list_overr : 1;
			u8 fis_rx_enable : 1;
			u8 : 3;
			u8 current_cmd_slot : 5;
			u8 mpss : 1;
			u8 fis_rx_running : 1;
			u8 cmd_list_running : 1;
			u8 cps : 1;
			u8 pma : 1;
		};
	} cmd;
	u32 : 32;

	u32 task_file_data;
	u32 sig;

	union _attr_packed {
		u32 value;
		struct _attr_packed {
			u8 det : 4;
			u8 spd : 4;
			u8 ipm : 4;
		};
	} ssts;
	u32 sctrl;
	u32 serror;
	u32 sactive;

	u32 cmd_issue;

	u32 snotif;

	u32 fis_based_switch;

	u32 res[15];
} hba_port;

typedef volatile struct _attr_packed hba_mem {
	u32 host_capability;
	u32 global_host_ctrl;
	u32 intr_sts;
	u32 ports_impl;
	u32 ver;
	u32 ccc_ctrl;
	u32 ccc_pts;
	u32 em_loc;
	u32 em_ctrl;
	u32 host_capability2;
	u32 handoff_sts;

	u8 res[0xd4];

	hba_port ports[32];
} hba_mem;

typedef volatile struct _attr_packed hba_cmd_hdr {
	u8 fis_len : 5; // FIS hossz * 32 bit
	u8 satapi : 1;
	u8 write : 1;
	u8 prefetchable : 1;
	u8 reset : 1;
	u8 bist : 1;
	u8 clear_busy_upon_ok : 1;
	u8 : 1;
	u8 pm : 1;

	u16 prdt_num_entries;
	
	volatile u32 prd_bytes_transferred;

	u32 cmd_table_desc_l;
	u32 cmd_table_desc_u;

	u32 _[4];
} hba_cmd_hdr;

typedef volatile struct _attr_packed hba_prdt_entry {
	u32 data_base_addrl;
	u32 data_base_addru;
	u32 : 32;

	u32 num_bytes : 22;
	u32 : 9;
	u32 ioc : 1;
} hba_prdt_entry;

typedef volatile struct _attr_packed hba_cmd_table {
	u8 cmd_fis[64];
	u8 atapi_cmd[16];
	u8 _[48];

	hba_prdt_entry prdt_entry[1]; // Lehet belőle semennyi meg 65535 is
} hba_cmd_table;

typedef struct ahci {
	u32 pi;
} ahci;

// typedef struct _attr_packed ata_identity {
// 	u16 flags;
// 	u16 r1[9];
// 	char serial[20];
// 	u16 r2[3];
// 	char firmware[8];
// 	char model[40];
// 	u16 sectors_per_int;
// 	u16 : 16;
// 	union _attr_packed {
// 		u32 value;
// 		struct _attr_packed {
// 			u8 phys_sector_alignment : 2;
// 			u8 : 6;
// 			u8 dma_support : 1;
// 			u8 lba_support : 1;
// 			u8 io_rdy_disable : 1;
// 			u8 io_rdy_supported : 1;
// 			u8 : 1;
// 			u8 standby_timer_support : 1;
// 			u8 : 2;
// 			u16 : 16;
// 		};
// 	} caps;
// 	u32 : 32;
// 	u16 valid_ext_data;
// 	u16 r4[5];
// 	u16 size_of_rw_mult;
// 	u32 sectors_28;
// 	u16 r5[38];
// 	u64 sectors_48;
// 	u16 r6[152];
// } ata_identity;

typedef struct _attr_packed {
	struct {
		u16 Reserved1 : 1;
		u16 Retired3 : 1;
		u16 ResponseIncomplete : 1;
		u16 Retired2 : 3;
		u16 FixedDevice : 1;
		u16 RemovableMedia : 1;
		u16 Retired1 : 7;
		u16 DeviceType : 1;
	} GeneralConfiguration;
	u16 NumCylinders;
	u16 SpecificConfiguration;
	u16 NumHeads;
	u16 Retired1[2];
	u16 NumSectorsPerTrack;
	u16 VendorUnique1[3];
	u8 serial[20];
	u16 Retired2[2];
	u16 Obsolete1;
	u8 firmware[8];
	u8 model[40];
	u8 MaximumBlockTransfer;
	u8 VendorUnique2;
	struct {
		u16 FeatureSupported : 1;
		u16 Reserved : 15;
	} TrustedComputing;
	struct {
		u8 CurrentLongPhysicalSectorAlignment : 2;
		u8 ReservedByte49 : 6;
		u8 DmaSupported : 1;
		u8 LbaSupported : 1;
		u8 IordyDisable : 1;
		u8 IordySupported : 1;
		u8 Reserved1 : 1;
		u8 StandybyTimerSupport : 1;
		u8 Reserved2 : 2;
		u16 ReservedWord50;
	} Capabilities;
	u16 ObsoleteWords51[2];
	u16 TranslationFieldsValid : 3;
	u16 Reserved3 : 5;
	u16 FreeFallControlSensitivity : 8;
	u16 NumberOfCurrentCylinders;
	u16 NumberOfCurrentHeads;
	u16 CurrentSectorsPerTrack;
	u32 CurrentSectorCapacity;
	u8 CurrentMultiSectorSetting;
	u8 MultiSectorSettingValid : 1;
	u8 ReservedByte59 : 3;
	u8 SanitizeFeatureSupported : 1;
	u8 CryptoScrambleExtCommandSupported : 1;
	u8 OverwriteExtCommandSupported : 1;
	u8 BlockEraseExtCommandSupported : 1;
	u32 UserAddressableSectors;
	u16 ObsoleteWord62;
	u16 MultiWordDMASupport : 8;
	u16 MultiWordDMAActive : 8;
	u16 AdvancedPIOModes : 8;
	u16 ReservedByte64 : 8;
	u16 MinimumMWXferCycleTime;
	u16 RecommendedMWXferCycleTime;
	u16 MinimumPIOCycleTime;
	u16 MinimumPIOCycleTimeIORDY;
	struct {
		u16 ZonedCapabilities : 2;
		u16 NonVolatileWriteCache : 1;
		u16 ExtendedUserAddressableSectorsSupported : 1;
		u16 DeviceEncryptsAllUserData : 1;
		u16 ReadZeroAfterTrimSupported : 1;
		u16 Optional28BitCommandsSupported : 1;
		u16 IEEE1667 : 1;
		u16 DownloadMicrocodeDmaSupported : 1;
		u16 SetMaxSetPasswordUnlockDmaSupported : 1;
		u16 WriteBufferDmaSupported : 1;
		u16 ReadBufferDmaSupported : 1;
		u16 DeviceConfigIdentifySetDmaSupported : 1;
		u16 LPSAERCSupported : 1;
		u16 DeterministicReadAfterTrimSupported : 1;
		u16 CFastSpecSupported : 1;
	} AdditionalSupported;
	u16 ReservedWords70[5];
	u16 QueueDepth : 5;
	u16 ReservedWord75 : 11;
	struct {
		u16 Reserved0 : 1;
		u16 SataGen1 : 1;
		u16 SataGen2 : 1;
		u16 SataGen3 : 1;
		u16 Reserved1 : 4;
		u16 NCQ : 1;
		u16 HIPM : 1;
		u16 PhyEvents : 1;
		u16 NcqUnload : 1;
		u16 NcqPriority : 1;
		u16 HostAutoPS : 1;
		u16 DeviceAutoPS : 1;
		u16 ReadLogDMA : 1;
		u16 Reserved2 : 1;
		u16 CurrentSpeed : 3;
		u16 NcqStreaming : 1;
		u16 NcqQueueMgmt : 1;
		u16 NcqReceiveSend : 1;
		u16 DEVSLPtoReducedPwrState : 1;
		u16 Reserved3 : 8;
	} SerialAtaCapabilities;
	struct {
		u16 Reserved0 : 1;
		u16 NonZeroOffsets : 1;
		u16 DmaSetupAutoActivate : 1;
		u16 DIPM : 1;
		u16 InOrderData : 1;
		u16 HardwareFeatureControl : 1;
		u16 SoftwareSettingsPreservation : 1;
		u16 NCQAutosense : 1;
		u16 DEVSLP : 1;
		u16 HybridInformation : 1;
		u16 Reserved1 : 6;
	} SerialAtaFeaturesSupported;
	struct {
		u16 Reserved0 : 1;
		u16 NonZeroOffsets : 1;
		u16 DmaSetupAutoActivate : 1;
		u16 DIPM : 1;
		u16 InOrderData : 1;
		u16 HardwareFeatureControl : 1;
		u16 SoftwareSettingsPreservation : 1;
		u16 DeviceAutoPS : 1;
		u16 DEVSLP : 1;
		u16 HybridInformation : 1;
		u16 Reserved1 : 6;
	} SerialAtaFeaturesEnabled;
	u16 MajorRevision;
	u16 MinorRevision;
	struct {
		u16 SmartCommands : 1;
		u16 SecurityMode : 1;
		u16 RemovableMediaFeature : 1;
		u16 PowerManagement : 1;
		u16 Reserved1 : 1;
		u16 WriteCache : 1;
		u16 LookAhead : 1;
		u16 ReleaseInterrupt : 1;
		u16 ServiceInterrupt : 1;
		u16 DeviceReset : 1;
		u16 HostProtectedArea : 1;
		u16 Obsolete1 : 1;
		u16 WriteBuffer : 1;
		u16 ReadBuffer : 1;
		u16 Nop : 1;
		u16 Obsolete2 : 1;
		u16 DownloadMicrocode : 1;
		u16 DmaQueued : 1;
		u16 Cfa : 1;
		u16 AdvancedPm : 1;
		u16 Msn : 1;
		u16 PowerUpInStandby : 1;
		u16 ManualPowerUp : 1;
		u16 Reserved2 : 1;
		u16 SetMax : 1;
		u16 Acoustics : 1;
		u16 BigLba : 1;
		u16 DeviceConfigOverlay : 1;
		u16 FlushCache : 1;
		u16 FlushCacheExt : 1;
		u16 WordValid83 : 2;
		u16 SmartErrorLog : 1;
		u16 SmartSelfTest : 1;
		u16 MediaSerialNumber : 1;
		u16 MediaCardPassThrough : 1;
		u16 StreamingFeature : 1;
		u16 GpLogging : 1;
		u16 WriteFua : 1;
		u16 WriteQueuedFua : 1;
		u16 WWN64Bit : 1;
		u16 URGReadStream : 1;
		u16 URGWriteStream : 1;
		u16 ReservedForTechReport : 2;
		u16 IdleWithUnloadFeature : 1;
		u16 WordValid : 2;
	} CommandSetSupport;
	struct {
		u16 SmartCommands : 1;
		u16 SecurityMode : 1;
		u16 RemovableMediaFeature : 1;
		u16 PowerManagement : 1;
		u16 Reserved1 : 1;
		u16 WriteCache : 1;
		u16 LookAhead : 1;
		u16 ReleaseInterrupt : 1;
		u16 ServiceInterrupt : 1;
		u16 DeviceReset : 1;
		u16 HostProtectedArea : 1;
		u16 Obsolete1 : 1;
		u16 WriteBuffer : 1;
		u16 ReadBuffer : 1;
		u16 Nop : 1;
		u16 Obsolete2 : 1;
		u16 DownloadMicrocode : 1;
		u16 DmaQueued : 1;
		u16 Cfa : 1;
		u16 AdvancedPm : 1;
		u16 Msn : 1;
		u16 PowerUpInStandby : 1;
		u16 ManualPowerUp : 1;
		u16 Reserved2 : 1;
		u16 SetMax : 1;
		u16 Acoustics : 1;
		u16 BigLba : 1;
		u16 DeviceConfigOverlay : 1;
		u16 FlushCache : 1;
		u16 FlushCacheExt : 1;
		u16 Resrved3 : 1;
		u16 Words119_120Valid : 1;
		u16 SmartErrorLog : 1;
		u16 SmartSelfTest : 1;
		u16 MediaSerialNumber : 1;
		u16 MediaCardPassThrough : 1;
		u16 StreamingFeature : 1;
		u16 GpLogging : 1;
		u16 WriteFua : 1;
		u16 WriteQueuedFua : 1;
		u16 WWN64Bit : 1;
		u16 URGReadStream : 1;
		u16 URGWriteStream : 1;
		u16 ReservedForTechReport : 2;
		u16 IdleWithUnloadFeature : 1;
		u16 Reserved4 : 2;
	} CommandSetActive;
	u16 UltraDMASupport : 8;
	u16 UltraDMAActive : 8;
	struct {
		u16 TimeRequired : 15;
		u16 ExtendedTimeReported : 1;
	} NormalSecurityEraseUnit;
	struct {
		u16 TimeRequired : 15;
		u16 ExtendedTimeReported : 1;
	} EnhancedSecurityEraseUnit;
	u16 CurrentAPMLevel : 8;
	u16 ReservedWord91 : 8;
	u16 MasterPasswordID;
	u16 HardwareResetResult;
	u16 CurrentAcousticValue : 8;
	u16 RecommendedAcousticValue : 8;
	u16 StreamMinRequestSize;
	u16 StreamingTransferTimeDMA;
	u16 StreamingAccessLatencyDMAPIO;
	u32 StreamingPerfGranularity;
	u32 Max48BitLBA[2];
	u16 StreamingTransferTime;
	u16 DsmCap;
	struct {
		u16 LogicalSectorsPerPhysicalSector : 4;
		u16 Reserved0 : 8;
		u16 LogicalSectorLongerThan256Words : 1;
		u16 MultipleLogicalSectorsPerPhysicalSector : 1;
		u16 Reserved1 : 2;
	} PhysicalLogicalSectorSize;
	u16 InterSeekDelay;
	u16 WorldWideName[4];
	u16 ReservedForWorldWideName128[4];
	u16 ReservedForTlcTechnicalReport;
	u16 WordsPerLogicalSector[2];
	struct {
		u16 ReservedForDrqTechnicalReport : 1;
		u16 WriteReadVerify : 1;
		u16 WriteUncorrectableExt : 1;
		u16 ReadWriteLogDmaExt : 1;
		u16 DownloadMicrocodeMode3 : 1;
		u16 FreefallControl : 1;
		u16 SenseDataReporting : 1;
		u16 ExtendedPowerConditions : 1;
		u16 Reserved0 : 6;
		u16 WordValid : 2;
	} CommandSetSupportExt;
	struct {
		u16 ReservedForDrqTechnicalReport : 1;
		u16 WriteReadVerify : 1;
		u16 WriteUncorrectableExt : 1;
		u16 ReadWriteLogDmaExt : 1;
		u16 DownloadMicrocodeMode3 : 1;
		u16 FreefallControl : 1;
		u16 SenseDataReporting : 1;
		u16 ExtendedPowerConditions : 1;
		u16 Reserved0 : 6;
		u16 Reserved1 : 2;
	} CommandSetActiveExt;
	u16 ReservedForExpandedSupportandActive[6];
	u16 MsnSupport : 2;
	u16 ReservedWord127 : 14;
	struct {
		u16 SecuritySupported : 1;
		u16 SecurityEnabled : 1;
		u16 SecurityLocked : 1;
		u16 SecurityFrozen : 1;
		u16 SecurityCountExpired : 1;
		u16 EnhancedSecurityEraseSupported : 1;
		u16 Reserved0 : 2;
		u16 SecurityLevel : 1;
		u16 Reserved1 : 7;
	} SecurityStatus;
	u16 ReservedWord129[31];
	struct {
		u16 MaximumCurrentInMA : 12;
		u16 CfaPowerMode1Disabled : 1;
		u16 CfaPowerMode1Required : 1;
		u16 Reserved0 : 1;
		u16 Word160Supported : 1;
	} CfaPowerMode1;
	u16 ReservedForCfaWord161[7];
	u16 NominalFormFactor : 4;
	u16 ReservedWord168 : 12;
	struct {
		u16 SupportsTrim : 1;
		u16 Reserved0 : 15;
	} DataSetManagementFeature;
	u16 AdditionalProductID[4];
	u16 ReservedForCfaWord174[2];
	u16 CurrentMediaSerialNumber[30];
	struct {
		u16 Supported : 1;
		u16 Reserved0 : 1;
		u16 WriteSameSuported : 1;
		u16 ErrorRecoveryControlSupported : 1;
		u16 FeatureControlSuported : 1;
		u16 DataTablesSuported : 1;
		u16 Reserved1 : 6;
		u16 VendorSpecific : 4;
	} SCTCommandTransport;
	u16 ReservedWord207[2];
	struct {
		u16 AlignmentOfLogicalWithinPhysical : 14;
		u16 Word209Supported : 1;
		u16 Reserved0 : 1;
	} BlockAlignment;
	u16 WriteReadVerifySectorCountMode3Only[2];
	u16 WriteReadVerifySectorCountMode2Only[2];
	struct {
		u16 NVCachePowerModeEnabled : 1;
		u16 Reserved0 : 3;
		u16 NVCacheFeatureSetEnabled : 1;
		u16 Reserved1 : 3;
		u16 NVCachePowerModeVersion : 4;
		u16 NVCacheFeatureSetVersion : 4;
	} NVCacheCapabilities;
	u16 NVCacheSizeLSW;
	u16 NVCacheSizeMSW;
	u16 NominalMediaRotationRate;
	u16 ReservedWord218;
	struct {
		u8 NVCacheEstimatedTimeToSpinUpInSeconds;
		u8 Reserved;
	} NVCacheOptions;
	u16 WriteReadVerifySectorCountMode : 8;
	u16 ReservedWord220 : 8;
	u16 ReservedWord221;
	struct {
		u16 MajorVersion : 12;
		u16 TransportType : 4;
	} TransportMajorVersion;
	u16 TransportMinorVersion;
	u16 ReservedWord224[6];
	u64 ExtendedNumberOfUserAddressableSectors;
	u16 MinBlocksPerDownloadMicrocodeMode03;
	u16 MaxBlocksPerDownloadMicrocodeMode03;
	u16 ReservedWord236[19];
	u16 Signature : 8;
	u16 CheckSum : 8;
} ata_identity;

ata_identity* ahci_identify(hba_port* port);
void ahci_init(dev_misc* hc);
void ahci_read(dev_misc* dev, u32 pnum, u64 start, u64 count, void* buf);
