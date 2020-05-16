/*
 * Copyright (c) 2018, Cobham Gaisler AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * AMBA Plug'n Play vendor and device name definitions.
 *
 * Created from GRLIB revision 4234
 */

#include <lib/ambapp_names.h>

#ifndef NULL
#define NULL 0
#endif

/* Vendor RESERVED devices */
static struct ambapp_device_name RESERVED_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor GAISLER devices */
static struct ambapp_device_name GAISLER_devices[] =
{
  {GAISLER_LEON2DSU, "LEON2DSU", "LEON2 Debug Support Unit"},
  {GAISLER_LEON3, "LEON3", "LEON3 SPARC V8 Processor"},
  {GAISLER_LEON3DSU, "LEON3DSU", "LEON3 Debug Support Unit"},
  {GAISLER_ETHAHB, "ETHAHB", "OC ethernet AHB interface"},
  {GAISLER_APBMST, "APBMST", "AHB/APB Bridge"},
  {GAISLER_AHBUART, "AHBUART", "AHB Debug UART"},
  {GAISLER_SRCTRL, "SRCTRL", "Simple SRAM Controller"},
  {GAISLER_SDCTRL, "SDCTRL", "PC133 SDRAM Controller"},
  {GAISLER_SSRCTRL, "SSRCTRL", "Synchronous SRAM Controller"},
  {GAISLER_I2C2AHB, "I2C2AHB", "I2C to AHB Bridge"},
  {GAISLER_APBUART, "APBUART", "Generic UART"},
  {GAISLER_IRQMP, "IRQMP", "Multi-processor Interrupt Ctrl."},
  {GAISLER_AHBRAM, "AHBRAM", "Single-port AHB SRAM module"},
  {GAISLER_AHBDPRAM, "AHBDPRAM", "Dual-port AHB SRAM module"},
  {GAISLER_GRIOMMU2, "GRIOMMU2", "IOMMU secondary master i/f"},
  {GAISLER_GPTIMER, "GPTIMER", "Modular Timer Unit"},
  {GAISLER_PCITRG, "PCITRG", "Simple 32-bit PCI Target"},
  {GAISLER_PCISBRG, "PCISBRG", "Simple 32-bit PCI Bridge"},
  {GAISLER_PCIFBRG, "PCIFBRG", "Fast 32-bit PCI Bridge"},
  {GAISLER_PCITRACE, "PCITRACE", "32-bit PCI Trace Buffer"},
  {GAISLER_DMACTRL, "DMACTRL", "PCI/AHB DMA controller"},
  {GAISLER_AHBTRACE, "AHBTRACE", "AMBA Trace Buffer"},
  {GAISLER_DSUCTRL, "DSUCTRL", "DSU/ETH controller"},
  {GAISLER_CANAHB, "CANAHB", "OC CAN AHB interface"},
  {GAISLER_GPIO, "GPIO", "General Purpose I/O port"},
  {GAISLER_AHBROM, "AHBROM", "Generic AHB ROM"},
  {GAISLER_AHBJTAG, "AHBJTAG", "JTAG Debug Link"},
  {GAISLER_ETHMAC, "ETHMAC", "GR Ethernet MAC"},
  {GAISLER_SWNODE, "SWNODE", "SpaceWire Node Interface"},
  {GAISLER_SPW, "SPW", "SpaceWire Serial Link"},
  {GAISLER_AHB2AHB, "AHB2AHB", "AHB-to-AHB Bridge"},
  {GAISLER_USBDC, "USBDC", "GR USB 2.0 Device Controller"},
  {GAISLER_USB_DCL, "USB_DCL", "USB Debug Communication Link"},
  {GAISLER_DDRMP, "DDRMP", "Multi-port DDR controller"},
  {GAISLER_ATACTRL, "ATACTRL", "ATA controller"},
  {GAISLER_DDRSP, "DDRSP", "Single-port DDR266 controller"},
  {GAISLER_EHCI, "EHCI", "USB Enhanced Host Controller"},
  {GAISLER_UHCI, "UHCI", "USB Universal Host Controller"},
  {GAISLER_I2CMST, "I2CMST", "AMBA Wrapper for OC I2C-master"},
  {GAISLER_SPW2, "SPW2", "GRSPW2 SpaceWire Serial Link"},
  {GAISLER_AHBDMA, "AHBDMA", "Simple AHB DMA controller"},
  {GAISLER_NUHOSP3, "NUHOSP3", "Nuhorizons Spartan3 IO I/F"},
  {GAISLER_CLKGATE, "CLKGATE", "Clock gating unit"},
  {GAISLER_SPICTRL, "SPICTRL", "SPI Controller"},
  {GAISLER_DDR2SP, "DDR2SP", "Single-port DDR2 controller"},
  {GAISLER_SLINK, "SLINK", "SLINK Master"},
  {GAISLER_GRTM, "GRTM", "CCSDS Telemetry Encoder"},
  {GAISLER_GRTC, "GRTC", "CCSDS Telecommand Decoder"},
  {GAISLER_GRPW, "GRPW", "PacketWire to AMBA AHB I/F"},
  {GAISLER_GRCTM, "GRCTM", "CCSDS Time Manager"},
  {GAISLER_GRHCAN, "GRHCAN", "ESA HurriCANe CAN with DMA"},
  {GAISLER_GRFIFO, "GRFIFO", "FIFO Controller"},
  {GAISLER_GRADCDAC, "GRADCDAC", "ADC / DAC Interface"},
  {GAISLER_GRPULSE, "GRPULSE", "General Purpose I/O with Pulses"},
  {GAISLER_GRTIMER, "GRTIMER", "Timer Unit with Latches"},
  {GAISLER_AHB2PP, "AHB2PP", "AMBA AHB to Packet Parallel I/F"},
  {GAISLER_GRVERSION, "GRVERSION", "Version and Revision Register"},
  {GAISLER_APB2PW, "APB2PW", "PacketWire Transmit Interface"},
  {GAISLER_PW2APB, "PW2APB", "PacketWire Receive Interface"},
  {GAISLER_GRCAN, "GRCAN", "CAN Controller with DMA"},
  {GAISLER_I2CSLV, "I2CSLV", "I2C Slave"},
  {GAISLER_U16550, "U16550", "Simple 16550 UART"},
  {GAISLER_AHBMST_EM, "AHBMST_EM", "AMBA Master Emulator"},
  {GAISLER_AHBSLV_EM, "AHBSLV_EM", "AMBA Slave Emulator"},
  {GAISLER_GRTESTMOD, "GRTESTMOD", "Test report module"},
  {GAISLER_ASCS, "ASCS", "ASCS Master"},
  {GAISLER_IPMVBCTRL, "IPMVBCTRL", "IPM-bus/MVBC memory controller"},
  {GAISLER_SPIMCTRL, "SPIMCTRL", "SPI Memory Controller"},
  {GAISLER_L4STAT, "L4STAT", "LEON4 Statistics Unit"},
  {GAISLER_LEON4, "LEON4", "LEON4 SPARC V8 Processor"},
  {GAISLER_LEON4DSU, "LEON4DSU", "LEON4 Debug Support Unit"},
  {GAISLER_PWM, "PWM", "PWM generator"},
  {GAISLER_L2CACHE, "L2CACHE", "L2-Cache Controller"},
  {GAISLER_SDCTRL64, "SDCTRL64", "64-bit PC133 SDRAM Controller"},
  {GAISLER_GR1553B, "GR1553B", "MIL-STD-1553B Interface"},
  {GAISLER_1553TST, "1553TST", "MIL-STD-1553B Test Device"},
  {GAISLER_GRIOMMU, "GRIOMMU", "IO Memory Management Unit"},
  {GAISLER_FTAHBRAM, "FTAHBRAM", "Generic FT AHB SRAM module"},
  {GAISLER_FTSRCTRL, "FTSRCTRL", "Simple FT SRAM Controller"},
  {GAISLER_AHBSTAT, "AHBSTAT", "AHB Status Register"},
  {GAISLER_LEON3FT, "LEON3FT", "LEON3FT SPARC V8 Processor"},
  {GAISLER_FTMCTRL, "FTMCTRL", "Memory controller with EDAC"},
  {GAISLER_FTSDCTRL, "FTSDCTRL", "FT PC133 SDRAM Controller"},
  {GAISLER_FTSRCTRL8, "FTSRCTRL8", "FT 8-bit SRAM/16-bit IO Ctrl"},
  {GAISLER_MEMSCRUB, "MEMSCRUB", "AHB Memory Scrubber"},
  {GAISLER_FTSDCTRL64, "FTSDCTRL64", "64-bit FT SDRAM Controller"},
  {GAISLER_NANDFCTRL, "NANDFCTRL", "NAND Flash Controller"},
  {GAISLER_N2DLLCTRL, "N2DLLCTRL", "N2X DLL Dynamic Config. i/f"},
  {GAISLER_N2PLLCTRL, "N2PLLCTRL", "N2X PLL Dynamic Config. i/f"},
  {GAISLER_SPI2AHB, "SPI2AHB", "SPI to AHB Bridge"},
  {GAISLER_DDRSDMUX, "DDRSDMUX", "Muxed FT DDR/SDRAM controller"},
  {GAISLER_AHBFROM, "AHBFROM", "Flash ROM Memory"},
  {GAISLER_PCIEXP, "PCIEXP", "Xilinx PCI EXPRESS Wrapper"},
  {GAISLER_APBPS2, "APBPS2", "PS2 interface"},
  {GAISLER_VGACTRL, "VGACTRL", "VGA controller"},
  {GAISLER_LOGAN, "LOGAN", "On chip Logic Analyzer"},
  {GAISLER_SVGACTRL, "SVGACTRL", "SVGA frame buffer"},
  {GAISLER_T1AHB, "T1AHB", "Niagara T1 PCX/AHB bridge"},
  {GAISLER_MP7WRAP, "MP7WRAP", "CoreMP7 wrapper"},
  {GAISLER_GRSYSMON, "GRSYSMON", "AMBA wrapper for System Monitor"},
  {GAISLER_GRACECTRL, "GRACECTRL", "System ACE I/F Controller"},
  {GAISLER_ATAHBSLV, "ATAHBSLV", "AMBA Test Framework AHB Slave"},
  {GAISLER_ATAHBMST, "ATAHBMST", "AMBA Test Framework AHB Master"},
  {GAISLER_ATAPBSLV, "ATAPBSLV", "AMBA Test Framework APB Slave"},
  {GAISLER_MIGDDR2, "MIGDDR2", "Xilinx MIG DDR2 Controller"},
  {GAISLER_LCDCTRL, "LCDCTRL", "LCD Controller"},
  {GAISLER_SWITCHOVER, "SWITCHOVER", "Switchover Logic"},
  {GAISLER_FIFOUART, "FIFOUART", "UART with large FIFO"},
  {GAISLER_MUXCTRL, "MUXCTRL", "Analogue multiplexer control"},
  {GAISLER_B1553BC, "B1553BC", "AMBA Wrapper for Core1553BBC"},
  {GAISLER_B1553RT, "B1553RT", "AMBA Wrapper for Core1553BRT"},
  {GAISLER_B1553BRM, "B1553BRM", "AMBA Wrapper for Core1553BRM"},
  {GAISLER_AES, "AES", "Advanced Encryption Standard"},
  {GAISLER_ECC, "ECC", "Elliptic Curve Cryptography"},
  {GAISLER_PCIF, "PCIF", "AMBA Wrapper for CorePCIF"},
  {GAISLER_CLKMOD, "CLKMOD", "CPU Clock Switching Ctrl module"},
  {GAISLER_HAPSTRAK, "HAPSTRAK", "HAPS HapsTrak I/O Port"},
  {GAISLER_TEST_1X2, "TEST_1X2", "HAPS TEST_1x2 interface"},
  {GAISLER_WILD2AHB, "WILD2AHB", "WildCard CardBus interface"},
  {GAISLER_BIO1, "BIO1", "Basic I/O board BIO1"},
  {GAISLER_AESDMA, "AESDMA", "AES 256 DMA"},
  {GAISLER_GRPCI2, "GRPCI2", "GRPCI2 PCI/AHB bridge"},
  {GAISLER_GRPCI2_DMA, "GRPCI2_DMA", "GRPCI2 DMA interface"},
  {GAISLER_GRPCI2_TB, "GRPCI2_TB", "GRPCI2 Trace buffer"},
  {GAISLER_MMA, "MMA", "Memory Mapped AMBA"},
  {GAISLER_SATCAN, "SATCAN", "SatCAN controller"},
  {GAISLER_CANMUX, "CANMUX", "CAN Bus multiplexer"},
  {GAISLER_GRTMRX, "GRTMRX", "CCSDS Telemetry Receiver"},
  {GAISLER_GRTCTX, "GRTCTX", "CCSDS Telecommand Transmitter"},
  {GAISLER_GRTMDESC, "GRTMDESC", "CCSDS Telemetry Descriptor"},
  {GAISLER_GRTMVC, "GRTMVC", "CCSDS Telemetry VC Generator"},
  {GAISLER_GEFFE, "GEFFE", "Geffe Generator"},
  {GAISLER_GPREG, "GPREG", "General Purpose Register"},
  {GAISLER_GRTMPAHB, "GRTMPAHB", "CCSDS Telemetry VC AHB Input"},
  {GAISLER_SPWCUC, "SPWCUC", "CCSDS CUC / SpaceWire I/F"},
  {GAISLER_SPW2_DMA, "SPW2_DMA", "GRSPW Router DMA interface"},
  {GAISLER_SPWROUTER, "SPWROUTER", "GRSPW Router"},
  {GAISLER_EDCLMST, "EDCLMST", "EDCL master interface"},
  {GAISLER_GRPWTX, "GRPWTX", "PacketWire Transmitter with DMA"},
  {GAISLER_GRPWRX, "GRPWRX", "PacketWire Receiver with DMA"},
  {GAISLER_GPREGBANK, "GPREGBANK", "General Purpose Register Bank"},
  {GAISLER_MIG_7SERIES, "MIG_7SERIES", "Xilinx MIG DDR3 Controller"},
  {GAISLER_GRSPW2_SIST, "GRSPW2_SIST", "GRSPW Router SIST"},
  {GAISLER_SGMII, "SGMII", "XILINX SGMII Interface"},
  {GAISLER_RGMII, "RGMII", "Gaisler RGMII Interface"},
  {GAISLER_IRQGEN, "IRQGEN", "Interrupt generator"},
  {GAISLER_GRDMAC, "GRDMAC", "GRDMAC DMA Controller"},
  {GAISLER_AHB2AVLA, "AHB2AVLA", "Avalon-MM memory controller"},
  {GAISLER_SPWTDP, "SPWTDP", "CCSDS TDP / SpaceWire I/F"},
  {GAISLER_L3STAT, "L3STAT", "LEON3 Statistics Unit"},
  {GAISLER_GR740THS, "GR740THS", "Temperature sensor"},
  {GAISLER_GRRM, "GRRM", "Reconfiguration Module"},
  {GAISLER_CMAP, "CMAP", "CCSDS Memory Access Protocol"},
  {GAISLER_CPGEN, "CPGEN", "Discrete Command Pulse Gen"},
  {GAISLER_AMBAPROT, "AMBAPROT", "AMBA Protection Unit"},
  {GAISLER_IGLOO2_BRIDGE, "IGLOO2_BRIDGE", "Microsemi SF2/IGLOO2 MSS/HPMS"},
  {GAISLER_AHB2AXI, "AHB2AXI", "AMBA AHB/AXI Bridge"},
  {GAISLER_AXI2AHB, "AXI2AHB", "AMBA AXI/AHB Bridge"},
  {GAISLER_FDIR_RSTCTRL, "FDIR_RSTCTRL", "FDIR Reset Controller"},
  {GAISLER_APB3MST, "APB3MST", "AHB/APB3 Bridge"},
  {GAISLER_LRAM, "LRAM", "Dual-port AHB(/CPU) On-Chip RAM"},
  {GAISLER_BOOTSEQ, "BOOTSEQ", "Custom AHB sequencer"},
  {GAISLER_TCCOP, "TCCOP", "CCSDS Telecommand Decoder (COP)"},
  {GAISLER_SPIMASTER, "SPIMASTER", "Simple SPI Master"},
  {GAISLER_SPISLAVE, "SPISLAVE", "Dual-port SPI Slave"},
  {GAISLER_GRSRIO, "GRSRIO", "Serial RapidIO Logical Layer"},
  {GAISLER_AHBLM2AHB, "AHBLM2AHB", "AHB-Lite master to AHB master"},
  {GAISLER_AHBS2NOC, "AHBS2NOC", "AHB slave to NoC"},
  {GAISLER_TCAU, "TCAU", "Authentication Unit"},
  {GAISLER_GRTMDYNVCID, "GRTMDYNVCID", "CCSDS Telemetry Dynamic VCID"},
  {GAISLER_RNOCIRQPROP, "RNOCIRQPROP", "RNoC Interrupt propagator"},
  {GAISLER_FTADDR, "FTADDR", "DDR2/DDR3 controller with EDAC"},
  {GAISLER_ATG, "ATG", "AMBA2 Test Pattern Generator"},
  {GAISLER_DFITRACE, "DFITRACE", "DFI2.1 Trace Buffer"},
  {GAISLER_SELFTEST, "SELFTEST", "TV selftest module"},
  {GAISLER_DFIERRINJ, "DFIERRINJ", "DFI error injection module"},
  {GAISLER_DFICHECK, "DFICHECK", "DFI timing check module"},
  {GAISLER_GRCANFD, "GRCANFD", "CAN-FD Controller with DMA"},
  {GAISLER_NIM, "NIM", "Synchronous serial interface"},
  {GAISLER_GRSHYLOC, "GRSHYLOC", "SHYLOC Compressor with DMA"},
  {GAISLER_GRTACHOM, "GRTACHOM", "Simple Digital Tachometer"},
  {GAISLER_L5STAT, "L5STAT", "LEON5 Statistics Unit"},
  {GAISLER_LEON5, "LEON5", "LEON5 SPARC V8 Processor"},
  {GAISLER_LEON5DSU, "LEON5DSU", "LEON5 Debug Support Unit"},
  {GAISLER_SPFI, "SPFI", "GRSPFI SpaceFibre Serial Link"},
  {0, NULL, NULL}
};


