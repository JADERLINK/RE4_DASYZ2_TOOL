#pragma once
#ifndef __PPMDecode_cxx__
#define __PPMDecode_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

// STL
#ifdef WIN32
#ifdef _DEBUG

  #pragma warning( disable : 4786 )

#endif // _DEBUG
#endif // WIN32

#include "RangeDecode.cxx"     // class RangeDecode<> エントロピー符号（復号）
#include "FrequencyDecode.cxx"
#include "Frequency4Tbl.cxx"
#include "PPMTbl.cxx"

//-------------------------------------
//  頻度表をもちいた復号化

template<
    class  _InEv,       // 入力データ
    class  _RangeCode   //
>
class PPMDecode
{
    private : RangeDecode< _InEv, _RangeCode > &                     m_range_decode ;  // エントロピー符号（復号）
    private : FrequencyDecode< _InEv, Frequency4Tbl< _RangeCode > >  m_freq_code768 ;  //

    private : std::vector< PPMTbl< _RangeCode > >  m_ppm_tbl ; // PPM頻度表
    private: unsigned short                        m_ppm_key ;

    //---------------------------------
    // コンストラクタ
public :
    PPMDecode(
        RangeDecode< _InEv, _RangeCode > &  in_decode_buff,  // in : RangeCodeで符号化されたデータ
        const unsigned short &              in_n             // in : 頻度表テーブルサイズ
    )
    : m_range_decode ( in_decode_buff )
    , m_freq_code768 ( m_range_decode, in_n ) // 768
    , m_ppm_tbl      ( in_n )
    , m_ppm_key      ( 0 )
    {
    }

    //---------------------------------
    // 復号（逆変換）
    private: int                            // out: 復号コード
    code_Transfer(                          // ---: ----------
        const int            &  in_pos,     // in : 位置
        PPMTbl< _RangeCode > &  in_ppm_tbl
    )
    {
        // コードを探す（中速版、でもメモリは食わない。）
        int  code_max = in_ppm_tbl.m_size ;
        int  code = 0 ;

        // バイナリーサーチ
        for ( ; ; )
        {
            {
                int wk = code + ((code_max-code) / 2) ;
                const _RangeCode &  rc = in_ppm_tbl.m_out_range_tbl[ wk ] ;
                if ( rc.m_low <= in_pos )
                {
                    code = wk ;
                }
                else
                {
                    code_max = wk ;
                }
            }
            const _RangeCode &  rc = in_ppm_tbl.m_out_range_tbl[ code ] ;
            if ( rc.m_low <= in_pos
              &&             in_pos < (rc.m_low + rc.m_width)
               )
            {
                // 発見
                break ;
            }
            code ++ ;

        }
        // end for

        return  code ;
    }

    //---------------------------------
    // 復号
public:
    int                    // out: 復号コード
    Decode(                // ---: ----------
    )
    {
        PPMTbl< _RangeCode > &  ppm = m_ppm_tbl[ m_ppm_key ] ;
        int  pos  = m_range_decode.shift_Decode() ; // Range内の一点
        int  code = code_Transfer( pos, ppm ) ;

        m_range_decode.shift_Update(
            ppm.m_out_range_tbl[ code ]
        ) ;

        const unsigned short  last = ppm.m_out_range_tbl.size() - 1 ;
        if ( code == last )
        {
            // PPM table には未登録
            code = m_freq_code768.Decode() ;
            ppm.Count( last ) ;
        }
        else
        {
            // PPM table に登録済み
            ppm.Count( code ) ;
        }
        m_ppm_key = code ;
        return  code ;
    }

    //---------------------------------
    // カウントテーブルの初期化
public :
    void      // out: なし
    ReSet(    // ---: ----
    )         // in : なし
    {
        m_freq_code768.ReSet() ;

        const int  max = m_ppm_tbl.size() ;
        int i = 0 ;
        for ( ; i<max ; ++i ) m_ppm_tbl[i].ReSet() ;
    }
} ;

//-----------------------------------------------
#ifdef __UnitTest__PPMDecode_cxx__

#error "単体テストは FrequencyEncode.cxx でまとめてやってます。"

#endif // __UnitTest__PPMDecode_cxx__

#endif // __PPMDecode_cxx__
