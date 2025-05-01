#pragma once
#ifndef __RangeCode_cxx__
#define __RangeCode_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

//-------------------------------------------------------------------
//  RangeCode 幅を現す
template<
    int       _in_shift_bit = 12,             // RengeEncode 時の shift_bit 数 12bit なら 4096 ってこと.
    typename  _TableType    = unsigned short, // shift_bit 数 12bit の入る型、unsigned short なら 16bit ってこと.
    typename  _WorkType     = unsigned long   // 内部ワーク(有効精度長)のタイプ. 32bit 使うなら unsigned long.
>
struct RangeCode
{
    enum { shift_bit = _in_shift_bit } ;      // RengeEncode 時の shift_bit 数 12bit なら 4096 ってこと.
    typedef _TableType      value_type ;      // shift_bit 数 12bit の入る型、unsigned short なら 16bit ってこと.
    typedef _WorkType       work_type ;       // 内部ワーク(有効精度長)のタイプ. 32bit 使うなら unsigned long.

    value_type  m_width ; // 幅の値
    value_type  m_low ;   // 幅の下の値
} ;

//-----------------------------------------------
#ifdef __UnitTest__RangeCode_cxx__

// テストするほどのコードではないですね。^^;
int main()
{
    RangeCode<>  rc ;

    getchar() ;

    return 0 ;
}

#endif // __UnitTest__RangeCode_cxx__

#endif // __RangeCode_cxx__
