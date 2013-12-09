JDI> nerase 32 16

JDI> nprog 4096 def_boot_config.bin     // Config file for test, size can not larger than 1 page.

JDI> nprog 4097 boot.bin                // The image resource.

After that copy ucos.bin to the tftp_server folder, download it and run it.