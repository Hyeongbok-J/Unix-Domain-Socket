//*****************************************************************************
// Filename	: Def_Communication.h
// Created	: 2018/08/20
// Modified	: 2018/08/20
//
// Author	: KHO
//	
// Purpose	: 
//*****************************************************************************
#ifndef Def_Communication_h__
#define Def_Communication_h__

#define DEBUG_PRINT_LOG

// 알고리즘 종류

#pragma pack (push,1)
typedef enum enInspectionAlgoritm
{
	Inspection_Init = 0,
	Inspection_SFR,
	Inspection_FiducialMark,
	Inspection_EdgeLine,
	Inspection_SNR,
	Inspection_Stain,
	Inspection_DefectPixel,
	Inspection_Corner,
	Inspection_Max,
};

typedef enum enResultCode_AI
{
	RC_AI_Unknown_Code = 0,			// 코드 미설정
	RC_AI_OK,						// 이상 없음
	RC_AI_ImageBuf_NULL,			// 이미지 버퍼가 NULL
	RC_AI_ImageSizeWidth_Wrong,		// 이미지 Width 값이 다름
	RC_AI_ImageSizeHeight_Wrong,	// 이미지 Height 값이 다름
	RC_AI_MsgBuf_NULL,				// Input MSG 버퍼가 NULL
	RC_AI_Algorithm_NoneSelect,		// 알고리즘 항목 선택이 안됨
	RC_AI_Item_Count_Zero,			// ROI나 검사 항목의 갯수 지정이 안됨
	RC_AI_FindMark_Fail,			// 피두셜 마크를 찾지 못함
	RC_AI_ResultBuf_NULL,			// 결과 버퍼를 담을 메모리가 NULL
};

// FiducialMark
typedef enum enFiducialMark_ROI
{
	Mark_ROI_FST = 0,
	Mark_ROI_LST = 14,
	Mark_ROI_Max,
};

typedef enum enFiducialMark_TYPE
{
	Mark_T_BLOCK = 0,
	Mark_T_WHITE,
};

// Edge Line
typedef enum enEdgeLine_ROI
{
	Line_ROI_FST = 0,
	Line_ROI_LST = 9,
	Line_ROI_Max,
};

typedef enum enEdgeLind_TYPE
{
	Line_T_LRWB,
	Line_T_LRBW,
	Line_T_TBWB,
	Line_T_TBBW,
};

// SNR 
typedef enum enSNR_ROI
{
	SNR_ROI_FST = 0,
	SNR_ROI_LST = 73,
	SNR_ROI_Max,
};

// SFR
typedef enum enSFR_ROI
{
	SFR_ROI_FST = 0,
	SFR_ROI_LST = 49,
	SFR_ROI_Max,
};

// Stain
typedef enum enStain_ROI
{
	Stain_ROI_Edge = 0,
	Stain_ROI_VerEdge,
	Stain_ROI_HorEdge,
	Stain_ROI_Center,
	Stain_ROI_Max,
};

// Fail Info
typedef enum enStain_F_ROI
{
	Stain_ROI_F_FST = 0,
	Stain_ROI_F_LST = 29,
	Stain_ROI_F_Max,
};

// Defect Pixel
typedef enum enDefectPixel_Type
{
	Defect_Hot = 0,
	Defect_Dead,
};

// Fail Info
typedef enum enDefectPixel_F_ROI
{
	DefectPixel_F_FST = 0,
	DefectPixel_F_LST = 49,
	DefectPixel_F_Max,
};

typedef enum enCorner
{
	Corner_FST = 0,
	Corner_LST = 99,
	Corner_Max,
};

// Header
typedef struct _tag_SO_Header
{
	// 알고리즘 종류(INSPECTION_ALGORITHM)
	unsigned char	iTestItem;

	_tag_SO_Header()
	{
		iTestItem = Inspection_Init;
	};

}ST_SO_Header;

