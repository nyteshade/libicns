/*
File:       icns_utils.c
Copyright (C) 2001-2013 Mathew Eis <mathew@eisbox.net>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>


#include "icns.h"
#include "icns_internals.h"

/********* The constant type names usable in code ***********/

const icns_type_t kICNSTypes[] = {
  0x49434E23, // "ICN#"
  0x49434F4E, // "ICON"
  0x544F4320, // "TOC "
  0x64726F70, // "drop"
  0x68386D6B, // "h8mk"
	0x69633034, // "ic04"
	0x69633035, // "ic05"
  0x69633037, // "ic07"
  0x69633038, // "ic08"
  0x69633039, // "ic09"
  0x69633130, // "ic10"
  0x69633131, // "ic11"
  0x69633132, // "ic12"
  0x69633133, // "ic13"
  0x69633134, // "ic14"
  0x69636823, // "ich#"
  0x69636834, // "ich4"
  0x69636838, // "ich8"
  0x69636C34, // "icl4"
  0x69636C38, // "icl8"
  0x69636D23, // "icm#"
  0x69636D34, // "icm4"
  0x69636D38, // "icm8"
  0x69636E56, // "icnV"
  0x69636E73, // "icns"
	0x69637034, // "icp4"
	0x69637035, // "icp5"
	0x69637036, // "icp6"
  0x69637323, // "ics#"
  0x69637334, // "ics4"
  0x69637338, // "ics8"
	0x69637362, // "icsb"
	0x69637342, // "icsB"
  0x69683332, // "ih32"
  0x696C3332, // "il32"
	0x696E666F, // "info"
  0x69733332, // "is32"
  0x69743332, // "it32"
  0x6C386D6B, // "l8mk"
  0x6D42494E, // "mBIN"
	0x6E616D65, // "name"
  0x6F647270, // "odrp"
  0x6F70656E, // "open"
  0x6F766572, // "over"
  0x73386D6B, // "s8mk"
	0x73623234, // "sb24"
	0x53423234, // "SB24"
	0x73627470, // "sbtp"
	0x736C6374, // "slct"
  0x74386D6B, // "t8mk"
  0x74696C65, // "tile"
  0xFDD92FA8  // "\xFD\xD9\x2F\xA8"
};

const char *kICNSTypeNames[] = {
  "ICN#",             // 0x49434E23
  "ICON",             // 0x49434F4E
  "TOC ",             // 0x544F4320
  "drop",             // 0x64726F70
  "h8mk",             // 0x68386D6B
	"ic04",							// 0x69633034
	"ic05",							// 0x69633035
  "ic07",             // 0x69633037
  "ic08",             // 0x69633038
  "ic09",             // 0x69633039
  "ic10",             // 0x69633130
  "ic11",             // 0x69633131
  "ic12",             // 0x69633132
  "ic13",             // 0x69633133
  "ic14",             // 0x69633134
  "ich#",             // 0x69636823
  "ich4",             // 0x69636834
  "ich8",             // 0x69636838
  "icl4",             // 0x69636C34
  "icl8",             // 0x69636C38
  "icm#",             // 0x69636D23
  "icm4",             // 0x69636D34
  "icm8",             // 0x69636D38
  "icnV",             // 0x69636E56
  "icns",             // 0x69636E73
	"icp4", 						// 0x69637034
	"icp5",							// 0x69637035
	"icp6",							// 0x69637036
  "ics#",             // 0x69637323
  "ics4",             // 0x69637334
  "ics8",             // 0x69637338
	"icsb",							// 0x69637362
	"icsB",             // 0x69637342
  "ih32",             // 0x69683332
  "il32",             // 0x696C3332
	"info",             // 0x696E666F
  "is32",             // 0x69733332
  "it32",             // 0x69743332
  "l8mk",             // 0x6C386D6B
  "mBIN",             // 0x6D42494E
	"name",             // 0x6E616D65
  "odrp",             // 0x6F647270
  "open",             // 0x6F70656E
  "over",             // 0x6F766572
  "s8mk",             // 0x73386D6B
	"sb24",							// 0x73623234
	"SB24",							// 0x53423234
	"sbtp",             // 0x73627470
	"slct",             // 0x736C6374
  "t8mk",             // 0x74386D6B
  "tile",             // 0x74696C65
  "\xFD\xD9\x2F\xA8"  // 0xFDD92FA8
};

