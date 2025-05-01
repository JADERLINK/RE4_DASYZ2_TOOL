#pragma once
#ifndef __yz2RangeEnc_cxx__
#define __yz2RangeEnc_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。
#include <assert.h>

#include "RangeCode/RangeEncode.cxx"
#include "RangeCode/FrequencyEncode.cxx"
#include "RangeCode/Frequency4Tbl.cxx"       // class Frequency4Tbl<>  頻度表

#include "RangeCode/PPMTbl.cxx"              // class PPMTbl

//-------------------------------------------------------------------
//  RangeCode 符号、符号化  yz2RangeEnc

template<
    class _OutEv
>
class yz2RangeEnc
{
    private: typedef RangeCode<
        15,             // RengeEncode 時の shift_bit 数 12bit なら 4096 ってこと.
        unsigned short, // shift_bit 数 12bit の入る型、unsigned short なら 16bit ってこと.
        unsigned long   // 内部ワーク(有効精度長)のタイプ. 32bit 使うなら unsigned long.
    > Range15Type ;

    private: RangeEncode< _OutEv, Range15Type >                       m_range_encode ; // out: RangeCodeで符号化されたデータの出力先
    private: FrequencyEncode< _OutEv, Frequency4Tbl< Range15Type > >  m_freq_code768 ; //
    private: FrequencyEncode< _OutEv, Frequency4Tbl< Range15Type > >  m_freq_leng ;    //

    // PPM table
    private: std::vector< PPMTbl< Range15Type > >  m_ppm_tbl ;
    private: unsigned short                        m_ppm_key ;

    private: const bool  m_ppm_codec_flag_on ; // true:on / false:off PPM符号化フラグ

    //---------------------------------
    //  コンストラクタ  yz2RangeEnc
public:
    yz2RangeEnc(
        _OutEv  &               out_ev,                 // out: 出力バッファ RangeCodeOutBuffer< OutEv >
        const unsigned short &  in_dic_range_x2 = 1024, // in : 512*2 頻度表 code のテーブルサイズ
        const unsigned short &  in_code_range   = 256,  // in : 256   頻度表 leng のテーブルサイズ
        const bool              in_ppm_codec_flag_on = false // true:on / false:off PPM符号化フラグ
    )
    : m_range_encode ( out_ev )
    , m_freq_code768 ( m_range_encode, in_dic_range_x2 + in_code_range ) // 1024+256
    , m_freq_leng    ( m_range_encode, 256 )             // 8bit
    , m_ppm_tbl      ( in_dic_range_x2 + in_code_range ) // PPM Table
    , m_ppm_key      ( 0 )
    , m_ppm_codec_flag_on( in_ppm_codec_flag_on )
    {}

    //---------------------------------
    //  in_code を RangeCode符号化して出力
public:
    void                                // out: なし
    code_Put(                           // ---: ---------------
        const unsigned short &  in_code // in : code 0～1024+256
    )
    {
        if (  m_ppm_codec_flag_on == false ) // true:on / false:off PPM符号化フラグ
        {
            // PPM OFF
            m_freq_code768.Encode( in_code ) ;
        }
        else
        {
            // PPM ON
            if ( m_ppm_tbl[ m_ppm_key ].m_out_range_tbl[ in_code ].m_width == 0 )
            {
                // PPM table には未登録
                const int  tbl_last = m_ppm_tbl[ m_ppm_key ].m_out_range_tbl.size() - 1 ;

                m_range_encode.shift_Encode(
                    m_ppm_tbl[ m_ppm_key ].m_out_range_tbl[ tbl_last ]
                ) ;
                m_ppm_tbl[ m_ppm_key ].Count( tbl_last ) ;

                m_freq_code768.Encode( in_code ) ;
            }
            else
            {
                // PPM table に登録済み
                m_range_encode.shift_Encode(
                    m_ppm_tbl[ m_ppm_key ].m_out_range_tbl[ in_code ]
                ) ;
                m_ppm_tbl[ m_ppm_key ].Count( in_code ) ;

            }
            m_ppm_key = in_code ;
        }
        // end if
    }