/* Vendor PENDER devices */
static struct ambapp_device_name PENDER_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor ESA devices */
static struct ambapp_device_name ESA_devices[] =
{
  {ESA_LEON2, "LEON2", "LEON2 SPARC V8 Processor"},
  {ESA_LEON2APB, "LEON2APB", "LEON2 Peripheral Bus"},
  {ESA_IRQ, "IRQ", "LEON2 Interrupt Controller"},
  {ESA_TIMER, "TIMER", "LEON2 Timer"},
  {ESA_UART, "UART", "LEON2 UART"},
  {ESA_CFG, "CFG", "LEON2 Configuration Register"},
  {ESA_IO, "IO", "LEON2 Input/Output"},
  {ESA_MCTRL, "MCTRL", "LEON2 Memory Controller"},
  {ESA_PCIARB, "PCIARB", "PCI Arbiter"},
  {ESA_HURRICANE, "HURRICANE", "HurriCANe/HurryAMBA CAN Ctrl"},
  {ESA_SPW_RMAP, "SPW_RMAP", "UoD/Saab SpaceWire/RMAP link"},
  {ESA_AHBUART, "AHBUART", "LEON2 AHB Debug UART"},
  {ESA_SPWA, "SPWA", "ESA/ASTRIUM SpaceWire link"},
  {ESA_BOSCHCAN, "BOSCHCAN", "SSC/BOSCH CAN Ctrl"},
  {ESA_IRQ2, "IRQ2", "LEON2 Secondary Irq Controller"},
  {ESA_AHBSTAT, "AHBSTAT", "LEON2 AHB Status Register"},
  {ESA_WPROT, "WPROT", "LEON2 Write Protection"},
  {ESA_WPROT2, "WPROT2", "LEON2 Extended Write Protection"},
  {ESA_PDEC3AMBA, "PDEC3AMBA", "ESA CCSDS PDEC3AMBA TC Decoder"},
  {ESA_PTME3AMBA, "PTME3AMBA", "ESA CCSDS PTME3AMBA TM Encoder"},
  {0, NULL, NULL}
};