const icns_size_t kICNSTypesCount = 52;
const icns_size_t kICNSTypeNamesCount = 52;

const icns_bool_t kICNSTrue = 1;
const icns_bool_t kICNSFalse = 0;

/********* This variable is intentionally global ************/
/********* scope is the internals of the icns library *******/
#ifdef ICNS_DEBUG
icns_bool_t	gShouldPrintErrors = 1;
#else
icns_bool_t	gShouldPrintErrors = 0;
#endif

icns_uint32_t icns_get_element_order(icns_type_t iconType)
{
	// Note: 1 bit mask is 'excluded' as
	// 1 bit data and mask ID's are equal
	// data stored in the same element
	switch(iconType)
	{
	case ICNS_DARK_MODE_DATA:
	  return 101;
	case ICNS_ICON_VERSION:
		return 100;
	case ICNS_512x512_2X_32BIT_ARGB_DATA:
	//case ICNS_1024x1024_32BIT_ARGB_DATA:
		return 27;
	case ICNS_256x256_2X_32BIT_ARGB_DATA:
		return 26;
	case ICNS_128x128_32BIT_ARGB_DATA:
	case ICNS_128x128_2X_32BIT_ARGB_DATA:
		return 25;
	case ICNS_32x32_2X_32BIT_ARGB_DATA:
		return 24;
	case ICNS_16x16_2X_32BIT_ARGB_DATA:
		return 23;
	case ICNS_512x512_32BIT_ARGB_DATA:
		return 22;
	case ICNS_256x256_32BIT_ARGB_DATA:
		return 21;
	case ICNS_128X128_8BIT_MASK:
		return 20;
	case ICNS_128X128_32BIT_DATA:
		return 19;
	case ICNS_48x48_8BIT_MASK:
		return 18;
	case ICNS_48x48_32BIT_DATA:
		return 17;
	case ICNS_48x48_8BIT_DATA:
		return 16;
	case ICNS_48x48_4BIT_DATA:
		return 15;
	case ICNS_48x48_1BIT_DATA:
		return 14;
	case ICNS_32x32_8BIT_MASK:
		return 13;
	case ICNS_32x32_32BIT_DATA:
		return 12;
	case ICNS_32x32_8BIT_DATA:
		return 11;
	case ICNS_32x32_4BIT_DATA:
		return 10;
	case ICNS_32x32_1BIT_DATA:
		return 9;
	case ICNS_16x16_8BIT_MASK:
		return 8;
	case ICNS_16x16_32BIT_DATA:
		return 7;
	case ICNS_16x16_8BIT_DATA:
		return 6;
	case ICNS_16x16_4BIT_DATA:
		return 5;
	case ICNS_16x16_1BIT_DATA:
		return 4;
	case ICNS_16x12_8BIT_DATA:
		return 3;
	case ICNS_16x12_4BIT_DATA:
		return 2;
	case ICNS_16x12_1BIT_DATA:
		return 1;
	case ICNS_TABLE_OF_CONTENTS:
		return 0;
	default:
		return 1000;
	}
	
	return 1000;
}

