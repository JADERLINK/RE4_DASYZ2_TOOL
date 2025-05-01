#pragma once
#ifndef __RangeCodeOutBuffer_cxx__
#define __RangeCodeOutBuffer_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

#include <deque> //using std::deque ;

//-------------------------------------------------------------------
//  RangeCodeOutBuffer （RangeCode専用出力バッファ）

template<
    class _OutEv // = OutEv
>
class RangeCodeOutBuffer
{
public:
    typedef typename _OutEv::value_type  value_type ;

    private : std::deque< value_type >  m_out_buff ; // 出力バッファ
    private : _OutEv &                  m_ev ;

    //---------------------------------
    //  コンストラクタ  RangeCodeOutBuffer
public:
    RangeCodeOutBuffer(
        _OutEv &  out_ev
    )
    : m_ev( out_ev )
    {
    }

    //---------------------------------
    //  デストラクタ  RangeCodeOutBuffer
public:
    ~RangeCodeOutBuffer(
    )
    {
        // たまったデータをすべて吐き出す。
        const int  size = m_out_buff.size() ;

        { int i = 0 ;
          for ( ; i<size ; ++ i )
        {
            value_type  wk = m_out_buff.front() ; // 取り出す
            m_out_buff.pop_front() ;              // 消す
            m_ev.OutEv_WriteData(
                & wk,        // in : 出力データの先頭アドレス
                sizeof( wk ) // in : 出力データ要求サイズ
            ) ;
        }}
        // end for i
    }


    //---------------------------------
    //  データ出力
public:
    void
    byte_Put(
        const value_type &  in_code
    )
    {
        m_out_buff.push_back( in_code ) ;

        return ;
    }

    //---------------------------------
    //  桁上がり
public: void
    carry_Up(
    )
    {
        //  桁上がり
        int i = m_out_buff.size() - 1 ;
        for ( ; i>=0 ; --i )
        {
            value_type &  val = m_out_buff[ i ] ;
            val ++ ;
            if ( val != 0 ) break ;
        }

        // バッファがたまっていたら吐き出す。
        const int  wk_size = 1024 ;
        if ( i > wk_size )
        {
            // バッファ出力
            value_type  wk[wk_size] ;

            { int j = 0 ;
              for ( ; j<wk_size ; ++j )
            {
                wk[j] = m_out_buff.front() ; // 取り出す
                m_out_buff.pop_front() ;     // 消す
            }}

            m_ev.OutEv_WriteData(
                wk,      // in : 出力データの先頭アドレス
                wk_size  // in : 出力データ要求サイズ
            ) ;
        }

        return ;
    }

} ;

//-----------------------------------------------
#ifdef __UnitTest__RangeCodeOutBuffer_cxx__

#include "RangeCodeInBuffer.cxx"

#include <stdio.h> // printf()

#include "../OutEv.cxx"
#include "../InEv.cxx"

// バッファが正しく動くかどうかテストする。

int main()
{
    const int  max = 100000 ;
    {
        OutEv    out_ev(
            "debug.out"   // in : 出力先ファイル名
        ) ;
        {
            RangeCodeOutBuffer< OutEv >  buff_out( out_ev ) ;

            { srand( 0 ) ;
              int  i = 0 ;
              for ( ; i<max ; ++i )
            {
                buff_out.byte_Put( (rand()%100)+1 ) ;
                if ( (rand()%5) == 0 )
                {
                    buff_out.carry_Up() ;
                }
            }}
        }
        out_ev.m_write_bin_file.Close() ;
    }
    printf( "OK>" ) ; getchar() ;

    {
        InEv  in_ev(
            "debug.out"   // in : 入力先ファイル名
        ) ;
        RangeCodeInBuffer< InEv >  buff_in( in_ev ) ;

        { srand( 0 ) ;
          int i = 0 ;
          for ( ; i<max ; ++i )
        {
            unsigned long  r1 = buff_in.byte_Get() ;
            unsigned long  r2 = (rand()%100)+1 ;

            if ( (rand()%5) == 0 ) r2 += 1 ;

            if ( r2 != r1 )
            {
                printf( "error:%d, %d != %d\n", i, r1, r2 ) ;
            }
        }}

    }
    printf( "OK>" ) ; getchar() ;

    return 0 ;
}

#endif // __UnitTest__RangeCodeOutBuffer_cxx__

#endif  // __RangeCodeOutBuffer_cxx__
