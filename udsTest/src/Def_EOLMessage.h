
#include <stdint.h>


#pragma once


#pragma pack (push,1)

/// Bi-directional Request/Response

typedef enum enEOL_ID1
{
	EOL_ID1_UART_CMD = 0x02,
	EOL_ID1_UART_ACK = 0x82,
	EOL_ID1_UART_NCK = 0x92,
	EOL_ID1_CMD		 = 0x04,
	EOL_ID1_ACK		 = 0x84,
	EOL_ID1_NCK		 = 0x94,
} EN_EOL_ID1;

typedef enum enEOL_ID2
{
	EOL_ID2_CAPTURE_IMG  = 0x70,
	EOL_ID2_IRLED_SWITCH = 0x71,
	EOL_ID2_READ_EEPROM  = 0x72,
	EOL_ID2_WRITE_EEPROM = 0x73,
	EOL_ID2_INSPECTION   = 0x74,
	EOL_ID2_CALIBRATION  = 0x75,
	EOL_ID2_EOL_SWITCH   = 0x76,
} EN_EOL_ID2;

// S <-> C
typedef struct _tag_EOL_Command_Opt
{
	uint8_t		id1;	// EN_EOL_ID1
	uint8_t		id2;	// EN_EOL_ID2
	char		data[0];
} ST_EOL_Command_Opt;

// S <- C
typedef struct _tag_EOL_NCK_Opt
{
	uint8_t		code;
} ST_EOL_NCK_Opt;


/// Image Capture Request/Response

typedef enum enEOL_CapImg_Opt
{
	EOL_IMG_CAPTURE = 0x00,
	EOL_IMG_DELETE  = 0x01,
} EN_EOL_CAPIMG_CMD;

typedef enum enEOL_CapImgRes_Opt
{
	EOL_RES_VGA = 0x00,	// 640x480
	EOL_RES_LGE = 0x01,	// 1280x964
} EN_EOL_CAPIMG_RES;

// S -> C
typedef struct _tag_EOL_ReqCapImg_Opt
{
	uint8_t		cmd;	// EN_EOL_CAPIMG_CMD
} ST_EOL_ReqCapImg_Opt;

typedef struct _tag_EOL_ResCapImgAck_Opt
{
	uint8_t		res;	// EN_EOL_CAPIMG_RES
} ST_EOL_ResCapImgAck_Opt;


/// Calibration Request/Response
/// Image Quality Inspection Request/Response

typedef enum enEOL_Calibration_Opt
{
	EOL_CALIBRATION_DUMY  = 0x00,
	EOL_CALIBRATION_CONER = 0x01,
} EN_EOL_CALIBRATION_CMD;

typedef enum enEOL_Inspection_Opt
{
	EOL_INSPECTION_DUMY			= 0x00,
	EOL_INSPECTION_FIDUCIAL_MARK= 0x01,
	EOL_INSPECTION_EDGELINE		= 0x02,
	EOL_INSPECTION_DEFECT_PIXEL	= 0x03,
	EOL_INSPECTION_SFR			= 0x04,
	EOL_INSPECTION_SNR			= 0x05,
	EOL_INSPECTION_STAIN		= 0x06,
	EOL_INSPECTION_GET_IMAGE	= 0x07,
} EN_EOL_INSPECTION_CMD;

typedef enum enEOL_ImageSelect_Opt
{
	EOL_IMG_LEFT  = 0x00,
	EOL_IMG_RIGHT = 0x01,
} EN_EOL_IMAGE_SEL;

// S -> C
typedef struct _tag_EOL_ReqAlgorithm_Opt
{
	uint8_t		cmd;	// EN_EOL_INSPECTION_CMD
	uint8_t		img;	// EN_EOL_IMAGE_SEL
} ST_EOL_ReqAlgorithm_Opt;


/// EOL Mode on/off Request/Response

typedef enum enEOL_Mode_Opt
{
	EOL_MODE_OFF = 0x00,
	EOL_MODE_ON  = 0x01,
} EN_EOL_MODE_CMD;

// S -> C
typedef struct _tag_EOL_ReqMode_Opt
{
	uint8_t		cmd;	// EN_EOL_MODE_CMD
} ST_EOL_ReqMode_Opt;


/// IRLED Request/Response