icns_type_t icns_get_mask_type_for_icon_type(icns_type_t iconType)
{
	switch(iconType)
	{
	// Obviously the TOC type has no mask
	case ICNS_TABLE_OF_CONTENTS:
		return ICNS_NULL_MASK;

	// Obviously the version type has no mask
	case ICNS_ICON_VERSION:
		return ICNS_NULL_MASK;

	// The dark mode icon type has no mask
	case ICNS_DARK_MODE_DATA:
	  return ICNS_NULL_MASK;
		
	// 32-bit image types > 256x256 - no mask (mask is already in image)
	case ICNS_512x512_2X_32BIT_ARGB_DATA:
	//case ICNS_1024x1024_32BIT_ARGB_DATA:
		return ICNS_NULL_MASK;
	case ICNS_256x256_2X_32BIT_ARGB_DATA:
		return ICNS_NULL_MASK;
	case ICNS_128x128_2X_32BIT_ARGB_DATA:
		return ICNS_NULL_MASK;
	case ICNS_32x32_2X_32BIT_ARGB_DATA:
		return ICNS_NULL_MASK;
	case ICNS_16x16_2X_32BIT_ARGB_DATA:
		return ICNS_NULL_MASK;
	case ICNS_512x512_32BIT_ARGB_DATA:
		return ICNS_NULL_MASK;
	case ICNS_256x256_32BIT_ARGB_DATA:
	case ICNS_128x128_32BIT_ARGB_DATA:
		return ICNS_NULL_MASK;

	// 32-bit image types - 8-bit mask type
	case ICNS_128X128_32BIT_DATA:
		return ICNS_128X128_8BIT_MASK;	
	case ICNS_48x48_32BIT_DATA:
		return ICNS_48x48_8BIT_MASK;	
	case ICNS_32x32_32BIT_DATA:
		return ICNS_32x32_8BIT_MASK;
	case ICNS_16x16_32BIT_DATA:
		return ICNS_16x16_8BIT_MASK;
		
	// 8-bit image types - 1-bit mask types
	case ICNS_48x48_8BIT_DATA:
		return ICNS_48x48_1BIT_MASK;
	case ICNS_32x32_8BIT_DATA:
		return ICNS_32x32_1BIT_MASK;
	case ICNS_16x16_8BIT_DATA:
		return ICNS_16x16_1BIT_MASK;
	case ICNS_16x12_8BIT_DATA:
		return ICNS_16x12_1BIT_MASK;
		
	// 4 bit image types - 1-bit mask types
	case ICNS_48x48_4BIT_DATA:
		return ICNS_48x48_1BIT_MASK;
	case ICNS_32x32_4BIT_DATA:
		return ICNS_32x32_1BIT_MASK;
	case ICNS_16x16_4BIT_DATA:
		return ICNS_16x16_1BIT_MASK;
	case ICNS_16x12_4BIT_DATA:
		return ICNS_16x12_1BIT_MASK;
		
	// 1 bit image types - 1-bit mask types
	case ICNS_48x48_1BIT_DATA:
		return ICNS_48x48_1BIT_MASK;
	case ICNS_32x32_1BIT_DATA:
		return ICNS_32x32_1BIT_MASK;
	case ICNS_16x16_1BIT_DATA:
		return ICNS_16x16_1BIT_MASK;
	case ICNS_16x12_1BIT_DATA:
		return ICNS_16x12_1BIT_MASK;
	default:
		return ICNS_NULL_MASK;
	}
		
	return ICNS_NULL_MASK;
}

