/* CH32V307VCT6, default split: 256K Flash and 64K SRAM. The user option
 * word can select another split (see the datasheet, note 2).
 *
 * The image links at the flash alias at 0, as the boot from the user flash
 * maps it. FLASH parameters: give them with -Wl,--defsym=<symbol>=<bytes>
 * before the scripts (-T). A --defsym after a script comes too late, and ld
 * silently uses the default.
 *   __flash_offset  Flash in front of the image, for example for a
 *                   bootloader. Default 0.
 *   __flash_length  Length of FLASH. Default: from the image start to the
 *                   end of the 256K zero-wait flash. */
__flash_origin = 0x00000000 + (DEFINED(__flash_offset) ? __flash_offset : 0);

MEMORY
{
	FLASH (rx) : ORIGIN = __flash_origin, LENGTH = DEFINED(__flash_length) ? __flash_length : 256K - __flash_origin
	RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 64K
}