// ROI
typedef struct _tag_SO_ROI
{
	unsigned short	sLeft;
	unsigned short	sTop;
	unsigned short	sWidth;
	unsigned short	sHeight;

	_tag_SO_ROI()
	{
		sLeft	= 0;
		sTop	= 0;
		sWidth	= 0;
		sHeight = 0;
	};

}ST_SO_ROI;

//-----------------------------------------------------------------------------
// SFR, Opt : PC -> CAMERA , Result : CAMERA -> PC
//-----------------------------------------------------------------------------
typedef struct _tag_SO_SFR_Opt
{
	ST_SO_Header	stHeader;
	ST_SO_ROI		stROI[SFR_ROI_Max];

	float			fPixelSize;
	BYTE			byRoiCnt;
	BYTE			byMode;

	float			fLinePair[SFR_ROI_Max];

	_tag_SO_SFR_Opt()
	{
		stHeader.iTestItem = Inspection_SFR;

		for (int iIdx = 0; iIdx < SFR_ROI_Max; iIdx++)
		{
			fLinePair[iIdx] = 0.0f;
		}

		fPixelSize = 0;
		byRoiCnt = 0;
		byMode = 0;
	};

}ST_SO_SFR_Opt;

typedef struct _tag_SO_SFR_Result
{
	ST_SO_Header	stHeader;
	WORD			wResultCode;

	// ����ϴ� ROI ����
	BYTE byRoiCnt;

	double fValue[SFR_ROI_Max];

	_tag_SO_SFR_Result()
	{
		stHeader.iTestItem = Inspection_SFR;
		wResultCode = RC_AI_Unknown_Code;

		for (int iIdx = 0; iIdx < SFR_ROI_Max; iIdx++)
		{
			fValue[iIdx] = 0.0f;
		}
	};

}ST_SO_SFR_Result;

//-----------------------------------------------------------------------------
// FiducialMark, Opt : PC -> CAMERA , Result : CAMERA -> PC
//-----------------------------------------------------------------------------
typedef struct _tag_SO_FiducialMark_Opt
{
	ST_SO_Header	stHeader;

	BYTE			byRoiCnt;

	ST_SO_ROI		stROI[Mark_ROI_Max];

	BYTE			byMarkColor[Mark_ROI_Max];
	BYTE			byBrightness[Mark_ROI_Max];

	_tag_SO_FiducialMark_Opt()
	{
		stHeader.iTestItem = Inspection_FiducialMark;

		byRoiCnt = 0;

		for (int iIdx = 0; iIdx < Mark_ROI_Max; iIdx++)
		{
			byMarkColor[iIdx] = Mark_T_BLOCK;
			byBrightness[iIdx] = 0;
		}
	};

}ST_SO_FiducialMark_Opt;

typedef struct _tag_SO_FiducialMark_Result
{
	ST_SO_Header	stHeader;
	WORD			wResultCode;

	BYTE			byRoiCnt;

	short  sMarkCenterX[Mark_ROI_Max];
	short  sMarkCenterY[Mark_ROI_Max];

	_tag_SO_FiducialMark_Result()
	{
		stHeader.iTestItem = Inspection_FiducialMark;
		wResultCode = RC_AI_Unknown_Code;

		for (int iIdx = 0; iIdx < Mark_ROI_Max; iIdx++)
		{
			sMarkCenterX[iIdx] = 0;
			sMarkCenterY[iIdx] = 0;
		}
	};

}ST_SO_FiducialMark_Result;

