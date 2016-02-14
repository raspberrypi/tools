/* Generated automatically from machmode.def and config/arm/arm-modes.def
   by genmodes.  */

#ifndef GCC_INSN_MODES_H
#define GCC_INSN_MODES_H

enum machine_mode
{
  VOIDmode,                /* machmode.def:172 */
  BLKmode,                 /* machmode.def:176 */
  CCmode,                  /* machmode.def:207 */
  CC_NOOVmode,             /* config/arm/arm-modes.def:43 */
  CC_Zmode,                /* config/arm/arm-modes.def:44 */
  CC_CZmode,               /* config/arm/arm-modes.def:45 */
  CC_NCVmode,              /* config/arm/arm-modes.def:46 */
  CC_SWPmode,              /* config/arm/arm-modes.def:47 */
  CCFPmode,                /* config/arm/arm-modes.def:48 */
  CCFPEmode,               /* config/arm/arm-modes.def:49 */
  CC_DNEmode,              /* config/arm/arm-modes.def:50 */
  CC_DEQmode,              /* config/arm/arm-modes.def:51 */
  CC_DLEmode,              /* config/arm/arm-modes.def:52 */
  CC_DLTmode,              /* config/arm/arm-modes.def:53 */
  CC_DGEmode,              /* config/arm/arm-modes.def:54 */
  CC_DGTmode,              /* config/arm/arm-modes.def:55 */
  CC_DLEUmode,             /* config/arm/arm-modes.def:56 */
  CC_DLTUmode,             /* config/arm/arm-modes.def:57 */
  CC_DGEUmode,             /* config/arm/arm-modes.def:58 */
  CC_DGTUmode,             /* config/arm/arm-modes.def:59 */
  CC_Cmode,                /* config/arm/arm-modes.def:60 */
  CC_Nmode,                /* config/arm/arm-modes.def:61 */
  BImode,                  /* machmode.def:179 */
  QImode,                  /* machmode.def:187 */
  HImode,                  /* machmode.def:188 */
  SImode,                  /* machmode.def:189 */
  DImode,                  /* machmode.def:190 */
  TImode,                  /* machmode.def:191 */
  EImode,                  /* config/arm/arm-modes.def:78 */
  OImode,                  /* config/arm/arm-modes.def:79 */
  CImode,                  /* config/arm/arm-modes.def:80 */
  XImode,                  /* config/arm/arm-modes.def:81 */
  QQmode,                  /* machmode.def:210 */
  HQmode,                  /* machmode.def:211 */
  SQmode,                  /* machmode.def:212 */
  DQmode,                  /* machmode.def:213 */
  TQmode,                  /* machmode.def:214 */
  UQQmode,                 /* machmode.def:216 */
  UHQmode,                 /* machmode.def:217 */
  USQmode,                 /* machmode.def:218 */
  UDQmode,                 /* machmode.def:219 */
  UTQmode,                 /* machmode.def:220 */
  HAmode,                  /* machmode.def:222 */
  SAmode,                  /* machmode.def:223 */
  DAmode,                  /* machmode.def:224 */
  TAmode,                  /* machmode.def:225 */
  UHAmode,                 /* machmode.def:227 */
  USAmode,                 /* machmode.def:228 */
  UDAmode,                 /* machmode.def:229 */
  UTAmode,                 /* machmode.def:230 */
  HFmode,                  /* config/arm/arm-modes.def:26 */
  SFmode,                  /* machmode.def:202 */
  DFmode,                  /* machmode.def:203 */
  SDmode,                  /* machmode.def:242 */
  DDmode,                  /* machmode.def:243 */
  TDmode,                  /* machmode.def:244 */
  CQImode,                 /* machmode.def:238 */
  CHImode,                 /* machmode.def:238 */
  CSImode,                 /* machmode.def:238 */
  CDImode,                 /* machmode.def:238 */
  CTImode,                 /* machmode.def:238 */
  CEImode,                 /* machmode.def:238 */
  COImode,                 /* machmode.def:238 */
  CCImode,                 /* machmode.def:238 */
  CXImode,                 /* machmode.def:238 */
  HCmode,                  /* machmode.def:239 */
  SCmode,                  /* machmode.def:239 */
  DCmode,                  /* machmode.def:239 */
  V4QImode,                /* config/arm/arm-modes.def:64 */
  V2HImode,                /* config/arm/arm-modes.def:64 */
  V8QImode,                /* config/arm/arm-modes.def:65 */
  V4HImode,                /* config/arm/arm-modes.def:65 */
  V2SImode,                /* config/arm/arm-modes.def:65 */
  V16QImode,               /* config/arm/arm-modes.def:66 */
  V8HImode,                /* config/arm/arm-modes.def:66 */
  V4SImode,                /* config/arm/arm-modes.def:66 */
  V2DImode,                /* config/arm/arm-modes.def:66 */
  V4QQmode,                /* config/arm/arm-modes.def:71 */
  V2HQmode,                /* config/arm/arm-modes.def:71 */
  V4UQQmode,               /* config/arm/arm-modes.def:72 */
  V2UHQmode,               /* config/arm/arm-modes.def:72 */
  V2HAmode,                /* config/arm/arm-modes.def:73 */
  V2UHAmode,               /* config/arm/arm-modes.def:74 */
  V4HFmode,                /* config/arm/arm-modes.def:67 */
  V2SFmode,                /* config/arm/arm-modes.def:67 */
  V8HFmode,                /* config/arm/arm-modes.def:68 */
  V4SFmode,                /* config/arm/arm-modes.def:68 */
  V2DFmode,                /* config/arm/arm-modes.def:68 */
  MAX_MACHINE_MODE,

