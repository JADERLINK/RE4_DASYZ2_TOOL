#pragma once
#ifndef __FrequencyEncode_cxx__
#define __FrequencyEncode_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

// STL
#ifdef WIN32
#ifdef _DEBUG

  #pragma warning( disable : 4786 )

#endif // _DEBUG
#endif // WIN32

#include "RangeEncode.cxx"     // class RangeEncode<>   エントロピー符号

//-------------------------------------
//  頻度表をもちいた符号化

template<
    class _OutEv,           // 出力データ
    class _FrequencyTblType // 頻度表 Frequency4Tbl
>
class FrequencyEncode
{
    private : typedef typename _FrequencyTblType::range_type  range_type ; // RangeCode<>

    private : RangeEncode< _OutEv, range_type > &  m_range_encode ;    // エントロピー符号
    private : _FrequencyTblType                    m_frequency_tbl ;   // 頻度表

    //---------------------------------
    // コンストラクタ
public:
    FrequencyEncode(
        RangeEncode< _OutEv, range_type > &  out_encode,        // out: RangeEncode (データの出力先)
        const int &                          in_code_size = 256 // in : 0～255 のコードの出現頻度を 0～4095 の値で現す
    )
    : m_range_encode( out_encode )
    , m_frequency_tbl( in_code_size )
    {
    }

    //---------------------------------
    //  符号化
public:
    void                          // out: なし
    Encode(                       // ---: -----------------
        const int &  in_code      // in : 符号化するコード
    )
    {
        m_range_encode.shift_Encode(
            m_frequency_tbl.m_out_range_tbl[ in_code ]
        ) ;
        m_frequency_tbl.Count( in_code ) ;
    }

    //---------------------------------
    //  頻度テーブルのリセット（クリア）
public:
    void                          // out: なし
    ReSet(                        // ---: -----------------
    )                             // in : なし
    {
        m_frequency_tbl.ReSet() ;
    }
} ;

//===============================================
#ifdef __UnitTest__FrequencyEncode_cxx__

#include "../../../cxl/SystemTime.cxx" // class CXL::SystemTime
#include "../../../cxl/BinFileIn.cxx"  // class CXL::BinFileIn

#include "Frequency4Tbl.cxx"   // class Frequency4Tbl<>  頻度表

#include "../InEv.cxx"         // class InEv
#include "FrequencyDecode.cxx" // class FrequencyDecode

#include <string>  // STL using  std::string ;

//-------------------------------------
//
void  test(
    const vector< unsigned char > &  input_data 
)
{
    typedef  RangeCode< 13, unsigned short, unsigned long >  Range13Type ;
    typedef  Frequency4Tbl< Range13Type >                    Freq4Type ;

    long  input_size = input_data.size() ;

    // 符号化
    {
        CXL::SystemTime    sys_time ; // 時間を計る
        {
            OutEv  debug_out( "debug.out" ) ; // 出力先ファイル
            {
                RangeEncode< OutEv, Range13Type >   range_encode( debug_out ) ;   // Range符号
                FrequencyEncode< OutEv, Freq4Type > freq_enc( range_encode, 257 ) ; // 頻度テーブル

                for ( int i=0 ; i<input_size ; i++ )
                {
                    freq_enc.Encode( input_data[i] ) ;
                }
                freq_enc.Encode( 256 ) ;
            } // <- このスコープで RangeCodeOutBuffer をフラッシュする
            debug_out.m_write_bin_file.Close() ;

            int  encode_size = debug_out.m_write_bin_file.write_size_Get() ;
            int  per = (int)((double)encode_size*(double)10000/(double)input_size) ;
            printf( "size: %d byte (%d.%02d%%) ", encode_size, per/100, per%100 ) ;
        }
        printf( "time: %s sec.\n", sys_time.str_Get() ) ;
    }

    // 復号化
    {
        std::vector< unsigned char >   decode_data ;

        CXL::SystemTime   sys_time ; // 時間を計る
        {
            InEv                                debug_out( "debug.out" ) ;      // 入力ファイル
            RangeDecode< InEv, Range13Type >    range_decode( debug_out ) ;     // Range復号
            FrequencyDecode< InEv, Freq4Type >  freq_dec( range_decode, 257 ) ; // 頻度テーブル

            for ( int i=0 ; i<input_size ; ++i )
            {
                int  code = freq_dec.Decode() ;

                if ( code >= 256 )
                {
                    break ; // 終了
                }
                if ( code != (int)input_data[i] )
                {
                    printf( "i:%d (%d)\n", i-2, (int)input_data[i-2] ) ;
                    printf( "i:%d (%d)\n", i-1, (int)input_data[i-1] ) ;
                    printf( "i:%d (%d!=%d)\n", i, code, (int)input_data[i] ) ;
                    printf( "i:%d (%d  %d)\n", i+1, freq_dec.Decode(), (int)input_data[i+1] ) ;
                    printf( "i:%d (%d  %d)\n", i+2, freq_dec.Decode(), (int)input_data[i+2] ) ;

                    printf( "error >" ) ; getchar() ;

                    break;
                }
                decode_data.push_back( (unsigned char)code ) ;
            }
        }

        int  decode_size = decode_data.size() ;
        printf( "size: %d byte (%d) ", decode_size, input_size ) ;
        printf( "time: %s sec.\n", sys_time.str_Get() ) ;
    }
}

//-------------------------------------
//
void  file_test(
    const CXL::FileName &  in_filename
)
{
    // ファイルを読み込む
    std::vector< unsigned char >  input_data ;
    if ( CXL::BinFileIn( in_filename ).Get( input_data ) == 0 )
    {
        // エラー
        printf( "No file.('%s')\n", in_filename.c_str() ) ;

        return  ;
    }

    puts( "----" ) ;
    puts( in_filename.c_str() ) ;

    test( input_data ) ;
}

//-------------------------------------
//
int main()
{
    int  j = 0 ; 
    // for (; j<1000*1000 ; ++j )
    {
        printf( "---------rand(%d)\n", j ) ;

        {
            vector< unsigned char >  data ;
            const int  test_size = 1000*1000*5 ; // 5MByte

            srand( j ) ;
            { int  i = 0 ;
              for ( ; i<test_size ; i++ )
            {
                data.push_back(
                    (unsigned char)(rand() % 256)
                ) ;
            }}
            puts( "----" ) ;
            puts( "rand() % 256" ) ;
            test( data ) ;
        }
        {
            vector< unsigned char >  data ;
            const int  test_size = 5*1000*1000 ; // 5MByte

            srand( j ) ;
            { int  i = 0 ;
              for ( ; i<test_size ; i++ )
            {
                data.push_back(
                    (unsigned char)((rand()%65) + (rand()%65) + (rand()%65) + (rand()%64))
                ) ;
            }}
            puts( "----" ) ;
            puts( "(rand()%65) + (rand()%65) + (rand()%65) + (rand()%64)" ) ;
            test( data ) ;
        }
    }

    file_test( "large/bible.txt" ) ;
    file_test( "large/E.coli" ) ;
    file_test( "large/kjv.gutenberg" ) ;
    file_test( "large/world192.txt" ) ;

    printf( "end >" ) ; getchar() ;

    return 0 ;
}

#endif // __UnitTest__FrequencyEncode_cxx__

#endif // __FrequencyEncode_cxx__