icns_icon_info_t icns_get_image_info_for_type(icns_type_t iconType)
{
	icns_icon_info_t iconInfo;
	
	memset(&iconInfo,0,sizeof(iconInfo));
	
	if(iconType == ICNS_NULL_TYPE)
	{
		icns_print_err("icns_get_image_info_for_type: Unable to parse NULL type!\n");
		return iconInfo;
	}
	
	/*
	#ifdef ICNS_DEBUG
	{
		char typeStr[5];
		printf("Retrieving info for type '%s'...\n",icns_type_str(iconType,typeStr));
	}
	#endif
	*/
	
	iconInfo.iconType = iconType;
	
	switch(iconType)
	{
	// TOC type
	case ICNS_TABLE_OF_CONTENTS:
		iconInfo.isImage = 0;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 0;
		iconInfo.iconHeight = 0;
		iconInfo.iconChannels = 0;
		iconInfo.iconPixelDepth = 0;
		iconInfo.iconBitDepth = 0;
		break;
	// Version type
	case ICNS_ICON_VERSION:
		iconInfo.isImage = 0;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 0;
		iconInfo.iconHeight = 0;
		iconInfo.iconChannels = 0;
		iconInfo.iconPixelDepth = 0;
		iconInfo.iconBitDepth = 0;
		break;
	// 32-bit image types
	case ICNS_512x512_2X_32BIT_ARGB_DATA:
	//case ICNS_1024x1024_32BIT_ARGB_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 1024;
		iconInfo.iconHeight = 1024;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
	case ICNS_256x256_2X_32BIT_ARGB_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 512;
		iconInfo.iconHeight = 512;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
	case ICNS_128x128_2X_32BIT_ARGB_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 256;
		iconInfo.iconHeight = 256;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
	case ICNS_32x32_2X_32BIT_ARGB_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 64;
		iconInfo.iconHeight = 64;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
	case ICNS_16x16_2X_32BIT_ARGB_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 32;
		iconInfo.iconHeight = 32;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
	case ICNS_512x512_32BIT_ARGB_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 512;
		iconInfo.iconHeight = 512;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
	case ICNS_128x128_32BIT_ARGB_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 128;
		iconInfo.iconHeight = 128;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
	case ICNS_256x256_32BIT_ARGB_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 256;
		iconInfo.iconHeight = 256;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
	case ICNS_128X128_32BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 128;
		iconInfo.iconHeight = 128;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
	case ICNS_48x48_32BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 48;
		iconInfo.iconHeight = 48;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;	
		break;
	case ICNS_32x32_32BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 32;
		iconInfo.iconHeight = 32;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
	case ICNS_16x16_32BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 16;
		iconInfo.iconHeight = 16;
		iconInfo.iconChannels = 4;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 32;
		break;
		
	// 8-bit mask types
	case ICNS_128X128_8BIT_MASK:
		iconInfo.isImage = 0;
		iconInfo.isMask = 1;
		iconInfo.iconWidth = 128;
		iconInfo.iconHeight = 128;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 8;
		break;
	case ICNS_48x48_8BIT_MASK:
		iconInfo.isImage = 0;
		iconInfo.isMask = 1;
		iconInfo.iconWidth = 48;
		iconInfo.iconHeight = 48;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 8;
		break;
	case ICNS_32x32_8BIT_MASK:
		iconInfo.isImage = 0;
		iconInfo.isMask = 1;
		iconInfo.iconWidth = 32;
		iconInfo.iconHeight = 32;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 8;
		break;
	case ICNS_16x16_8BIT_MASK:
		iconInfo.isImage = 0;
		iconInfo.isMask = 1;
		iconInfo.iconWidth = 16;
		iconInfo.iconHeight = 16;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 8;
		break;
	
	// 8-bit image types
	case ICNS_48x48_8BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 48;
		iconInfo.iconHeight = 48;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 8;
		break;
	case ICNS_32x32_8BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 32;
		iconInfo.iconHeight = 32;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 8;
		break;
	case ICNS_16x16_8BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 16;
		iconInfo.iconHeight = 16;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 8;
		break;
	case ICNS_16x12_8BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 16;
		iconInfo.iconHeight = 12;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 8;
		iconInfo.iconBitDepth = 8;
		break;
		
	// 4 bit image types
	case ICNS_48x48_4BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 48;
		iconInfo.iconHeight = 48;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 4;
		iconInfo.iconBitDepth = 4;
		break;
	case ICNS_32x32_4BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 32;
		iconInfo.iconHeight = 32;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 4;
		iconInfo.iconBitDepth = 4;
		break;
	case ICNS_16x16_4BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 16;
		iconInfo.iconHeight = 16;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 4;
		iconInfo.iconBitDepth = 4;
		break;
	case ICNS_16x12_4BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		iconInfo.iconWidth = 16;
		iconInfo.iconHeight = 12;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 4;
		iconInfo.iconBitDepth = 4;
		break;
		
	// 1 bit image types - same as mask typess
	case ICNS_48x48_1BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 1;
		iconInfo.iconWidth = 48;
		iconInfo.iconHeight = 48;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 1;
		iconInfo.iconBitDepth = 1;
		break;
	case ICNS_32x32_1BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 1;
		iconInfo.iconWidth = 32;
		iconInfo.iconHeight = 32;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 1;
		iconInfo.iconBitDepth = 1;
		break;
	case ICNS_16x16_1BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 1;
		iconInfo.iconWidth = 16;
		iconInfo.iconHeight = 16;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 1;
		iconInfo.iconBitDepth = 1;
		break;
	case ICNS_16x12_1BIT_DATA:
		iconInfo.isImage = 1;
		iconInfo.isMask = 1;
		iconInfo.iconWidth = 16;
		iconInfo.iconHeight = 12;
		iconInfo.iconChannels = 1;
		iconInfo.iconPixelDepth = 1;
		iconInfo.iconBitDepth = 1;
		break;
	default:
		{
			char typeStr[5];
			icns_print_err("icns_get_image_info_for_type: Unable to parse icon type '%s'\n",icns_type_str(iconType,typeStr));
			iconInfo.iconType = ICNS_NULL_TYPE;
		}
		break;
	}
	
	iconInfo.iconRawDataSize = iconInfo.iconHeight * iconInfo.iconWidth * iconInfo.iconBitDepth / ICNS_BYTE_BITS;
	
	/*
	#ifdef ICNS_DEBUG
	{
		char typeStr[5];
		printf("  type is: '%s'\n",icns_type_str(iconInfo.iconType));
		printf("  width is: %d\n",iconInfo.iconWidth);
		printf("  height is: %d\n",iconInfo.iconHeight);
		printf("  channels are: %d\n",iconInfo.iconChannels);
		printf("  pixel depth is: %d\n",iconInfo.iconPixelDepth);
		printf("  bit depth is: %d\n",iconInfo.iconBitDepth);
		printf("  data size is: %d\n",(int)iconInfo.iconRawDataSize);
	}
	#endif
	*/
	
	return iconInfo;
}