    //---------------------------------
    //  in_leng を RangeCode符号化して出力
public:
    void                               // out: なし
    length_Put(                        // ---: ---------------
        const unsigned long &  in_leng // in : 0～2G(32bit)
    )
    {
        const unsigned long  wk_leng = in_leng + 3 ;

        if ( wk_leng >= (1<<24) )
        {
            m_freq_leng.Encode( 0 ) ;
            m_freq_leng.Encode( (wk_leng >> 24) & 0x000000FF ) ; // 8bit
            m_freq_leng.Encode( (wk_leng >> 16) & 0x000000FF ) ; // 8bit 
            m_freq_leng.Encode( (wk_leng >>  8) & 0x000000FF ) ; // 8bit 
            m_freq_leng.Encode( (wk_leng      ) & 0x000000FF ) ; // 8bit
        }
        else if ( wk_leng >= (1<<16) )
        {
            m_freq_leng.Encode( 1 ) ;
            m_freq_leng.Encode( (wk_leng >> 16) & 0x000000FF ) ; // 8bit
            m_freq_leng.Encode( (wk_leng >>  8) & 0x000000FF ) ; // 8bit 
            m_freq_leng.Encode( (wk_leng      ) & 0x000000FF ) ; // 8bit
        }
        else if ( wk_leng >= (1<<8) )
        {
            m_freq_leng.Encode( 2 ) ;
            m_freq_leng.Encode( (wk_leng >>  8) & 0x000000FF ) ; // 8bit 
            m_freq_leng.Encode( (wk_leng      ) & 0x000000FF ) ; // 8bit
        }
        else
        {
            m_freq_leng.Encode( (wk_leng      ) & 0x000000FF ) ; // 8bit
        }

        return ;
    }

    //---------------------------------
    //  頻度テーブルのリセット（クリア）
public:
    void                   // out: なし
    ReSet()                // ---: ---------------
    {
        m_freq_code768.ReSet() ;
        m_freq_leng.ReSet() ;

        { int        i = 0 ;
          const int  max = m_ppm_tbl.size() ;
          for ( ; i<max ; ++i )
        {
            m_ppm_tbl[i].ReSet() ;
        }}
    }

} ;

//-----------------------------------------------
#ifdef __UnitTest__yz2RangeEnc_cxx__

#include "yz2RangeDec.cxx"

#include <stdio.h> // printf()

#include "OutEv.cxx"
#include "InEv.cxx"

// yz2RangeEnc が動くかどうかテストする。

int main()
{
    const int  max = 10000 ;
    {
        OutEv                 out_ev( "debug.out" ) ;
        yz2RangeEnc< OutEv >  test_rang_enc( out_ev, 512, 256 ) ;

        { srand( 0 ) ;
          int  j = 0 ;
          for ( ; j<5 ; ++j )
        {
            { int  i = 0 ;
              for ( ; i<max ; ++i )
            {
                if ( (rand()%5) != 0 )
                {
                    test_rang_enc.code_Put( rand()%768 ) ;
                }
                else
                {
                    test_rang_enc.length_Put( rand() ) ;
                }
            }}
            test_rang_enc.ReSet() ;
        }}
        out_ev.m_write_bin_file.Close() ;
    }
    printf( "encode OK>" ) ; getchar() ;

    {
        InEv                 in_ev( "debug.out" ) ;
        yz2RangeDec< InEv >  test_rang_dec( in_ev, 768 ) ;

        { srand( 0 ) ;
          int j = 0 ;
          for ( ; j<5 ; ++j )
        {
            { int  i = 0 ;
              for ( ; i<max ; ++i )
            {
                unsigned long  r1, r2 ;
                if ( (rand()%5) != 0 )
                {
                    r1 = test_rang_dec.code_Get() ;
                    r2 = rand()%768 ;
                    if ( r2 != r1 )
                    {
                        printf( "CODE error:%d, %d != %d\n", i, r1, r2 ) ;
                    }
                }
                else
                {
                    r1 = test_rang_dec.length_Get() ;
                    r2 = rand() ;
                    if ( r2 != r1 )
                    {
                        printf( "LENG error:%d, %d != %d\n", i, r1, r2 ) ;
                    }
                }
            }}
            test_rang_dec.ReSet() ;
        }}

    }
    printf( "decode OK>" ) ; getchar() ;

    return 0 ;
}

#endif // __UnitTest__yz2RangeEnc_cxx__

#endif  // __yz2RangeEnc_cxx__