/* Vendor ASTRIUM devices */
static struct ambapp_device_name ASTRIUM_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor OPENCHIP devices */
static struct ambapp_device_name OPENCHIP_devices[] =
{
  {OPENCHIP_APBGPIO, "APBGPIO", "APB General Purpose IO"},
  {OPENCHIP_APBI2C, "APBI2C", "APB I2C Interface"},
  {OPENCHIP_APBSPI, "APBSPI", "APB SPI Interface"},
  {OPENCHIP_APBCHARLCD, "APBCHARLCD", "APB Character LCD"},
  {OPENCHIP_APBPWM, "APBPWM", "APB PWM"},
  {OPENCHIP_APBPS2, "APBPS2", "APB PS/2 Interface"},
  {OPENCHIP_APBMMCSD, "APBMMCSD", "APB MMC/SD Card Interface"},
  {OPENCHIP_APBNAND, "APBNAND", "APB NAND(SmartMedia) Interface"},
  {OPENCHIP_APBLPC, "APBLPC", "APB LPC Interface"},
  {OPENCHIP_APBCF, "APBCF", "APB CompactFlash (IDE)"},
  {OPENCHIP_APBSYSACE, "APBSYSACE", "APB SystemACE Interface"},
  {OPENCHIP_APB1WIRE, "APB1WIRE", "APB 1-Wire Interface"},
  {OPENCHIP_APBJTAG, "APBJTAG", "APB JTAG TAP Master"},
  {OPENCHIP_APBSUI, "APBSUI", "APB Simple User Interface"},
  {0, NULL, NULL}
};


