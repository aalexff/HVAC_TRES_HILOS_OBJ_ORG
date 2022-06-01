 //FileName:        types.h
 //Dependencies:    None
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Compatibilidad con MQX, re-definición de tipos de datos. Comentarios en inglés.
 //About author:   	Adaptación de Freescale.
 //Updated:         12/2018

#ifndef TYPES_H_
#define TYPES_H_

#define _PTR_           *
#define _CODE_PTR_      *

typedef char _PTR_                    char_ptr;    /* signed character       */
typedef unsigned char  uchar, _PTR_   uchar_ptr;   /* unsigned character     */
typedef volatile char _PTR_                    vchar_ptr;    /* signed character       */
typedef volatile unsigned char  vuchar, _PTR_   vuchar_ptr;   /* unsigned character     */

typedef signed   char   int_8, _PTR_   int_8_ptr;   /* 8-bit signed integer   */
typedef unsigned char  uint_8, _PTR_  uint_8_ptr;  /* 8-bit signed integer   */
typedef volatile signed   char   vint_8, _PTR_   vint_8_ptr;   /* 8-bit volatile signed integer   */
typedef volatile unsigned char  vuint_8, _PTR_  vuint_8_ptr;  /* 8-bit volatile signed integer   */

typedef          short int_16, _PTR_  int_16_ptr;  /* 16-bit signed integer  */
typedef unsigned short uint_16, _PTR_ uint_16_ptr; /* 16-bit unsigned integer*/
typedef volatile          short vint_16, _PTR_  vint_16_ptr;  /* 16-bit volatile signed integer  */
typedef volatile unsigned short vuint_16, _PTR_ vuint_16_ptr; /* 16-bit volatile unsigned integer*/

typedef          long  int_32, _PTR_  int_32_ptr;  /* 32-bit signed integer  */
typedef unsigned long  uint_32, _PTR_ uint_32_ptr; /* 32-bit unsigned integer*/
typedef volatile          long  vint_32, _PTR_  vint_32_ptr;  /* 32-bit signed integer  */
typedef volatile unsigned long  vuint_32, _PTR_ vuint_32_ptr; /* 32-bit unsigned integer*/

typedef    long  long  int_64, _PTR_  int_64_ptr;       /* 64-bit signed   */
typedef unsigned long long  uint_64, _PTR_ uint_64_ptr; /* 64-bit unsigned */
typedef volatile   long  long  vint_64, _PTR_  vint_64_ptr;       /* 64-bit signed   */
typedef volatile unsigned long long  vuint_64, _PTR_ vuint_64_ptr; /* 64-bit unsigned */

typedef unsigned long  boolean;  /* Machine representation of a boolean */

typedef uint_32     _file_size;
typedef int_32      _file_offset;
typedef void _PTR_  pointer;

typedef uint_32  _mqx_uint, _PTR_ _mqx_uint_ptr;
typedef int_32   _mqx_int,  _PTR_ _mqx_int_ptr;

#endif /* TYPES_H_ */
