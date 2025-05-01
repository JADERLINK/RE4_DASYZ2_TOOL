#pragma once
#ifndef __RangeCodeInBuffer_cxx__
#define __RangeCodeInBuffer_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

#include <deque> // using std::deque ;

//-------------------------------------------------------------------
//  RangeCodeInBuffer (RangeDecode専用入力バッファ)
template<
    class _InEv // = InEv
>
class RangeCodeInBuffer
{
public:
    typedef typename _InEv::value_type  value_type ; // unsigned char

    private: std::deque< unsigned char >  m_in_buff ; // 入力バッファ
    private: _InEv &                      m_ev ;

    //---------------------------------
    //  コンストラクタ  RangeCodeInBuffer
public:
    RangeCodeInBuffer(
        _InEv &  in_ev
    )
    : m_ev( in_ev )
    {
    }

    //---------------------------------
    //  デストラクタ  ~RangeCodeInBuffer
public:
    ~RangeCodeInBuffer()
    {
    }

    //---------------------------------
    // データ入力
public:
    unsigned char
    byte_Get()
    {
        if ( m_in_buff.size() == 0 )
        {
            // バッファが無い場合、データを読み込む

            const int      wk_size = 1024 ;
            unsigned char  wk[wk_size] ;

            const int  size = m_ev.InEv_ReadData(
                wk,      // out: データの先頭アドレス
                wk_size  // in : データ要求サイズ
            ) ;

            // バッファに追加
            for ( int i=0 ; i<size ; ++ i)
            {
                m_in_buff.push_back( wk[i] ) ;
            }
        }

        if ( m_in_buff.size() == 0 )  return 0 ; // バッファが無い場合は０を返す

        unsigned char  rtn = m_in_buff.front() ;
        m_in_buff.pop_front() ;

        return  rtn ;
    }
} ;

#endif // __RangeCodeInBuffer_cxx__