/* Vendor OPENCORES devices */
static struct ambapp_device_name OPENCORES_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor CONTRIB devices */
static struct ambapp_device_name CONTRIB_devices[] =
{
  {CONTRIB_CORE1, "CORE1", "Contributed core 1"},
  {CONTRIB_CORE2, "CORE2", "Contributed core 2"},
  {CONTRIB_CORE3, "CORE3", "Contributed core 2"},
  {0, NULL, NULL}
};


/* Vendor DLR devices */
static struct ambapp_device_name DLR_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor EONIC devices */
static struct ambapp_device_name EONIC_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor TELECOMPT devices */
static struct ambapp_device_name TELECOMPT_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor DTU devices */
static struct ambapp_device_name DTU_devices[] =
{
  {DTU_IV, "IV", "Instrument Virtualizer"},
  {DTU_RBMMTRANS, "RBMMTRANS", "RB/MM Transfer"},
  {DTU_FTMCTRL, "FTMCTRL", "Memory controller with 8CS"},
  {0, NULL, NULL}
};


/* Vendor BSC devices */
static struct ambapp_device_name BSC_devices[] =
{
  {BSC_CORE1, "CORE1", "Core 1"},
  {BSC_CORE2, "CORE2", "Core 2"},
  {0, NULL, NULL}
};


