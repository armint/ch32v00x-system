/* CH32V307VCT6, default split: 256K Flash and 64K SRAM. The user option
 * word can select another split (see the datasheet, note 2). */
MEMORY
{
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
	RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 64K
}