icns_type_t	icns_get_type_from_image_info_advanced(icns_icon_info_t iconInfo, icns_bool_t isHiDPI)
{
	// Give our best effort to returning a type from the given information
	// But there is only so much we can't work with...
	if( (iconInfo.isImage == 0) && (iconInfo.isMask == 0) )
		return ICNS_NULL_TYPE;
	
	if( (iconInfo.iconWidth == 0) || (iconInfo.iconHeight == 0) )
	{
		// For some really small sizes, we can tell from just the data size...
		switch(iconInfo.iconRawDataSize)
		{
		case 24:
			// Data is too small to have both
			if( (iconInfo.isImage == 1) && (iconInfo.isMask == 1) )
				return ICNS_NULL_TYPE;
			if( iconInfo.isImage == 1 )
				return ICNS_16x12_1BIT_DATA;
			if( iconInfo.isMask == 1 )
				return ICNS_16x12_1BIT_MASK;
			break;
		case 32:
			// Data is too small to have both
			if( (iconInfo.isImage == 1) && (iconInfo.isMask == 1) )
				return ICNS_NULL_TYPE;
			if( iconInfo.isImage == 1 )
				return ICNS_16x16_1BIT_DATA;
			if( iconInfo.isMask == 1 )
				return ICNS_16x16_1BIT_MASK;
			break;
		default:
			return ICNS_NULL_TYPE;
		}
	}
	
	// We need the bit depth to determine the type for sizes < 128
	if( (iconInfo.iconBitDepth == 0) && (iconInfo.iconWidth < 128 || iconInfo.iconHeight < 128) )
	{
		if(iconInfo.iconPixelDepth == 0 || iconInfo.iconChannels == 0)
			return ICNS_NULL_TYPE;
		else
			iconInfo.iconBitDepth = iconInfo.iconPixelDepth * iconInfo.iconChannels;
	}
	
	// Special case for these mini icons
	if(iconInfo.iconWidth == 16 && iconInfo.iconHeight == 12)
	{
		switch(iconInfo.iconBitDepth)
		{
		case 1:
			if(iconInfo.isImage == 1)
				return ICNS_16x12_1BIT_DATA;
			if(iconInfo.isMask == 1)
				return ICNS_16x12_1BIT_MASK;
			break;
		case 4:
			return ICNS_16x12_4BIT_DATA;
			break;
		case 8:
			return ICNS_16x12_8BIT_DATA;
			break;
		default:
			return ICNS_NULL_TYPE;
			break;
		}
	}
	
	// Width must equal hieght from here on...
	if(iconInfo.iconWidth != iconInfo.iconHeight)
		return ICNS_NULL_TYPE;
	
	switch(iconInfo.iconWidth)
	{
	case 16:
		switch(iconInfo.iconBitDepth)
		{
		case 1:
			if(iconInfo.isImage == 1)
				return ICNS_16x16_1BIT_DATA;
			if(iconInfo.isMask == 1)
				return ICNS_16x16_1BIT_MASK;
			break;
		case 4:
			return ICNS_16x16_4BIT_DATA;
			break;
		case 8:
			if(iconInfo.isImage == 1)
				return ICNS_16x16_8BIT_DATA;
			if(iconInfo.isMask == 1)
				return ICNS_16x16_8BIT_MASK;
			break;
		case 32:
			return ICNS_16x16_32BIT_DATA;
			break;
		default:
			return ICNS_NULL_TYPE;
			break;
		}
		break;
	case 32:
		switch(iconInfo.iconBitDepth)
		{
		case 1:
			if(iconInfo.isImage == 1)
				return ICNS_32x32_1BIT_DATA;
			if(iconInfo.isMask == 1)
				return ICNS_32x32_1BIT_MASK;
			break;
		case 4:
			return ICNS_32x32_4BIT_DATA;
			break;
		case 8:
			if(iconInfo.isImage == 1)
				return ICNS_32x32_8BIT_DATA;
			if(iconInfo.isMask == 1)
				return ICNS_32x32_8BIT_MASK;
			break;
		case 32:
		  if(isHiDPI)
		    return ICNS_16x16_2X_32BIT_ARGB_DATA;
			else
			  return ICNS_32x32_32BIT_DATA;
			break;
		default:
			return ICNS_NULL_TYPE;
			break;
		}
		break;
	case 48:
		switch(iconInfo.iconBitDepth)
		{
		case 1:
			if(iconInfo.isImage == 1)
				return ICNS_48x48_1BIT_DATA;
			if(iconInfo.isMask == 1)
				return ICNS_48x48_1BIT_MASK;
			break;
		case 4:
			return ICNS_48x48_4BIT_DATA;
			break;
		case 8:
			if(iconInfo.isImage == 1)
				return ICNS_48x48_8BIT_DATA;
			if(iconInfo.isMask == 1)
				return ICNS_48x48_8BIT_MASK;
			break;
		case 32:
			return ICNS_48x48_32BIT_DATA;
			break;
		default:
			return ICNS_NULL_TYPE;
			break;
		}
		break;
	case 64:
	  if(iconInfo.iconBitDepth == 32)
	    return ICNS_32x32_2X_32BIT_ARGB_DATA;
	  else
	    return ICNS_NULL_TYPE;
	  break;
	case 128:
		if(iconInfo.isImage == 1 || iconInfo.iconBitDepth == 32)
			return ICNS_128x128_32BIT_ARGB_DATA;
		if(iconInfo.isMask == 1 || iconInfo.iconBitDepth == 8)
			return ICNS_128X128_8BIT_MASK;
		break;
	case 256:
    if(isHiDPI)
      return ICNS_128x128_2X_32BIT_ARGB_DATA;
    else
      return ICNS_256x256_32BIT_ARGB_DATA;
		break;
	case 512:
    if(isHiDPI)
      return ICNS_256x256_2X_32BIT_ARGB_DATA;
    else
      return ICNS_512x512_32BIT_ARGB_DATA;
		break;
	case 1024:
    if(isHiDPI)
      return ICNS_512x512_2X_32BIT_ARGB_DATA;
    else
      return ICNS_1024x1024_32BIT_ARGB_DATA;
		break;
		
	}
	
	return ICNS_NULL_TYPE;
}