/* Vendor RADIONOR devices */
static struct ambapp_device_name RADIONOR_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor GLEICHMANN devices */
static struct ambapp_device_name GLEICHMANN_devices[] =
{
  {GLEICHMANN_CUSTOM, "CUSTOM", "Custom device"},
  {GLEICHMANN_GEOLCD01, "GEOLCD01", "GEOLCD01 graphics system"},
  {GLEICHMANN_DAC, "DAC", "Sigma delta DAC"},
  {GLEICHMANN_HPI, "HPI", "AHB-to-HPI bridge"},
  {GLEICHMANN_SPI, "SPI", "SPI master"},
  {GLEICHMANN_HIFC, "HIFC", "Human interface controller"},
  {GLEICHMANN_ADCDAC, "ADCDAC", "Sigma delta ADC/DAC"},
  {GLEICHMANN_SPIOC, "SPIOC", "SPI master for SDCard IF"},
  {GLEICHMANN_AC97, "AC97", "AC97 Controller"},
  {0, NULL, NULL}
};


/* Vendor MENTA devices */
static struct ambapp_device_name MENTA_devices[] =
{
  {MENTA_EFPGA_IP, "EFPGA_IP", "eFPGA Core IP"},
  {0, NULL, NULL}
};


/* Vendor SUN devices */
static struct ambapp_device_name SUN_devices[] =
{
  {SUN_T1, "T1", "Niagara T1 SPARC V9 Processor"},
  {SUN_S1, "S1", "Niagara S1 SPARC V9 Processor"},
  {0, NULL, NULL}
};


