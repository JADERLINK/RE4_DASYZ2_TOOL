#pragma once
#ifndef __Frequency4Tbl_cxx__
#define __Frequency4Tbl_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

// STL
#ifdef WIN32
#ifdef _DEBUG

  #pragma warning( disable : 4786 )

#endif // _DEBUG
#endif // WIN32

#include <vector>    // STL using  std::vector ;

#include "RangeCode.cxx" // class RangeCode<> 

//-------------------------------------------------------------------
//  Frequency4Tbl （RangeCode 専用の頻度テーブル）

template<
    class RangeType = RangeCode<>
>
class Frequency4Tbl
{
public :
    typedef RangeType  range_type ;

    private: typedef typename RangeType::value_type  range_value_type ;

    private: std::vector< range_value_type >  m_code_cnt ; // コードカウンタ
    private: unsigned long                    m_sum ;

    private: int                        m_bit ;         // 出力 rangeテーブルを作り直すビット数
    private: unsigned long              m_check_point ; // 出力 rangeテーブルを作り直すポイント

    public : std::vector< range_type >  m_out_range_tbl ; // 出力先 Rangeテーブル
    public : const int                  m_size ;          // in : 0～255 Rangeテーブルのサイズ

    #ifndef ORIGINAL_CODE

        // 高速化版
        // 逆変換テーブルフラグ  true:有効 / false:無効
        public : bool  m_decode_tbl_flg ;

    #endif ORIGINAL_CODE

    //---------------------------------
    // カウントテーブルの初期化
public :
    void      // out: なし
    ReSet()   // ---: ----
    {
        //-----------------------------
        // m_code_cnt を作り直す。
        {
            { int i = 0 ;
              for ( ; i<m_size ; ++i )
            {
                m_code_cnt[i] = 1 ;
            }}
            // end for i

            m_sum = m_size ;
        }

        //-----------------------------
        // テーブルを作り直すポイントを決める。m_bit, m_check_point を作る
        {
            m_bit = 0 ;
            { for ( ; m_bit<range_type::shift_bit ; ++m_bit )
            {
                if ( m_sum < (1<<m_bit) ) break ;
            }}

            m_check_point = 1 << m_bit ;
        }

        #ifndef ORIGINAL_CODE
        {
            // 高速化版
            // 逆変換テーブルフラグ true:有効, false:無効
            m_decode_tbl_flg = false ;
        }
        #endif // ORIGINAL_CODE
    }

    //---------------------------------
    // コンストラクタ
public :
    Frequency4Tbl(
        const int &  in_code_size = 256 // in : 0～255 のコードの出現頻度を 0～4095 の値で現す
    )
    : m_size( in_code_size ) // in : 0～255
    {
        m_code_cnt.resize( in_code_size ) ; // 0～255 のコードの出現頻度
        m_out_range_tbl.resize( in_code_size ) ;

        //-----------------------------
        // 仮テーブル作成
        {
            // 均等？に割り振る
            { int i = 0 ;
              int j = 0 ;
              for ( ; j<(1<<range_type::shift_bit) ; ++j )
            {
                m_code_cnt[i] ++ ;
                if ( ++i >= m_size ) i = 0 ;
            }}
            // end for i, j

            // range テーブル作成
            { range_value_type  sum = 0 ;
              int i = 0 ;
              int i_max = m_code_cnt.size() ;
              for ( ; i<i_max ; ++ i )
            {
                m_out_range_tbl[i].m_width = m_code_cnt[i] ;
                m_out_range_tbl[i].m_low   = sum ;
                sum += m_code_cnt[i] ;
            }}
            // end for i
        }

        //-----------------------------
        // テーブル作成
        ReSet() ;
    }

