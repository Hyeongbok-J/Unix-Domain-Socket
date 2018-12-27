#ifndef LT_EOL_FUNC_h__
#define LT_EOL_FUNC_h__

#pragma once
#include "Def_Type.h"

// Return Value
typedef WORD	LT_ERROR;

class EOL_Algorithm
{
private:
	int m_nWidth;
	int m_nHeight;

public:
	LT_ERROR Initialize(int nWidth, int nHeight);

	// pIR : IR camera capture data Memory pointer
	// pDepth : Depth camera capture data Memory pointer
	// pMSG : 검사 구분, 항목, ROI등을 포함한 CAN 을 통한 전송되는 MSG 전체
	// resultLen : resultMSG length
	// pResultMSG : 검사 결과 data
	LT_ERROR Inspection(BYTE *pIR, BYTE *pDepth, BYTE *pMSG, int &resultLen, BYTE *pResultMSG);

	LT_ERROR Finalize();
};
#endif