/* Vendor MOVIDIA devices */
static struct ambapp_device_name MOVIDIA_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor L3T devices */
static struct ambapp_device_name L3T_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor ORBITA devices */
static struct ambapp_device_name ORBITA_devices[] =
{
  {ORBITA_1553B, "1553B", "MIL-STD-1553B Controller"},
  {ORBITA_429, "429", "429 Interface"},
  {ORBITA_SPI, "SPI", "SPI Interface"},
  {ORBITA_I2C, "I2C", "I2C Interface"},
  {ORBITA_SMARTCARD, "SMARTCARD", "Smart Card Reader"},
  {ORBITA_SDCARD, "SDCARD", "SD Card Reader"},
  {ORBITA_UART16550, "UART16550", "16550 UART"},
  {ORBITA_CRYPTO, "CRYPTO", "Crypto Engine"},
  {ORBITA_SYSIF, "SYSIF", "System Interface"},
  {ORBITA_PIO, "PIO", "Programmable IO module"},
  {ORBITA_RTC, "RTC", "Real-Time Clock"},
  {ORBITA_COLORLCD, "COLORLCD", "Color LCD Controller"},
  {ORBITA_PCI, "PCI", "PCI Module"},
  {ORBITA_DSP, "DSP", "DPS Co-Processor"},
  {ORBITA_USBHOST, "USBHOST", "USB Host"},
  {ORBITA_USBDEV, "USBDEV", "USB Device"},
  {0, NULL, NULL}
};


/* Vendor SYNOPSYS devices */
static struct ambapp_device_name SYNOPSYS_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor NASA devices */
static struct ambapp_device_name NASA_devices[] =
{
  {NASA_EP32, "EP32", "EP32 Forth processor"},
  {0, NULL, NULL}
};


/* Vendor NIIET devices */
static struct ambapp_device_name NIIET_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor JHUAPL_SRI devices */
static struct ambapp_device_name JHUAPL_SRI_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor JHUAPL_SEE devices */
static struct ambapp_device_name JHUAPL_SEE_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor JHUAPL_SER devices */
static struct ambapp_device_name JHUAPL_SER_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor JHUAPL_SES devices */
static struct ambapp_device_name JHUAPL_SES_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor SEMIBLOCKS devices */
static struct ambapp_device_name SEMIBLOCKS_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor S3 devices */
static struct ambapp_device_name S3_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor UC_BERKELEY devices */
static struct ambapp_device_name UC_BERKELEY_devices[] =
{
  {UC_BERKELEY_ROCKET, "BERKELEY_ROCKET", "RISC-V Rocket Core"},
  {UC_BERKELEY_DEBUG, "BERKELEY_DEBUG", "Rocket Core Debug Module"},
  {0, NULL, NULL}
};


/* Vendor TAS devices */
static struct ambapp_device_name TAS_devices[] =
{
  {TAS_HOUSE_KEEPING_ADC, "HOUSE_KEEPING_ADC", "House-Keeping ADC"},
  {0, NULL, NULL}
};


/* Vendor RECORE devices */
static struct ambapp_device_name RECORE_devices[] =
{
  {RECORE_PROC_SUB_BRDG, "PROC_SUB_BRDG", "Processing Subsystem Bridge"},
  {RECORE_PROC_SUB_DBG, "PROC_SUB_DBG", "Processing Subsystem Debug"},
  {RECORE_XENTIUM_CORE, "XENTIUM_CORE", "Xentium DSP IP core"},
  {RECORE_XENTIUM_DEBUG, "XENTIUM_DEBUG", "Xentium DSP Debug Support Unit"},
  {RECORE_XENTIUM_ICACHE, "XENTIUM_ICACHE", "Xentium iCache AHB master i/f"},
  {RECORE_XENTIUM_DEVICE, "XENTIUM_DEVICE", "Xentium devices"},
  {0, NULL, NULL}
};


/* Vendor AAC devices */
static struct ambapp_device_name AAC_devices[] =
{
  {AAC_MEMCTRL, "MEMCTRL", "SDRAM Memory controller"},
  {AAC_SOCINFO, "SOCINFO", "System-on-Chip info"},
  {AAC_GPIO, "GPIO", "General Purpose I/O"},
  {AAC_WDT, "WDT", "Watchdog timer"},
  {AAC_NVRAM, "NVRAM", "Non-volatile RAM"},
  {AAC_ERRMAN, "ERRMAN", "Error manager"},
  {AAC_SCET, "SCET", "SpaceCraft Elapsed Timer"},
  {AAC_NANDFLASH, "NANDFLASH", "System Flash for boot images"},
  {AAC_ADC, "ADC", "Analog to Digital Converter"},
  {AAC_SPW, "SPW", "SpaceWire interface with DMA"},
  {AAC_MM, "MM", "Mass memory flash with DMA"},
  {AAC_CCSDS, "CCSDS", "CCSDS TM/TC with DMA"},
  {AAC_UART, "UART", "16550D compatible UART"},
  {AAC_I2C, "I2C", "I2C master/slave"},
  {AAC_ETHMAC, "ETHMAC", "Ethernet MAC with DMA"},
  {AAC_CAN, "CAN", "CAN bus controller"},
  {AAC_USB, "USB", "USB slave"},
  {AAC_SPI, "SPI", "SPI master with DMA"},
  {AAC_CUSTOM1, "CUSTOM1", "Custom IP core type 1"},
  {AAC_CUSTOM2, "CUSTOM2", "Custom IP core type 2"},
  {AAC_CUSTOM3, "CUSTOM3", "Custom IP core type 3"},
  {0, NULL, NULL}
};