    //---------------------------------
    // テーブル出力
    private : void      // out: なし
    range_tbl_Out()     // ---: ------
    {
        #ifndef ORIGINAL_CODE
        {
            // 高速化版
            // 逆変換テーブルフラグ true:有効, false:無効
            m_decode_tbl_flg = false ;
        }
        #endif // ORIGINAL_CODE

        // テーブル飽和前
        { range_value_type  sum = 0 ;
          int i = 0 ;
          int i_max = m_code_cnt.size() ;
          for ( ; i<i_max ; ++ i )
        {
            range_value_type  x = 1 << (range_type::shift_bit - m_bit) ;

            // range テーブル作成
            m_out_range_tbl[i].m_width = m_code_cnt[i] * x ;
            m_out_range_tbl[i].m_low   = sum ;
            sum += m_code_cnt[i] * x ;
        }}
        // end for i
    }

    //---------------------------------
    // テーブル出力
    private : void      // out: なし
    range_tbl_Half()    // ---: ----
    {
        #ifndef ORIGINAL_CODE
        {
            // 高速化版
            // 逆変換テーブルフラグ true:有効, false:無効
            m_decode_tbl_flg = false ;
        }
        #endif // ORIGINAL_CODE

        // テーブル飽和後（テーブルを半分に）
        m_sum = 0 ;

        // テーブル出力
        { range_value_type  sum = 0 ;
          int       i     = 0 ;
          const int i_max = m_code_cnt.size() ;
          for ( ; i<i_max ; ++ i )
        {
            // range テーブル作成
            m_out_range_tbl[i].m_width = m_code_cnt[i] ;
            m_out_range_tbl[i].m_low   = sum ;
            sum += m_code_cnt[i] ;

            // カウントテーブルを 1/2 に
            range_value_type  c = m_code_cnt[i] >> 1 ;  // 1/2に
            if ( c == 0 )
            {
                // 1以下にはしない
                m_code_cnt[i] = 1 ;
            }
            else 
            {
                m_code_cnt[i] = c ;
            }
            m_sum += m_code_cnt[i] ;
        }}
        // end for i
    }

    //---------------------------------
    // 出現頻度カウント
public:
    void                      // out: なし
    Count(                    // ---: -----------
        const int &  in_code  // in : 出現コード
    )
    {
        // カウント
        m_code_cnt[ in_code ] += 1 ;
        m_sum += 1 ;

        if ( range_type::shift_bit > m_bit )
        {
            // カウントテーブル飽和前
            if ( m_sum == m_check_point )
            {
                // rangeテーブル出力（再計算）
                range_tbl_Out() ;

                // 次の作成ポイント
                m_bit += 1 ;
                m_check_point = 1 << m_bit ;
            }
        }
        else
        {
            // カウントテーブル飽和後
            if ( m_sum >= (1<<range_type::shift_bit) )
            {
                // rangeテーブル出力（再計算）
                range_tbl_Half() ;
            }
        }

        return ;
    }

} ;



//-----------------------------------------------
#ifdef __UnitTest__Frequency4Tbl_cxx__

#include <stdio.h> // printf(), getchar()

int main()
{
    Frequency4Tbl< RangeCode<5> >  freq_tbl( 10 ) ; // 5bit=32, 10<16 

    { int i = 0 ;
      int i_size = freq_tbl.m_out_range_tbl.size() ;
      for ( ; i<i_size ; ++ i )
    {
        printf( "%d:%d %d, ", i,
            freq_tbl.m_out_range_tbl[i].m_width,
            freq_tbl.m_out_range_tbl[i].m_low
        ) ;
    }}
    puts("") ;

    for ( int j=0 ; j<12 ; ++j )
    {
        freq_tbl.Count( 5 ) ;
        freq_tbl.Count( 6 ) ;
        { int i = 0 ;
          int i_size = freq_tbl.m_out_range_tbl.size() ;
          for ( ; i<i_size ; ++ i )
        {
            printf( "%d:%d %d, ", i,
                freq_tbl.m_out_range_tbl[i].m_width,
                freq_tbl.m_out_range_tbl[i].m_low
            ) ;
        }} 
        puts("") ;
    }

    printf( "test end >" ) ; getchar() ;

    return 0 ;
}

#endif // __UnitTest__Frequency4Tbl_cxx__

#endif // __Frequency4Tbl_cxx__