typedef enum enEOL_IRLED_Opt
/*
	'0' : OFF(variable current / shutter)
	'1' : current 4.5A / shutter 320us
	'2' : current 4.0A / shutter 448us
	'3' : current 3.5A / shutter 704us
	'4' : current 3A / shutter 1088us
	'5' : current 2.5A / shutter 1472us
	'6' : current 2.5A / shutter 1920us
	'7' : current 2A / shutter 3328us
	'8' : current 2A / shutter 3584us
	'9' : current 2A / shutter 3840us
	'A' : current 2A / shutter 4096us

	'f1' : current 0A / shutter 320us
	'f2' : current 0A / shutter 448us
	'f3' : current 0A / shutter 704us
	'f4' : current 0A / shutter 1088us
	'f5' : current 0A / shutter 1472us
	'f6' : current 0A / shutter 1920us
	'f7' : current 0A / shutter 3328us
	'f8' : current 0A / shutter 4096us
*/
{
	EOL_IRLED_OFF = 0x00,
	EOL_IRLED_320US = 0x01,
	EOL_IRLED_448US = 0x02,
	EOL_IRLED_704US = 0x03,
	EOL_IRLED_1088US = 0x04,
	EOL_IRLED_1472US = 0x05,
	EOL_IRLED_1920US = 0x06,
	EOL_IRLED_3328US = 0x07,
	EOL_IRLED_3584US = 0x08,
	EOL_IRLED_3840US = 0x09,
	EOL_IRLED_4096US = 0x0A,
	EOL_IRLED_320US_0A = 0xF1,
	EOL_IRLED_448US_0A = 0xF2,
	EOL_IRLED_704US_0A = 0xF3,
	EOL_IRLED_1088US_0A = 0xF4,
	EOL_IRLED_1472US_0A = 0xF5,
	EOL_IRLED_1920US_0A = 0xF6,
	EOL_IRLED_3328US_0A = 0xF7,
	EOL_IRLED_4096US_0A = 0xF8,
} EN_EOL_IRLED_CMD;

// S -> C
typedef struct _tag_EOL_IRLED_Opt
{
	uint8_t		cmd;	// EN_EOL_IRLED_CMD
} ST_EOL_IRLED_Opt;


/// EEPROM Read Request/Response

#ifndef WIN32
	typedef float float_t;
#endif

typedef struct _tag_EOL_EEPROMDisplayCenter_Opt
{
	float_t rotX;
	float_t rotY;
	float_t rotZ;
	float_t transX;
	float_t transY;
	float_t transZ;
} EOL_EEPROMDisplayCenter_Opt;

typedef struct _tag_EOL_EEPROMCalib_Opt
{
	float_t imageX;
	float_t imageY;
	struct MasterLens {
		float_t focalX;
		float_t focalY;
		float_t principleX;
		float_t principleY;
		float_t k1;
		float_t k2;
		float_t k3;
		float_t k4;
		float_t k5;
	} master;
	struct SlaveLens {
		float_t focalX;
		float_t focalY;
		float_t principleX;
		float_t principleY;
		float_t k1;
		float_t k2;
		float_t k3;
		float_t k4;
		float_t k5;
	} slave;
	struct SlavePos {
		float_t rotX;
		float_t rotY;
		float_t rotZ;
		float_t transX;
		float_t transY;
		float_t transZ;
	} center;
} EOL_EEPROMCalib_Opt;

typedef enum enEOL_EEPROM_Opt
{
	EOL_EEPROM_CALIBRATION		= 0x00,
	EOL_EEPROM_DISPLAY_CENTER	= 0x01,
	EOL_EEPROM_BARCODE			= 0x02,
} EN_EOL_EEPROM_CMD;

// S -> C
typedef struct _tag_EOL_EEPROMRead_Opt
{
	uint8_t		cmd;	// EN_EOL_EEPROM_CMD
} ST_EOL_EEPROMRead_Opt;

// C -> S
typedef struct _tag_EOL_ResEEPROMReadAck_Opt
{
	uint8_t		cmd;	// EN_EOL_EEPROM_CMD
	uint16_t	dataLength;
	uint8_t		data[0];
} ST_EOL_ResEEPROMReadAck_Opt;


/// EEPROM Write Request/Response

// S -> C
typedef struct _tag_EOL_EEPROMWrite_Opt
{
	uint8_t		cmd;	// EN_EOL_EEPROM_CMD
	uint16_t	dataLength;
	uint8_t		data[0];
} ST_EOL_EEPROMWrite_Opt;

#pragma pack(pop)