/* Vendor ACTEL devices */
static struct ambapp_device_name ACTEL_devices[] =
{
  {ACTEL_COREMP7, "COREMP7", "CoreMP7 Processor"},
  {ACTEL_RTG4FDDRCE, "RTG4FDDRCE", "RTG4 FDDR East Controller"},
  {ACTEL_RTG4FDDRCW, "RTG4FDDRCW", "RTG4 FDDR West Controller"},
  {ACTEL_IGLOO2_BRIDGE, "IGLOO2_BRIDGE", "Microsemi SF2/IGLOO2 MSS/HPMS"},
  {ACTEL_MDDR, "MDDR", "MDDR Bridge"},
  {ACTEL_APB3SLV, "APB3SLV", "Generic APB3 Slave Interface"},
  {ACTEL_SERDES, "SERDES", "SERDES Interface"},
  {ACTEL_FICSLV, "FICSLV", "FIC Slave Wrapper"},
  {ACTEL_FICMST, "FICMST", "FIC Master Wrapper"},
  {ACTEL_RTG4SERDES, "RTG4SERDES", "RTG4 SERDES Interface"},
  {ACTEL_PFFDDR3, "PFFDDR3", "PolarFire FDDR3 Controller"},
  {ACTEL_PFFDDR4, "PFFDDR4", "PolarFire FDDR4 Controller"},
  {0, NULL, NULL}
};


/* Vendor APPLECORE devices */
static struct ambapp_device_name APPLECORE_devices[] =
{
  {APPLECORE_UTLEON3, "UTLEON3", "AppleCore uT-LEON3 Processor"},
  {APPLECORE_UTLEON3DSU, "UTLEON3DSU", "AppleCore uT-LEON3 DSU"},
  {APPLECORE_APBPERFCNT, "APBPERFCNT", ""},
  {0, NULL, NULL}
};


/* Vendor C3E devices */
static struct ambapp_device_name C3E_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor CBKPAN devices */
static struct ambapp_device_name CBKPAN_devices[] =
{
  {CBKPAN_FTNANDCTRL, "FTNANDCTRL", "NAND FLASH controller w/DMA"},
  {CBKPAN_FTEEPROMCTRL, "FTEEPROMCTRL", "Fault Toler. EEPROM Controller"},
  {CBKPAN_FTSDCTRL16, "FTSDCTRL16", "Fault Toler. 16-bit SDRAM Ctrl."},
  {CBKPAN_STIXCTRL, "STIXCTRL", "SolO/STIX IDPU dedicated ctrl."},
  {0, NULL, NULL}
};


/* Vendor CAL devices */
static struct ambapp_device_name CAL_devices[] =
{
  {CAL_DDRCTRL, "DDRCTRL", ""},
  {0, NULL, NULL}
};


/* Vendor CETON devices */
static struct ambapp_device_name CETON_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor SSTL devices */
static struct ambapp_device_name SSTL_devices[] =
{
  {SSTL_HDLC, "HDLC", "HDLC Controller"},
  {SSTL_INICAN, "INICAN", ""},
  {SSTL_ZERO_MEM, "ZERO_MEM", ""},
  {0, NULL, NULL}
};


/* Vendor EMBEDDIT devices */
static struct ambapp_device_name EMBEDDIT_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor NASA_GSFC devices */
static struct ambapp_device_name NASA_GSFC_devices[] =
{
  {0, NULL, NULL}
};


/* Vendor AZST devices */
static struct ambapp_device_name AZST_devices[] =
{
  {0, NULL, NULL}
};