  MIN_MODE_RANDOM = VOIDmode,
  MAX_MODE_RANDOM = BLKmode,

  MIN_MODE_CC = CCmode,
  MAX_MODE_CC = CC_Nmode,

  MIN_MODE_INT = QImode,
  MAX_MODE_INT = XImode,

  MIN_MODE_PARTIAL_INT = VOIDmode,
  MAX_MODE_PARTIAL_INT = VOIDmode,

  MIN_MODE_FRACT = QQmode,
  MAX_MODE_FRACT = TQmode,

  MIN_MODE_UFRACT = UQQmode,
  MAX_MODE_UFRACT = UTQmode,

  MIN_MODE_ACCUM = HAmode,
  MAX_MODE_ACCUM = TAmode,

  MIN_MODE_UACCUM = UHAmode,
  MAX_MODE_UACCUM = UTAmode,

  MIN_MODE_FLOAT = HFmode,
  MAX_MODE_FLOAT = DFmode,

  MIN_MODE_DECIMAL_FLOAT = SDmode,
  MAX_MODE_DECIMAL_FLOAT = TDmode,

  MIN_MODE_COMPLEX_INT = CQImode,
  MAX_MODE_COMPLEX_INT = CXImode,

  MIN_MODE_COMPLEX_FLOAT = HCmode,
  MAX_MODE_COMPLEX_FLOAT = DCmode,

  MIN_MODE_VECTOR_INT = V4QImode,
  MAX_MODE_VECTOR_INT = V2DImode,

  MIN_MODE_VECTOR_FRACT = V4QQmode,
  MAX_MODE_VECTOR_FRACT = V2HQmode,

  MIN_MODE_VECTOR_UFRACT = V4UQQmode,
  MAX_MODE_VECTOR_UFRACT = V2UHQmode,

  MIN_MODE_VECTOR_ACCUM = V2HAmode,
  MAX_MODE_VECTOR_ACCUM = V2HAmode,

  MIN_MODE_VECTOR_UACCUM = V2UHAmode,
  MAX_MODE_VECTOR_UACCUM = V2UHAmode,

  MIN_MODE_VECTOR_FLOAT = V4HFmode,
  MAX_MODE_VECTOR_FLOAT = V2DFmode,

  NUM_MACHINE_MODES = MAX_MACHINE_MODE
};

#define CONST_MODE_SIZE const
#define CONST_MODE_BASE_ALIGN const
#define CONST_MODE_IBIT const
#define CONST_MODE_FBIT const

#define BITS_PER_UNIT (8)
#define MAX_BITSIZE_MODE_ANY_INT (64*BITS_PER_UNIT)
#define MAX_BITSIZE_MODE_ANY_MODE (128*BITS_PER_UNIT)

#endif /* insn-modes.h */