//-----------------------------------------------------------------------------
// EdgeLine, Opt : PC -> CAMERA , Result : CAMERA -> PC
//-----------------------------------------------------------------------------
typedef struct _tag_SO_EdgeLine_Opt
{
	ST_SO_Header	stHeader;

	BYTE			byRoiCnt;

	ST_SO_ROI		stROI[Line_ROI_Max];

	BYTE			byDetectColor[Line_ROI_Max];
	BYTE			byThreshold[Line_ROI_Max];
	BYTE			byBrightness[Line_ROI_Max];

	unsigned short  sCenterX;
	unsigned short  sCenterY;

	_tag_SO_EdgeLine_Opt()
	{
		stHeader.iTestItem = Inspection_EdgeLine;

		byRoiCnt = 0;
		sCenterX = 0;
		sCenterY = 0;

		for (int iIdx = 0; iIdx < Line_ROI_Max; iIdx++)
		{
			byDetectColor[iIdx] = Line_T_LRWB;
			byBrightness[iIdx] = 0;
			byThreshold[iIdx] = 0;
		}
	};

}ST_SO_EdgeLine_Opt;

typedef struct _tag_SO_EdgeLine_Result
{
	ST_SO_Header	stHeader;
	WORD			wResultCode;

	BYTE			byRoiCnt;

	short			sLineCenterX[Line_ROI_Max];
	short			sLineCenterY[Line_ROI_Max];

	_tag_SO_EdgeLine_Result()
	{
		stHeader.iTestItem = Inspection_EdgeLine;
		wResultCode = RC_AI_Unknown_Code;

		for (int iIdx = 0; iIdx < Line_ROI_Max; iIdx++)
		{
			sLineCenterX[iIdx] = 0;
			sLineCenterY[iIdx] = 0;
		}
	};

}ST_SO_EdgeLine_Result;

//-----------------------------------------------------------------------------
// SNR, Opt : PC -> CAMERA , Result : CAMERA -> PC
//-----------------------------------------------------------------------------
typedef struct _tag_SO_SNR_Opt
{
	ST_SO_Header	stHeader;

	BYTE			byRoiCnt;

	ST_SO_ROI		stROI[SNR_ROI_Max];

	_tag_SO_SNR_Opt()
	{
		stHeader.iTestItem = Inspection_SNR;
		byRoiCnt = 0;
	};

}ST_SO_SNR_Opt;

typedef struct _tag_SO_SNR_Result
{
	ST_SO_Header	stHeader;
	WORD			wResultCode;

	BYTE			byRoiCnt;

	unsigned short	sSignal[SNR_ROI_Max];
	float			fNoise[SNR_ROI_Max];

	_tag_SO_SNR_Result()
	{
		stHeader.iTestItem = Inspection_SNR;
		wResultCode = RC_AI_Unknown_Code;

		for (int iIdx = 0; iIdx < SNR_ROI_Max; iIdx++)
		{
			sSignal[iIdx] = 0;
			fNoise[iIdx] = 0.0;
		}
	};

}ST_SO_SNR_Result;

//-----------------------------------------------------------------------------
// Stain, Opt : PC -> CAMERA , Result : CAMERA -> PC
//-----------------------------------------------------------------------------
typedef struct _tag_SO_Stain_Opt
{
	ST_SO_Header stHeader;

	int			iEdgeW;
	int			iEdgeH;
	float		fThreConcentration[Stain_ROI_Max];	// 멍농도 Threshold
	float		fThreSize[Stain_ROI_Max];			// 멍크기 Threshold
	float		fSensitivity;						// 민감도

	_tag_SO_Stain_Opt()
	{
		stHeader.iTestItem = Inspection_Stain;

		iEdgeW = 0;
		iEdgeH = 0;
		fSensitivity = 0.0f;

		for (int i = 0; i < Stain_ROI_Max; i++)
		{
			fThreConcentration[i] = 0.0f;
			fThreSize[i] = 0.0f;
		}
	};

}ST_SO_Stain_Opt;

typedef struct _tag_SO_Stain_Result
{
	ST_SO_Header	stHeader;
	WORD			wResultCode;

	BYTE			byRoiCnt;
	ST_SO_ROI		stROI[Stain_ROI_F_Max];				// 발견한 오브젝트
	float			fConcentration[Stain_ROI_F_Max];	// 농도

	_tag_SO_Stain_Result()
	{
		stHeader.iTestItem	= Inspection_Stain;
		wResultCode			= RC_AI_Unknown_Code;

		byRoiCnt = 0;

		for (int i = 0; i < Stain_ROI_F_Max; i++)
		{
			fConcentration[i] = 0.0f;
		}
	};

}ST_SO_Stain_Result;

