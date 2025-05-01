#pragma once
#ifndef __RangeEncode_cxx__
#define __RangeEncode_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

#include "RangeCode.cxx" // struct RangeCode<>
#include "RangeCodeOutBuffer.cxx"

//-------------------------------------------------------------------
// RangeEncode
template<
    class _OutEv,    // = OutEv // 出力先
    class _RangeType // RangeCode< 12, unsigned short, unsigned long >
>
class RangeEncode
{
    typedef RangeCodeOutBuffer< _OutEv >           out_buff_type ;  // 出力バッファ
    typedef typename out_buff_type::value_type     out_value_type ; // unsigned char
    typedef typename _RangeType::work_type         work_type ;      // unsigned long

    #define  WORK_BIT   (sizeof(work_type)*8)        /* ビット数 (unsigned long -> 32bit) */
    #define  WORK_FULL  ((work_type)1<<(WORK_BIT-1)) /* 初期値 0x80000000 */
    #define  BUFF_BIT   (sizeof(out_value_type)*8)   /* ビット数 (unsigned char -> 8bit) */

    private : work_type  m_width ; // 幅
    private : work_type  m_low ;   // 幅の下の値

    private : RangeCodeOutBuffer< _OutEv >  m_output ; // 出力先バッファ

    //---------------------------------
    // コンストラクタ
public :
    RangeEncode(
        _OutEv &  in_out_ev   // out: 出力先
    )
    : m_output( in_out_ev )
    {
        m_width = WORK_FULL ;
        m_low   = 0 ;
    }


    //---------------------------------
    // バッファ出力
    private: void                 // out: なし
    buffer_Put(                   // ---: ------
    )                             // in : なし
    {
        // m_width が BUFF_BIT (1/256) 以下かどうか？
        for ( ; m_width <= (WORK_FULL>>BUFF_BIT) ; m_width <<= BUFF_BIT )
        {
            // バッファ出力
            m_output.byte_Put( (m_low >> (WORK_BIT-BUFF_BIT))  ) ;
            m_low <<= BUFF_BIT ;
        }
    }

    //---------------------------------
    // 符号化
public:
    void                              // out: なし
    shift_Encode(                     // ---: -----
        const _RangeType &  in_range  // in : RangeCode
    )
    {
        // バッファ出力
        buffer_Put() ;

        // 計算
        work_type  tmp = m_low ;
        {
            work_type  r = m_width >> (_RangeType::shift_bit-1) ;
            m_low   += r * in_range.m_low ;
            m_width = (r * in_range.m_width) >> 1 ;
        }

        // 桁上がりチェック
        if ( tmp > m_low )
        {
            // 桁上がり
            m_output.carry_Up() ;
        }
    }

    //---------------------------------
    // デストラクタ
public :
    ~RangeEncode(
    )
    {
        // バッファ出力
        buffer_Put() ;

        // 値を上に修正
        {
            work_type  tmp = m_low ;
            {
                m_low   += m_width ;
                m_width >>= 1 ;
            }
            // 桁上がりチェック
            if ( tmp > m_low )
            {
                // 桁上がり
                m_output.carry_Up() ;
            }
        }

        // バッファ出力
        buffer_Put() ;

        if ( m_width < WORK_FULL )
        {
            m_output.byte_Put( m_low >> (WORK_BIT-BUFF_BIT) ) ;
        }
    }

    #undef  WORK_BIT
    #undef  WORK_FULL
    #undef  BUFF_BIT
} ;

//-----------------------------------------------
#ifdef __UnitTest__RangeEncode_cxx__

#include "../../../cxl/SystemTime.cxx" // class SystemTime

#include "RangeDecode.cxx"

int main()
{
    const int  test_size = 10*1024*1024 ; // 10MByte
    const int  loop_size = 10 ;

    for( int j=0 ; j<loop_size ; ++j )
    {
        srand( j ) ;
        //CXL::SystemTime    sys_time ; // start
        {
            OutEv  debug_out( "debug.out" ) ;
            {
                RangeEncode< OutEv, RangeCode<8> >  range_encode( debug_out ) ;

                for ( int i=0 ; i<test_size ; ++i )
                {
                    RangeCode<8>  rc = { 1, rand()%256 } ;
                    range_encode.shift_Encode( rc ) ;
                }
            } // <- このスコープで RangeCodeOutBuffer をフラッシュする
            debug_out.m_write_bin_file.Close() ;

            int  encode_size = debug_out.m_write_bin_file.write_size_Get() ;
            printf( "size: %d byte (%d%%) \n", encode_size, encode_size*100/test_size ) ;
        }
        // printf( "time: %s sec.\n", sys_time.str_Get() ) ;

        srand( j ) ;
        CXL::SystemTime    sys_time ; // start
        {
            InEv                               debug_out( "debug.out" ) ;
            RangeDecode< InEv, RangeCode<8> >  range_decode( debug_out ) ;

            for ( int i=0 ; i<test_size ; ++i )
            {
                unsigned long  ul = range_decode.shift_Decode() ;
                RangeCode<8>   rc = { 1, ul } ;
                range_decode.shift_Update( rc ) ;
                unsigned long  r = rand()%256 ;
                if ( ul != r )
                {
                    printf( "error:[%d] %d != %d\n",i, ul, r ) ;
                    getchar() ;
                }
            }
        }
        printf( "time: %s sec.\n", sys_time.str_Get() ) ;
    }
    // end for j

    printf( "test end >" ) ; getchar() ;

    return 0 ;
}

#endif // __UnitTest__RangeEncode_cxx__

#endif // __RangeEncode_cxx__