/* Vendors and their devices */
static struct ambapp_ids vendors[] = 
{
  {VENDOR_RESERVED, "RESERVED", "", RESERVED_devices},
  {VENDOR_GAISLER, "GAISLER", "Cobham Gaisler", GAISLER_devices},
  {VENDOR_PENDER, "PENDER", "", PENDER_devices},
  {VENDOR_ESA, "ESA", "European Space Agency", ESA_devices},
  {VENDOR_ASTRIUM, "ASTRIUM", "", ASTRIUM_devices},
  {VENDOR_OPENCHIP, "OPENCHIP", "OpenChip", OPENCHIP_devices},
  {VENDOR_OPENCORES, "OPENCORES", "OpenCores", OPENCORES_devices},
  {VENDOR_CONTRIB, "CONTRIB", "Various contributions", CONTRIB_devices},
  {VENDOR_DLR, "DLR", "German Aerospace Center", DLR_devices},
  {VENDOR_EONIC, "EONIC", "Eonic BV", EONIC_devices},
  {VENDOR_TELECOMPT, "TELECOMPT", "Telecom ParisTech", TELECOMPT_devices},
  {VENDOR_DTU, "DTU", "DTU Space", DTU_devices},
  {VENDOR_BSC, "BSC", "BSC", BSC_devices},
  {VENDOR_RADIONOR, "RADIONOR", "Radionor Communications", RADIONOR_devices},
  {VENDOR_GLEICHMANN, "GLEICHMANN", "Gleichmann Electronics", GLEICHMANN_devices},
  {VENDOR_MENTA, "MENTA", "Menta", MENTA_devices},
  {VENDOR_SUN, "SUN", "Sun Microsystems", SUN_devices},
  {VENDOR_MOVIDIA, "MOVIDIA", "", MOVIDIA_devices},
  {VENDOR_L3T, "L3T", "L3 Technologies", L3T_devices},
  {VENDOR_ORBITA, "ORBITA", "Orbita", ORBITA_devices},
  {VENDOR_SYNOPSYS, "SYNOPSYS", "Synopsys Inc.", SYNOPSYS_devices},
  {VENDOR_NASA, "NASA", "NASA", NASA_devices},
  {VENDOR_NIIET, "NIIET", "NIIET", NIIET_devices},
  {VENDOR_JHUAPL_SRI, "JHUAPL_SRI", "JHUAPL Space Exploration", JHUAPL_SRI_devices},
  {VENDOR_JHUAPL_SEE, "JHUAPL_SEE", "JHUAPL Space Exploration", JHUAPL_SEE_devices},
  {VENDOR_JHUAPL_SER, "JHUAPL_SER", "JHUAPL Space Exploration", JHUAPL_SER_devices},
  {VENDOR_JHUAPL_SES, "JHUAPL_SES", "JHUAPL Space Exploration", JHUAPL_SES_devices},
  {VENDOR_SEMIBLOCKS, "SEMIBLOCKS", "SemiBlocks B.V.", SEMIBLOCKS_devices},
  {VENDOR_S3, "S3", "S3 Group", S3_devices},
  {VENDOR_UC_BERKELEY, "UC_BERKELEY", "UC, Berkeley", UC_BERKELEY_devices},
  {VENDOR_TAS, "TAS", "Thales Alenia Space", TAS_devices},
  {VENDOR_RECORE, "RECORE", "Recore Systems", RECORE_devices},
  {VENDOR_AAC, "AAC", "AAC Microtec", AAC_devices},
  {VENDOR_ACTEL, "ACTEL", "Microsemi Corporation", ACTEL_devices},
  {VENDOR_APPLECORE, "APPLECORE", "AppleCore", APPLECORE_devices},
  {VENDOR_C3E, "C3E", "TU Braunschweig C3E", C3E_devices},
  {VENDOR_CBKPAN, "CBKPAN", "CBK PAN", CBKPAN_devices},
  {VENDOR_CAL, "CAL", "", CAL_devices},
  {VENDOR_CETON, "CETON", "Ceton Corporation", CETON_devices},
  {VENDOR_SSTL, "SSTL", "SSTL", SSTL_devices},
  {VENDOR_EMBEDDIT, "EMBEDDIT", "Embedd.it", EMBEDDIT_devices},
  {VENDOR_NASA_GSFC, "NASA_GSFC", "NASA GSFC", NASA_GSFC_devices},
  {VENDOR_AZST, "AZST", "AZST", AZST_devices},
  {0, NULL, NULL, NULL}
};

struct ambapp_ids *ambapp_ids = &vendors[0];

/* Translate a device id to a human readable string
 * [in] ids       Pointer to a table if AMBA PnP IDs
 * [in] vendor    Vendor id of the device
 * [in] id        Device id of the device
 * [ret] Human readable string
 */
const char *ambapp_device_id2str(struct ambapp_ids *ids, int vendor, int id)
{
	struct ambapp_ids *ven = ids;
	while( ven->name ){
		if ( ven->vendor_id == vendor ){
			struct ambapp_device_name *devs = ven->devices;
			while( devs->name ){
				if ( devs->device_id == id )
					return devs->name;
				devs++;
			}
			return NULL;
		}
		ven++;
	}
	return NULL;
}

/* Translate a vendor id to a human readable string
 * [in] ids       Pointer to a table if AMBA PnP IDs
 * [in] vendor    A vendor id
 * [ret] Human readable string
 */
const char *ambapp_vendor_id2str(struct ambapp_ids *ids, int vendor)
{
	struct ambapp_ids *ven = ids;
	while( ven->name ){
		if ( ven->vendor_id == vendor ){
			return ven->name;
		}
		ven++;
	}
	return NULL;
}

/* Translate a device id to a human readable description
 * [in] ids       Pointer to a table if AMBA PnP IDs
 * [in] vendor    Vendor id of the device
 * [in] id        Device id of the device
 * [ret] Human readable string
 */
const char *ambapp_device_id2desc(struct ambapp_ids *ids, int vendor, int id)
{
	struct ambapp_ids *ven = ids;
	while( ven->name ){
		if ( ven->vendor_id == vendor ){
			struct ambapp_device_name *devs = ven->devices;
			while( devs->name ){
				if ( devs->device_id == id )
					return devs->desc;
				devs++;
			}
			return NULL;
		}
		ven++;
	}
	return NULL;
}

/* Translate a vendor id to a human readable description
 * [in] ids       Pointer to a table if AMBA PnP IDs
 * [in] vendor    A vendor id
 * [ret] Human readable string
 */
const char *ambapp_vendor_id2desc(struct ambapp_ids *ids, int vendor)
{
	struct ambapp_ids *ven = ids;
	while( ven->name ){
		if ( ven->vendor_id == vendor ){
			return ven->desc;
		}
		ven++;
	}
	return NULL;
}


