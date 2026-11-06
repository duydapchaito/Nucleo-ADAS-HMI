/**
 * @file    Dcm_Dsp.h
 * @brief   Diagnostic Service Processing - Xử lý nghiệp vụ các dịch vụ UDS và tương tác NvM.
 */

#ifndef DCM_DSP_H
#define DCM_DSP_H

#include "Dcm_Types.h"

void        Dcm_Dsp_Init(void);
Dcm_NrcType Dcm_Dsp_SessionControl(Dcm_MsgContextType *pMsgCtx);
Dcm_NrcType Dcm_Dsp_EcuReset(Dcm_MsgContextType *pMsgCtx);
Dcm_NrcType Dcm_Dsp_TesterPresent(Dcm_MsgContextType *pMsgCtx);
Dcm_NrcType Dcm_Dsp_ReadDataByIdentifier(Dcm_MsgContextType *pMsgCtx);
Dcm_NrcType Dcm_Dsp_WriteDataByIdentifier(Dcm_MsgContextType *pMsgCtx);

#endif /* DCM_DSP_H */