icns_type_t	icns_get_type_from_image_info(icns_icon_info_t iconInfo)
{
  return icns_get_type_from_image_info_advanced(iconInfo,0);
}

icns_type_t	icns_get_type_from_image(icns_image_t iconImage)
{
	icns_icon_info_t		iconInfo;
	
	iconInfo.iconWidth = iconImage.imageWidth;
	iconInfo.iconHeight = iconImage.imageHeight;
	iconInfo.iconChannels = iconImage.imageChannels;
	iconInfo.iconPixelDepth = iconImage.imagePixelDepth;
	iconInfo.iconBitDepth = iconInfo.iconPixelDepth * iconInfo.iconChannels;
	iconInfo.iconRawDataSize = iconImage.imageDataSize;
	
	if(iconInfo.iconBitDepth == 1)
	{
		int	flatSize = ((iconInfo.iconWidth * iconInfo.iconHeight) / 8);
		if(iconInfo.iconRawDataSize == (flatSize * 2) )
		{
		iconInfo.isImage = 1;
		iconInfo.isMask = 1;
		}
		else
		{
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
		}
	}
	else
	{
		iconInfo.isImage = 1;
		iconInfo.isMask = 0;
	}
	
	return icns_get_type_from_image_info(iconInfo);
}

icns_type_t	icns_get_type_from_mask(icns_image_t iconImage)
{
	icns_icon_info_t		iconInfo;
	
	iconInfo.iconWidth = iconImage.imageWidth;
	iconInfo.iconHeight = iconImage.imageHeight;
	iconInfo.iconChannels = iconImage.imageChannels;
	iconInfo.iconPixelDepth = iconImage.imagePixelDepth;
	iconInfo.iconBitDepth = iconInfo.iconPixelDepth * iconInfo.iconChannels;
	iconInfo.iconRawDataSize = iconImage.imageDataSize;
	
	if(iconInfo.iconBitDepth == 1)
	{
		int	flatSize = ((iconInfo.iconWidth * iconInfo.iconHeight) / 8);
		if(iconInfo.iconRawDataSize == (flatSize * 2) )
		{
		iconInfo.isImage = 1;
		iconInfo.isMask = 1;
		}
		else
		{
		iconInfo.isImage = 0;
		iconInfo.isMask = 1;
		}
	}
	else
	{
		iconInfo.isImage = 0;
		iconInfo.isMask = 1;
	}
	
	return icns_get_type_from_image_info(iconInfo);
}

