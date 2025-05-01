#pragma once
#ifndef __FrequencyDecode_cxx__
#define __FrequencyDecode_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

// STL
#ifdef WIN32
#ifdef _DEBUG

  #pragma warning( disable : 4786 )

#endif // _DEBUG
#endif // WIN32

#include "RangeDecode.cxx"     // class RangeDecode<> エントロピー符号（復号）

//-------------------------------------
//  頻度表をもちいた復号化

template<
    class  _InEv,            // 入力データ
    class  _FrequencyTblType // 頻度表  = Frequency4Tbl<>
>
class FrequencyDecode
{
    typedef typename _FrequencyTblType::range_type  range_type ;    // RangeCode<>

    private : RangeDecode< _InEv, range_type > &  m_range_decode ;  // エントロピー符号（復号）
    private : _FrequencyTblType                   m_frequency_tbl ; // 頻度表

    #ifdef HI_SPEED_CODE

        // 高速化版(15bitで128KByte)
        private : std::vector< int >  m_decode_tbl ; // 逆変換テーブル

    #endif // HI_SPEED_CODE

    //---------------------------------
    // コンストラクタ
public :
    FrequencyDecode(
        RangeDecode< _InEv, range_type > &  in_decode_buff,    // in : RangeCodeで符号化されたデータ
        const int &                         in_code_size = 256 // in : 0～255 のコードの出現頻度を 0～4095 の値で現す
    )
    : m_range_decode( in_decode_buff )
    , m_frequency_tbl( in_code_size )
    {
        #ifdef HI_SPEED_CODE
        {
            // 高速化版(15bitで128KByte)
            m_decode_tbl.resize( 1<<range_type::shift_bit ) ; // 逆変換テーブル
        }
        #endif // HI_SPEED_CODE
    }

    //---------------------------------
    // 復号（逆変換）
    private: int                // out: 復号コード
    code_Transfer(              // ---: ----------
        const int &  in_pos     // in : 位置
    )
    {
        #ifdef ORIGINAL_CODE
        {
            // コードを探す（これめっちゃ遅いけど、なんとかせなあかんなー）
            int  code = 0 ;
            { const int  code_max = m_frequency_tbl.m_size ;
              for ( ; code<code_max ; ++code )
            {
                const range_type &  rc = m_frequency_tbl.m_out_range_tbl[ code ] ;
                if ( rc.m_low <= in_pos
                  &&             in_pos < (rc.m_low + rc.m_width)
                   )
                {
                    // 発見
                    break ;
                }
            }}

            return  code ;
        }
        #else // ORIGINAL_CODE

        #ifdef HI_SPEED_CODE
        {
            // コードを探す（高速化版）
            // 逆変換テーブルフラグ true:作成済, false:未作成
            if ( m_frequency_tbl.m_decode_tbl_flg == false )
            {
                // 逆変換テーブル（メモリを食いすぎのような気も・・・^^;）
                m_decode_tbl.resize( 1<<range_type::shift_bit ) ;

                // 作る
                { int        i     = 0 ;
                  int        j     = 0 ;
                  const int  i_max = m_frequency_tbl.m_size ;
                  for ( ; i<i_max ; ++i )
                {
                    { const range_type & rc = m_frequency_tbl.m_out_range_tbl[ i ] ;
                      int  j_max = rc.m_low + rc.m_width ;
                      for ( ; j<j_max ; ++j )
                    {
                        m_decode_tbl[j] = i ;
                    }}
                }}

                // 逆変換テーブルフラグ true:有効, false:無効
                m_frequency_tbl.m_decode_tbl_flg = true ;
            }

            return  m_decode_tbl[ in_pos ] ;
        }
        #else // HI_SPEED_CODE
        {
            // コードを探す（中速版、でもメモリは食わない。）
            int  code_max = m_frequency_tbl.m_size ;
            int  code = 0 ;

            // バイナリーサーチ
            for(;;)
            {
                {
                    int wk = code + ((code_max-code) / 2) ;
                    const range_type &  rc = m_frequency_tbl.m_out_range_tbl[ wk ] ;
                    if ( rc.m_low <= in_pos )
                    {
                        code = wk ;
                    }
                    else
                    {
                        code_max = wk ;
                    }
                }
                const range_type &  rc = m_frequency_tbl.m_out_range_tbl[ code ] ;
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
        #endif // HI_SPEED_CODE

        #endif // ORIGINAL_CODE
    }

    //---------------------------------
    // 復号
public:
    int                   // out: 復号コード
    Decode(               // ---: ----------
    )                     // in : なし
    {
        int  pos  = m_range_decode.shift_Decode() ; // Range内の一点
        int  code = code_Transfer( pos ) ;

        m_range_decode.shift_Update(
            m_frequency_tbl.m_out_range_tbl[ code ]
        ) ;
        m_frequency_tbl.Count( code ) ;

        return  code ;
    }

    //---------------------------------
    // カウントテーブルの初期化
public :
    void      // out: なし
    ReSet(    // ---: ----
    )         // in : なし
    {
        m_frequency_tbl.ReSet() ;
    }
} ;

//-----------------------------------------------
#ifdef __UnitTest__FrequencyDecode_cxx__

#error "単体テストは FrequencyEncode.cxx でまとめてやってます。"

#endif // __UnitTest__FrequencyDecode_cxx__

#endif // __FrequencyDecode_cxx__
