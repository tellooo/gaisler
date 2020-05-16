/*
 *  Select PCI Configuration Library
 */
  #ifdef CONFIGURE_INIT
    #define PCI_LIB_NONE 0
    #define PCI_LIB_AUTO 1
    #define PCI_LIB_STATIC 2
    #define PCI_LIB_READ 3
    #define PCI_LIB_PERIPHERAL 4
    #if CONFIGURE_PCI_LIB == PCI_LIB_AUTO
      #define PCI_CFG_AUTO_LIB
      #include <pci/cfg.h>
      struct pci_bus pci_hb;
      #define PCI_LIB_INIT pci_config_auto
      #define PCI_LIB_CONFIG pci_config_auto_register
    #elif CONFIGURE_PCI_LIB == PCI_LIB_STATIC
      #define PCI_CFG_STATIC_LIB
      #include <pci/cfg.h>
      #define PCI_LIB_INIT pci_config_static
      #define PCI_LIB_CONFIG NULL
      /* Let user define PCI configuration (struct pci_bus pci_hb) */
    #elif CONFIGURE_PCI_LIB == PCI_LIB_READ
      #define PCI_CFG_READ_LIB
      #include <pci/cfg.h>
      #define PCI_LIB_INIT pci_config_read
      #define PCI_LIB_CONFIG NULL
      struct pci_bus pci_hb;
    #elif CONFIGURE_PCI_LIB == PCI_LIB_PERIPHERAL
      #define PCI_LIB_INIT pci_config_peripheral
      #define PCI_LIB_CONFIG NULL
      /* Let user define PCI configuration (struct pci_bus pci_hb) */
    #elif CONFIGURE_PCI_LIB == PCI_LIB_NONE
      #define PCI_LIB_INIT NULL
      #define PCI_LIB_CONFIG NULL
      /* No PCI Configuration at all, user can use/debug access routines */
    #else
      #error NO PCI LIBRARY DEFINED
    #endif

    const int pci_config_lib_type = CONFIGURE_PCI_LIB;
    int (*pci_config_lib_init)(void) = PCI_LIB_INIT;
    void (*pci_config_lib_register)(void *config) = PCI_LIB_CONFIG;
  #endif
