#pragma once
#ifndef __PPMTbl_cxx__
#define __PPMTbl_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

// STL
#ifdef WIN32
#ifdef _DEBUG

  #pragma warning( disable : 4786 )

#endif // _DEBUG
#endif // WIN32

#include <vector>        // STL using  std::vector ;

#include "RangeCode.cxx" // class RangeCode<> 

//-------------------------------------------------------------------
//  PPMTbl （PPM 用の頻度テーブル）

template<
    class RangeType = RangeCode<>
>
class PPMTbl
{
public :
    typedef RangeType                                range_type ;

    private: typedef typename RangeType::value_type  range_value_type ; // カウントするビット幅

    private: std::vector< range_value_type >  m_code_cnt ; // コードカウンタ
    private: unsigned long                    m_sum ;      // カウントの総数

    private: int               m_bit ;         // 出力 rangeテーブルを作り直すビット数
    private: unsigned long     m_check_point ; // 出力 rangeテーブルを作り直すポイント

public :
    std::vector< range_type >  m_out_range_tbl ; // 出力先 Range(m_width/m_low)テーブル
    const int                  m_size ;          // in : 0～256 Rangeテーブルのサイズ

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
                m_code_cnt[i] = 0 ;
            }}
            // end for i

            m_code_cnt[m_size-1] = 15 ; // hit しなかった（「16」は次は16個数えてからテーブルを更新するという意味）
            m_sum = m_code_cnt[m_size-1] ;
            m_code_cnt[0] = 1 ;
            m_sum += m_code_cnt[0] ;
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
    PPMTbl(
        const int &  in_code_size = 1024+256 // in : 0～256 のコードの出現頻度を 0～4095 の値で現す
    )
    : m_size( in_code_size + 1 ) // in : 0～256 + 1
    {
        m_code_cnt.resize( m_size ) ; // 0～256 + 1 のコードの出現頻度
        m_out_range_tbl.resize( m_size ) ;

        //-----------------------------
        // 仮テーブル作成
        {
            // ゼロで初期化
            { int  i = 0 ;
              for ( ; i<m_size ; ++i )
            {
                m_code_cnt[i] = 0 ;
            }}
            // end for i
            m_code_cnt[m_size-1] = 1 << range_type::shift_bit ; // Hit しなかった数

            m_code_cnt[m_size-1] -= 1 ;
            m_code_cnt[0]        += 1 ;
            
            // range テーブル作成
            { range_value_type  sum = 0 ;
              int        i = 0 ;
              const int  max = m_code_cnt.size() ;
              for ( ; i<max ; ++ i )
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
    // Rangeテーブルに出力
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
          int        i   = 0 ;
          const int  max = m_code_cnt.size() ;
          for ( ; i<max ; ++ i )
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
    // テーブルを半分にして出力
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
          int        i   = 0 ;
          const int  max = m_code_cnt.size() ;
          for ( ; i<max ; ++ i )
        {
            // range テーブル作成
            m_out_range_tbl[i].m_width = m_code_cnt[i] ;
            m_out_range_tbl[i].m_low   = sum ;
            sum += m_code_cnt[i] ;

            // カウントテーブルを 1/2 に
            m_code_cnt[i] >>= 1 ;  // 1/2に
            m_sum += m_code_cnt[i] ;
        }}
        // end for i

        // 「その他」はゼロにしたらいけない。
        if ( m_code_cnt[m_code_cnt.size()-1] == 0 )
        {
            m_code_cnt[m_code_cnt.size()-1] = 1 ;
            m_sum += m_code_cnt[m_code_cnt.size()-1] ;
        }
        // 「0」はゼロにしたらいけない。
        if ( m_code_cnt[0] == 0 )
        {
            m_code_cnt[0] = 1 ;
            m_sum += m_code_cnt[0] ;
        }
    }

    //---------------------------------
    // 出現頻度カウント
public:
    void                      // out: なし
    Count(                    // ---: -----------
        const int &  in_code  // in : 出現コード
    )
    {
        // for ( int i = 0 ; i<4 ; ++i )
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
        }
        // end for i
        return ;
    }

} ;



//-----------------------------------------------
#ifdef __UnitTest__PPMTbl_cxx__

#include <stdio.h> // printf(), getchar()

int main()
{
    PPMTbl< RangeCode<5> >  ppm_tbl( 9 ) ; // 5bit=32, 9<16 

    // 初期値表示
    { int        i      = 0 ;
      const int  i_size = ppm_tbl.m_out_range_tbl.size() ;
      for ( ; i<i_size ; ++ i )
    {
            printf( "%d:%d-%d, ", i,
                ppm_tbl.m_out_range_tbl[i].m_low,
                ppm_tbl.m_out_range_tbl[i].m_width
            ) ;
    }}
    puts("") ;

    { int j = 0 ;
      for ( ; j<8 ; ++j )
    {
        ppm_tbl.Count( 5 ) ;
        ppm_tbl.Count( 6 ) ;
        { int i = 0 ;
          int i_size = ppm_tbl.m_out_range_tbl.size() ;
          for ( ; i<i_size ; ++ i )
        {
            printf( "%d:%d-%d, ", i,
                ppm_tbl.m_out_range_tbl[i].m_low,
                ppm_tbl.m_out_range_tbl[i].m_width
            ) ;
        }} 
        // end for i
        puts("") ;
    }}
    // end for j

    { int j = 0 ;
      for ( ; j<8 ; ++j )
    {
        ppm_tbl.Count( 6 ) ;
        ppm_tbl.Count( 7 ) ;
        { int i = 0 ;
          int i_size = ppm_tbl.m_out_range_tbl.size() ;
          for ( ; i<i_size ; ++ i )
        {
            printf( "%d:%d-%d, ", i,
                ppm_tbl.m_out_range_tbl[i].m_low,
                ppm_tbl.m_out_range_tbl[i].m_width
            ) ;
        }} 
        // end for i
        puts("") ;
    }}
    // end for j

    printf( "test end >" ) ; getchar() ;

    { int j = 0 ;
      for ( ; j<8 ; ++j )
    {
        ppm_tbl.Count( 6 ) ;
        ppm_tbl.Count( 7 ) ;
        { int i = 0 ;
          int i_size = ppm_tbl.m_out_range_tbl.size() ;
          for ( ; i<i_size ; ++ i )
        {
            printf( "%d:%d-%d, ", i,
                ppm_tbl.m_out_range_tbl[i].m_low,
                ppm_tbl.m_out_range_tbl[i].m_width
            ) ;
        }} 
        // end for i
        puts("") ;
    }}
    // end for j

    printf( "test end >" ) ; getchar() ;

    { int j = 0 ;
      for ( ; j<8 ; ++j )
    {
        ppm_tbl.Count( 6 ) ;
        ppm_tbl.Count( 7 ) ;
        { int i = 0 ;
          int i_size = ppm_tbl.m_out_range_tbl.size() ;
          for ( ; i<i_size ; ++ i )
        {
            printf( "%d:%d-%d, ", i,
                ppm_tbl.m_out_range_tbl[i].m_low,
                ppm_tbl.m_out_range_tbl[i].m_width
            ) ;
        }} 
        // end for i
        puts("") ;
    }}
    // end for j

    printf( "test end >" ) ; getchar() ;

    return 0 ;
}

#endif // __UnitTest__PPMTbl_cxx__

#endif // __PPMTbl_cxx__
