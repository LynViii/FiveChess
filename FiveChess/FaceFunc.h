#pragma once

void FillGradientEx(CDC *pDC, CRect rect, COLORREF colorStart, COLORREF colorFinish, BOOL bHorz = TRUE);
void HorizontalGradient(CDC	*pDC, LPRECT lpRect, COLORREF clrStart,  COLORREF clrEnd);
void GradientFillRect(CDC	*pDC, CRect &rcFill, COLORREF crLeftTop, COLORREF crRightBottom, BOOL bVertical = FALSE,	BOOL bTran = FALSE);	// bTran 是否透明