icns_bool_t icns_get_is_hidpi(icns_type_t iconType)
{
	switch(iconType)
	{
	case ICNS_256x256_2X_32BIT_ARGB_DATA:
		return 1;
	case ICNS_128x128_2X_32BIT_ARGB_DATA:
		return 1;
	case ICNS_32x32_2X_32BIT_ARGB_DATA:
		return 1;
	case ICNS_16x16_2X_32BIT_ARGB_DATA:
		return 1;
	case ICNS_512x512_2X_32BIT_ARGB_DATA:
		return 1;
	default:
	  return 0;
	}
}

icns_bool_t icns_types_equal(icns_type_t typeA,icns_type_t typeB)
{
	if(memcmp(&typeA, &typeB, sizeof(icns_type_t)) == 0)
		return 1;
	else
		return 0;
}

// This is is largely for conveniance and readability
icns_bool_t icns_types_not_equal(icns_type_t typeA,icns_type_t typeB)
{
	if(memcmp(&typeA, &typeB, sizeof(icns_type_t)) != 0)
		return 1;
	else
		return 0;
}

icns_bool_t icns_known_type(icns_type_t iconType)
{
	icns_uint8_t i = 0;
	icns_bool_t known = 0;

	for (i = 0; i < kICNSTypesCount; i++)
	{
		if(iconType == kICNSTypes[i])
		{
			known = 1;
			break;
		}
	}

	return known;
}

icns_bool_t icns_known_type_string(const char* iconType)
{
	icns_uint8_t i = 0;
	icns_bool_t known = 0;

	for (i = 0; i < kICNSTypeNamesCount; i++)
	{
		if(strcmp(iconType, kICNSTypeNames[i]) == 0)
		{
			known = 1;
			break;
		}
	}

	return known;
}

const char * icns_type_str(icns_type_t type, char *strbuf)
{
	if(strbuf != NULL)
	{
		uint32_t v = *((uint32_t *)(&type));
		strbuf[0] = v >> 24;
		strbuf[1] = v >> 16;
		strbuf[2] = v >> 8;
		strbuf[3] = v;
		strbuf[4] = 0;
		return (const char *)strbuf;
	}
	return NULL;
}

void icns_set_print_errors(icns_bool_t shouldPrint)
{
	#ifdef ICNS_DEBUG
		if(shouldPrint == 0) {
			icns_print_err("Debugging enabled - error message status cannot be disabled!\n");
		}
	#else
		gShouldPrintErrors = shouldPrint;
	#endif
}

void icns_print_err(const char *template, ...)
{
	va_list ap;
	
	#ifdef ICNS_DEBUG
	printf ( "libicns: ");
	va_start (ap, template);
	vprintf (template, ap);
	va_end (ap);
	#else
	if(gShouldPrintErrors)
	{
		fprintf (stderr, "libicns: ");
		va_start (ap, template);
		vfprintf (stderr, template, ap);
		va_end (ap);
	}
	#endif
}

void icns_print_dbg(const char* prefix, const char* template, ...)
{	
	#ifdef ICNS_DEBUG
	va_list ap;

	if(prefix != NULL)
	{
		printf ("%s ", prefix);
	}
	va_start (ap, template);
	vprintf (template, ap);
	va_end (ap);
	#endif
}

void memcpy_be(void* output, const void* input, size_t n) {
    const uint8_t* inputBytes = (const uint8_t*)input;
    uint8_t* outputBytes = (uint8_t*)output;

    for (size_t i = 0; i < n; i++) {
        outputBytes[i] = inputBytes[(n - 1 - i)];
    }
}

void memcpy_le(void* output, const void* input, size_t n) {
	memcpy(output, input, n);
}

void memcpy_var(void* output, const void* input, size_t n, icns_bool_t isBigEndian)
{
	if(isBigEndian)
		memcpy_be(output, input, n);
	else 
		memcpy_le(output, input, n);
}
