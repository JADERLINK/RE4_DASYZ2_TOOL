#pragma once
#ifndef __yz2RangeDec_cxx__
#define __yz2RangeDec_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

#include "RangeCode/Frequency4Tbl.cxx"
#include "RangeCode/FrequencyDecode.cxx"
#include "RangeCode/RangeCodeInBuffer.cxx" // class RangeCodeInBuffer  入力バッファ

#include "RangeCode/PPMDecode.cxx"

//-------------------------------------------------------------------
//  RangeCode符号、復号化    yz2RangeDec

template<
    class _InEv
>
class yz2RangeDec
{
    private: typedef RangeCode<
        15,              // RengeEncode 時の shift_bit 数 12bit なら 4096 ってこと.
        unsigned short,  // shift_bit 数 12bit の入る型、unsigned short なら 16bit ってこと.
        unsigned long    // 内部ワーク(有効精度長)のタイプ. 32bit 使うなら unsigned long.
    > Range15Type ;

    private: RangeDecode< _InEv, Range15Type >                       m_range_decode ; // RangeCodeで符号化されたデータの入力
    private: FrequencyDecode< _InEv, Frequency4Tbl< Range15Type > >  m_freq_code768 ; //
    private: FrequencyDecode< _InEv, Frequency4Tbl< Range15Type > >  m_freq_leng ;    //

    // PPM table
    private: const bool                       m_ppm_mode_flag_on ; // PPMモード true:on / false:off
    // ↑なぜかconst bool & にすると内容が破壊される。どこかでメモリを破壊しているのか…コンパイラのバグか…謎。

    private: PPMDecode< _InEv, Range15Type >  m_ppm_tbl ;

    //---------------------------------
    //  コンストラクタ  yz2RangeDec
public:
    yz2RangeDec(                                            // ---: -------------------
        _InEv &                 in_ev,                      // in : 入力データ
        const unsigned short &  in_n,                       // in : 頻度表テーブルサイズ (512+512+256)
        const bool              in_ppm_mode_flag_on = false // in : PPMモード true:on / false:off
    )
    : m_range_decode    ( in_ev                )   //
    , m_freq_code768    ( m_range_decode, in_n )   // 
    , m_freq_leng       ( m_range_decode, 256  )   // 8bit
    , m_ppm_mode_flag_on( in_ppm_mode_flag_on  )   // PPMモード true:on / false:off
    , m_ppm_tbl         ( m_range_decode, in_n )   // PPM Table
    {}

    //---------------------------------
    //  ビットデータから、コードを取得    BuffGet()
public:
    unsigned short    // out: コード
    code_Get(         // ---: -------------------
    )                 // in : なし
    {
        if ( m_ppm_mode_flag_on == false ) // PPMモード true:on / false:off
        {
            // PPM OFF
            return  m_freq_code768.Decode() ;
        }
        else
        {
            // PPM ON
            return  m_ppm_tbl.Decode() ;
        }
    }

    //---------------------------------
    //  ビットデータから長さを取得   BuffGetLength()
public:
    unsigned long         // out: 長さ(32bit)
    length_Get()          // ---: -------------------
    {
        unsigned long  rtn = m_freq_leng.Decode() ;

        if ( rtn == 0 )
        {
            rtn =  m_freq_leng.Decode() << 24 ; // 8bit
            rtn |= m_freq_leng.Decode() << 16 ; // 8bit 
            rtn |= m_freq_leng.Decode() <<  8 ; // 8bit 
            rtn |= m_freq_leng.Decode()       ; // 8bit
        }
        else if ( rtn == 1 )
        {
            rtn =  m_freq_leng.Decode() << 16 ; // 8bit
            rtn |= m_freq_leng.Decode() <<  8 ; // 8bit 
            rtn |= m_freq_leng.Decode()       ; // 8bit
        }
        else if ( rtn == 2 )
        {
            rtn =  m_freq_leng.Decode() <<  8 ; // 8bit 
            rtn |= m_freq_leng.Decode()       ; // 8bit
        }

        return  rtn - 3 ;
    }

    //---------------------------------
    // カウントテーブルの初期化
public :
    void            // out: なし
    ReSet()         // ---: ----
    {
        m_freq_code768.ReSet() ;
        m_freq_leng.ReSet() ;
        m_ppm_tbl.ReSet() ;
    }
} ;

#endif // __yz2RangeDec_cxx__
