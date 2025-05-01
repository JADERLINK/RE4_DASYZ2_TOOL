#pragma once
#ifndef __RangeDecode_cxx__
#define __RangeDecode_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

#include "RangeCode.cxx" // struct RangeCode<>
#include "RangeCodeInBuffer.cxx"

//-------------------------------------------------------------------
//  RangeDecode
template<
    class _InEv,      // InEv
    class _RangeType  // RangeCode< 12, unsigned short, unsigned long, vector< unsigned char > >
>
class RangeDecode
{
    typedef RangeCodeInBuffer< _InEv >           in_buff_type ;       // 入力バッファ
    typedef typename in_buff_type::value_type    in_buff_value_type ; // unsigned char
    typedef typename _RangeType::work_type       work_type ;          // unsigned long

    #define  WORK_BIT   (sizeof(work_type)*8)          /* ビット数 (unsigned long -> 32bit) */
    #define  WORK_FULL  ((work_type)1<<(WORK_BIT-1))   /* 初期値 0x80000000 */
    #define  BUFF_BIT   (sizeof(in_buff_value_type)*8) /* ビット数 (unsigned char -> 8bit) */

    private : work_type       m_width ;      // 幅
    private : work_type       m_low ;        // 幅の下の値

    private : in_buff_type    m_input ;      // 入力バッファ

    //---------------------------------
    // コンストラクタ
    public :
    RangeDecode(
        _InEv &  in_in_ev  // in : 入力データ
    )
    : m_input( in_in_ev )
    {
        m_width = 1 << (BUFF_BIT-1) ;
        m_low   = m_input.byte_Get() ;
    }

    //---------------------------------
    // 復号化
    public:
    typename _RangeType::value_type      // out: Rangeの中のある一点
    shift_Decode(                        // ---: -----------------
    )
    {
        // 入力
        for ( ; m_width <= (WORK_FULL>>BUFF_BIT) ; m_width <<= BUFF_BIT )
        {
            m_low <<= BUFF_BIT ;
            m_low |= m_input.byte_Get() ;
        }

        // 計算
        m_width >>= (_RangeType::shift_bit-1) ;

        return  m_low / m_width ;
    }

    //---------------------------------
    // 更新
    public: void                     // out: 無し
    shift_Update(                    // ---: --------
        const _RangeType &  in_range // in : RangeCode
    )
    {
        // 計算
        m_low   -= m_width * in_range.m_low ;
        m_width *= in_range.m_width ;
        m_width >>= 1 ;
    }

    #undef  WORK_BIT
    #undef  WORK_FULL
    #undef  BUFF_BIT
} ;

//-----------------------------------------------
#ifdef __UnitTest__RangeDecode_cxx__

int main()
{
    puts( "RangeEncode.cxx でまとめてテストしてます。" ) ;
    getchar() ;

    return 0 ;
}

#endif // __UnitTest__RangeDecode_cxx__

#endif // __RangeDecode_cxx__