//-----------------------------------------------------------------------------
// Defect Pixel, Opt : PC -> CAMERA , Result : CAMERA -> PC
//-----------------------------------------------------------------------------
typedef struct _tag_SO_DefectPixel_Opt
{
	ST_SO_Header stHeader;
	
	bool	bType;				// 검출 타입
	float	fVeryHot_Thres;
	float	fHot_Thres;
	float	fVeryBright_Thres;
	float	fBright_Thres;
	float	fVeryDark_Thres;
	float	fDark_Thres;
	BYTE	byBlockSize;		// Block Size
	BYTE	byStepPixel;		// Step Pixel��

	_tag_SO_DefectPixel_Opt()
	{
		stHeader.iTestItem	= Inspection_DefectPixel;
		bType				= Defect_Hot;				
		fVeryHot_Thres		= 160.0;
		fHot_Thres			= 80.0;
		fVeryBright_Thres	= 50.0;
		fBright_Thres		= 25.0;
		fVeryDark_Thres		= -50.0;
		fDark_Thres			= -25.0;
		byBlockSize			= 1;						
		byStepPixel			= 1;						
	};

}ST_SO_DefectPixel_Opt;

typedef struct _tag_SO_DefectPixel_Result
{
	ST_SO_Header	stHeader;
	WORD			wResultCode;

	BYTE			byIndexCnt;							// 찾은 수량

	unsigned short	sPointX[DefectPixel_F_Max];			// Defect Pixel의 X좌표
	unsigned short	sPointY[DefectPixel_F_Max];			// Defect Pixel의 Y좌표
	float			fConcentration[DefectPixel_F_Max];	// 농도
	BYTE			byFailType[DefectPixel_F_Max];		// 찾은 DefectPixel의 Type --- 값이 0 : Very Hot Pixel, 1: Hot Pixel, 2 : Very Bright Pixel, 3: Bright Pixel, 4 : Very Dark Pixel, 5: Dark Pixel

	_tag_SO_DefectPixel_Result()
	{
		stHeader.iTestItem	= Inspection_DefectPixel;
		wResultCode			= RC_AI_Unknown_Code;

		byIndexCnt			= 0;

		for (int i = 0; i < DefectPixel_F_Max; i++)
		{
			sPointX[i]			= 0;
			sPointY[i]			= 0;
			fConcentration[i]	= 0.0f;
			byFailType[i]		= 0;
		}
	};

}ST_SO_DefectPixel_Result;

//-----------------------------------------------------------------------------
// Corner, Opt : PC -> CAMERA , Result : CAMERA -> PC
//-----------------------------------------------------------------------------
typedef struct _tag_SO_Corner_Opt
{
	ST_SO_Header stHeader;

	bool	bCircle;

	unsigned short sBoardW;
	unsigned short sBoardH;

	_tag_SO_Corner_Opt()
	{
		stHeader.iTestItem = Inspection_Corner;

		bCircle = true;

		sBoardW = 1;
		sBoardH = 1;
	};

}ST_SO_Corner_Opt;

typedef struct _tag_SO_Corner_Result
{
	ST_SO_Header	stHeader;
	WORD			wResultCode;

	unsigned short 	sPointX[Corner_Max];
	unsigned short 	sPointY[Corner_Max];

	_tag_SO_Corner_Result()
	{
		stHeader.iTestItem	= Inspection_DefectPixel;
		wResultCode			= RC_AI_Unknown_Code;

		for (int i = 0; i < Corner_Max; i++)
		{
			sPointX[i] = 0;
			sPointY[i] = 0;
		}
	};

}ST_SO_Corner_Result;

#pragma pack(pop)
#endif
