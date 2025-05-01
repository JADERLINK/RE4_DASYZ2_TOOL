#pragma once
#ifndef __yz2Encode_cxx__
#define __yz2Encode_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

// #include "huffman/HuffEnc.cxx"   //  class  HuffmanEncode
#include "yz2RangeEnc.cxx"    //  class  yz2RangeEnc

//-------------------------------------------------------------------

static const unsigned short  YZ2_ENC_KEY_1BYTE = 256 ; // KEY サイズ

static const short           YZ2_ENC_DIC_SIZE = 512 ; // 辞書 サイズ（512 は遅い。）


//-------------------------------------------------------------------
template<
    class _OutEv  // 出力データ（ファイル）
>
class yz2Encode
{
    class HiSpeedLink
    {
        //-------------------------------------------------------------------
        //  高速検索処理用リンクテーブル

        // 注意。↓は１バイトはみ出して参照するため yz2EncFiles で１バイト分多く確保している
        #define  BASE_KEY(A) ( ((*(A))<<1) ^ (*(A+1)) )
        private: short  m_base[256<<1] ;   // 256 Kbyte 高速化

        private: short  m_next[YZ2_ENC_DIC_SIZE] ; // 128 Kbyte 高速化
        private: short  m_prev[YZ2_ENC_DIC_SIZE] ; // 128 Kbyte 高速化

        //         m_base           m_next            m_prev
        //         (short*)         (short*)          (short*)
        //   ---- +--------------+  +--------------+  +--------------+
        //    A   |              |  |              |  |              |
        //    |   |              |  |              |  |              |
        //   KEY1 |              |  |              |  |              |
        //   BYTE |              |  |              |  |              |
        //   (256)|              |  |              |  |              |
        //    |   |              |  |              |  |              |
        //    V   |              |  |              |  |              |
        //   ---- +--------------+  +--------------+  +--------------+
        //        |              |  |              |  |              |
        //        |<--YZ2_ENC_   |  |<--YZ2_ENC_   |  |<--YZ2_ENC_   |
        //        |  KEY_1BYTE-->|  |   DIC_SIZE-->|  |   DIC_SIZE-->|
        //            (256<<1)           (512)             (512)
        //
        //  Key が 'A'（'AZ..'で始まる文字列）の場合の意味
        //
        //                +---------------------+ +---+
        //        m_base  |                     | |   |
        //        +-----+-|-+-----+       +----+V-|-+-V--+------+
        //        |     : | :     | m_next|    |  * | 0  |      |
        //        +-----+-|-+-----+       +----+----+----+------+
        //    'A':|     | * |     | m_prev|    | 0  | *  |      |
        //        +-----+---+-----+       +----+-A--+-|--+------+
        //        |     :   :     |       :    : |    |  :      :
        //        +-----+---+-----+       :    : +----+  :      :
        //              |'Z'|             +----+----+----+------+
        //                   dic.m_ptr'A':| *  | *  | *  |      |
        //                                +-|--+-|--+-|--+------+
        //                                  |    |    |
        //                                  V    V    V
        //                      m_area : '..Ax...AZ...AZ...'

        //-----------------------------
        // コンストラクタ
    public :
        void
        Init()
        {
            memset( m_base, -1, sizeof(m_base) ) ;
            memset( m_next, -1, sizeof(m_next) ) ;
            memset( m_prev, -1, sizeof(m_prev) ) ;
        }

        //-----------------------------
        // 最初の登録位置（next ポインタ）
    public :
        short
        next_start_Get(
            unsigned char *  in_ptr // in : 検索文字列
        )
        {
            return  m_base[ BASE_KEY( in_ptr ) ] ;
        }

        //-----------------------------
        // 次の登録位置（ next ポインタ）
    public :
        short
        next_Get(
            const short &  in_nxt // in : 登録位置（next ポインタ）
        )
        {
            return  m_next[ in_nxt ] ;
        }

        //-----------------------------
        // リンク設定
    public :
        void                                // out: なし
        link_Set(                           // ---: ------------
            const short &    in_cnt,        // in : 登録位置
            unsigned char *  in_delete_ptr, // in : 解除文字列
            unsigned char *  in_set_ptr     // in : 登録文字列
        )
        {
            // まず、今、入ってるのを「はずす」。
            {
                short  nxt = m_next[ in_cnt ] ;
                short  prv = m_prev[ in_cnt ] ;

                if ( nxt >= 0 )
                {
                    // 次があるので、次の Prev を書き換え
                    m_prev[ nxt ] = prv ;
                }
                if ( prv >= 0 )
                {
                    // 前があるので、前の Next を書き換え
                    m_next[ prv ] = nxt ;
                }
                else
                {
                    // 前が無い（前はBase なので Base を消す）
                    if ( in_delete_ptr != 0 )
                    {
                        m_base[ BASE_KEY(in_delete_ptr) ] = nxt ;
                    }
                }
            }

            // そして、新しいのを「はる」
            {
                short  base = m_base[ BASE_KEY( in_set_ptr ) ] ;

                m_next[ in_cnt ] = base ;
                m_prev[ in_cnt ] = -1 ;

                m_base[ BASE_KEY( in_set_ptr ) ] = in_cnt ;

                if ( base >= 0 )
                {
                    m_prev[ base ] = in_cnt ;
                }
            }
        }

        #undef BASE_KEY
    } ;

    //-----------------------------------------------------------
    class YzDictionary
    {
        //  YZ 辞書 の基本テーブル
        //  Key が 'A'（'A'で始まる文字列）の場合の意味
        //
        //        |<-YZ2_ENC_DIC_SIZE->| (512)
        //        |                    |
        //        |       V--m_cnt:最新更新位置(short)
        //        |       V--m_max:テーブル使用位置(short)
        //        +---+---+------------+
        //        | 3 | 5 |            |:m_lng:'A'で始まる文字列の長さ(long)
        //        +---+---+------------+ 
        //        | * | * |            |:m_ptr:'A'で始まる文字列へのポインタ列(char*)
        //        +-|-+-|-+------------+ 
        //          |   |
        //          V   V
        //    ".....Axx.Axxxx....":m_area                  
        //
    public:
        unsigned short   m_cnt ;            // 辞書 登録位置
        unsigned short   m_max ;            // 辞書 登録数
        unsigned char*   m_ptr[YZ2_ENC_DIC_SIZE] ;  // 辞書 文字列の先頭
        long             m_lng[YZ2_ENC_DIC_SIZE] ;  // 辞書 文字列の長さ / 圧縮率向上（同じ長さ）

        HiSpeedLink      m_hi_speed_link ;

        //-----------------------------
        // コンストラクタ
    public :
        void
        Init()
        {
            m_cnt = 0 ;  // 辞書 登録位置
            m_max = 0 ;  // 辞書 登録数
            memset( m_ptr, 0, sizeof(m_ptr) ) ;
            memset( m_lng, 0, sizeof(m_lng) ) ;

            m_hi_speed_link.Init() ;
        }

        //-----------------------------
        // 辞書登録
    public :    
        void                              // out: なし
        Set(                              // ---: ------------- 
            unsigned char *  in_str,      // in : 登録文字列
            const long &     in_str_size  // in : 文字列の長さ
        )
        {
            // 高速化用リンク登録
            m_hi_speed_link.link_Set(
                m_cnt,          // in : 登録位置
                m_ptr[ m_cnt ], // in : 解除文字列
                in_str          // in : 登録文字列
            ) ;

            //  で、辞書登録
            m_ptr[ m_cnt ] = in_str  ;
            m_lng[ m_cnt ] = in_str_size ; // 圧縮率向上（同じ長さ）

            // 最後に、次の辞書登録位置更新
            m_cnt ++ ;
            if ( m_max < m_cnt )
            {
                 m_max = m_cnt ;
            }
            m_cnt %= YZ2_ENC_DIC_SIZE ;
        }
    } ;

    //  このYzDictionaryがKeyの文字種類数２５６個あつまり、以下のようなテーブルになる。
    // 
    //         m_ptr            m_lng             m_cnt     m_max
    //         (char**)         (long*)           (short[]) (short[])
    //   ---- +--------------+  +--------------+  +------+  +------+
    //    A   | 文字列への   |  | 文字列の     |  |登録数|  |最大数|
    //    |   | ポインタ     |  | 長さ         |  |      |  |      |
    //   KEY1 |              |  | (圧縮率向上) |  |      |  |      |
    //   BYTE |              |  |              |  |      |  |      |
    //   (256)|              |  |              |  |      |  |      |
    //    |   |              |  |              |  |      |  |      |
    //    V   |              |  |              |  |      |  |      |
    //   ---- +--------------+  +--------------+  +------+  +------+
    //        |  A      A    |  |  A      A    |      |        |
    //        |  |cnt   |max |  |  |cnt   |max |      |        |
    //        |  +------+====|==|==+======+====|==<===+--------+
    //        |<--YZ2_ENC    |  |<--YZ2_ENC    |
    //        |   _DIC_SIZE->|  |   _DIC_SIZE->|
    //        |    (512)     |  |    (512)     |

    private: std::vector< YzDictionary >  m_dic_tbl ; // [YZ2_ENC_KEY_1BYTE] ; // 辞書


    //-------------------------------------------------------------------
    private: unsigned char *  m_area ;    // バッファの先頭アドレス
    private: long             m_end_pos ; // 処理するデータの終了位置（参照バッファEND）
    private: long             m_pos ;     // 処理位置（参照バッファSTART）
    private: long             m_key_pos ; // 検索キーの位置

    //   m_area  (unsigned char*)
    //   +-----+-+-+---------------------+-------------+
    //   |     | | |                     |             |
    //   +-----+-+-+---------------------+-------------+
    //          A A                      A
    //          | |                      |
    //  m_key_pos m_pos                  m_end_pos
    //            処理位置               終了位置

    //-------------------------------------------------------------------
    //  符号化テーブル (ハフマン符号->Range符号へ) 

    // typedef  HuffmanEncode< _OutEv >  EntropyEncodeType ; // エントロピー符号化
    typedef yz2RangeEnc< _OutEv >  EntropyEncodeType ; // エントロピー符号化

    private: EntropyEncodeType     m_entropy_encode ; // yz2RangeEnc<> 出力ファイル

    //-----------------------
    //  コンストラクタ
public:
    yz2Encode(
        _OutEv &      in_out_ev,             // in : データ出力先
        const bool &  in_ppm_codec_flag_on   // in : true:on / false:off PPM符号化フラグ
    )
    : m_entropy_encode(       // エントロピー符号 yz2RangeEnc<>
        in_out_ev,            // 出力データ
        YZ2_ENC_DIC_SIZE*2,   // 頻度表 code サイズ
        256,                  // 頻度表 leng サイズ
        in_ppm_codec_flag_on  // true:on / false:off PPM符号化フラグ
    )
    {
        m_dic_tbl.resize( YZ2_ENC_KEY_1BYTE ) ; // 辞書
    }

    //-----------------------
    //  ＹＺ符号化データ指定
public:
    void                            // out: 無し
    EncodeArea(                     // ---: -----------------------------
        unsigned char *  in_buff    // in : 処理するデータの先頭位置（検索バッファSTART）
    )
    {
        //-----------------------------
        //  テーブル初期化
        { int i = 0 ;
          for ( ; i<YZ2_ENC_KEY_1BYTE ; ++i )
        {
            m_dic_tbl[i].Init() ; // 辞書
        }}

        //-----------------------------
        m_area = in_buff ; // バッファの先頭アドレス

        //-----------------------------
        m_pos     = 0 ;          // 処理位置（参照バッファSTART）
        m_key_pos = 0 ;          // 検索キーの位置

        //-----------------------
        // 頻度テーブルのクリア（圧縮率向上）
        m_entropy_encode.ReSet() ; // 出力ファイル
    }

    //---------------------------------
    // 辞書内検索
    private: void               // out: なし
    StrSearch(                  // ---: -----------------
        long &   out_find_size, // out: 文字列の長さ
        short &  out_find_pos   // out: 辞書内の登録位置
    )
    {
        //-----------------------------
        //  検索バッファ m_dic_ptr 内、一致データ列、検索

        unsigned short  key = m_area[ m_key_pos ] ;
        YzDictionary &  dic = m_dic_tbl[ key ] ;

        if ( m_pos     >= 1
          && dic.m_max != 0
           )
        {
            { unsigned char *  find_end_char = & m_area[m_pos] ;     // 参照データ列
              long             find_area_max =   m_end_pos - m_pos ; // 参照データ列の最後までのサイズ

              short next = dic.m_hi_speed_link.next_start_Get( find_end_char ) ; //  高速化
              for ( ; next >= 0 ; next = dic.m_hi_speed_link.next_Get( next ) )  //  高速化
//+           for ( short next=0 ; next<dic.m_max ; ++next ) //+ 低速版 for ^^;
            {
                unsigned char *  dic_str = dic.m_ptr[next] ; // 検索データ列の最初

                //  データ列の比較（後ろから比較している）

                unsigned char *  k_str = &dic_str[out_find_size] ; //  検索データ列
                unsigned char *  s_str = find_end_char ;           //  参照データ列

                if ( *k_str == *s_str )
                {
                    k_str-- ; s_str-- ;

                    // データ列が同じか？
                    // 本当は高速な memcmp() を使いたいのだがなぜかうまく動作しない。memcmp() のバグか？

                    long  j = (long)(out_find_size - 1) ;
                    for ( ; j >= 0 ; j-- )
                    {
                        if ( *k_str-- != *s_str-- )
                        {
                            goto SEARCH_END ; // 異なるので、次。
                        }
                    }

                    // 同じである。さらに調べる。

                    k_str = & dic_str[out_find_size+1] ; //  検索データ列
                    s_str = & find_end_char[1] ;         //  参照データ列

                    for ( j=out_find_size+1 ; j<find_area_max ; j++ )
                    {
                        if ( *k_str++ != *s_str++ )
                        {
                            break ;
                        }
                    }

                    //  一致データ列、登録

                    find_end_char = & find_end_char[j-out_find_size] ; //  参照データ列
                    out_find_size = j ;
                    out_find_pos  = next ;

                    //  参照バッファ内？
                    if ( out_find_size >= find_area_max )
                    {
                        break ; // 参照バッファ外 exit for loop
                    }
                }
                //  データ列の比較終了
                SEARCH_END :;
            }}
            // end for ( ; next >= 0 ; )
            // 検索終了

            //-------------------------
            // 検索あと処理

            if ( out_find_size > 0 )
            {
                short  wk = out_find_pos ;

                // 検索位置 out_find_pos を m_dic_cnt[key] からの相対位置にする

                out_find_pos += (short)( YZ2_ENC_DIC_SIZE - dic.m_cnt ) ;
                out_find_pos %= (short)YZ2_ENC_DIC_SIZE ;

                // 圧縮率向上（同じ長さ）
                if ( out_find_size == dic.m_lng[wk] )
                {
                    // 長さが同じ場合 Count 値を上げる

                    out_find_pos += (short)YZ2_ENC_DIC_SIZE ;

/*　 0.4% の圧縮率アップ（いる？いらないよねー^^;）

                    if ( out_find_size >= 2 )
                    {
                        // 圧縮率向上（cnt にデータが有るなら、同じ長さを入れ替える cnt<->wk）
                        // 見つけた findStr と最新の KeyCount と入れ替える

                        unsigned short  cnt = dic.m_cnt ;
                        unsigned char * cp  = dic.m_ptr[cnt] ;
                        if ( cp != 0 )
                        {
//*--
                            HiSpeedLink &    lnk = dic.m_hi_speed_link ;
                            short  nxt = lnk.m_next[wk] ;
                            short  prv = lnk.m_prev[wk] ;

                            // まず、見つけたところ (wk) を「はずす」。
                            if ( nxt >= 0 )
                            {
                                // 次があるので、次の Prev を書き換え
                                lnk.m_prev[nxt] = prv ;
                            }
                            if ( prv >= 0 )
                            {
                                // 前があるので、前の Next を書き換え
                                lnk.m_next[prv] = nxt ;
                            }
                            else
                            {
                                unsigned char * cpw = dic.m_ptr[ wk ] ;
                                if ( cpw != 0 )
                                {
                                    lnk.m_base[(*cpw)] = nxt ;
                                }
                            }

                            // そして、これから上書きになるデータ cnt を wk に「はる」

                            unsigned char * ucp = dic.m_ptr[cnt] ;
                            short          base = lnk.m_base[(*ucp)] ;

                            lnk.m_next[wk] = base ;
                            lnk.m_prev[wk] = -1 ;

                            lnk.m_base[(*ucp)] = wk ;

                            if ( base >= 0 )
                            {
                                lnk.m_prev[base] = wk ;
                            }
//*--
                            //  で、データ登録
                            dic.m_ptr[wk] = dic.m_ptr[cnt] ;
                            dic.m_lng[wk] = dic.m_lng[cnt] ;
                        }
                        // end if cp != 0
                    }
                    // end if size>=2
*/ 
                }
                // end if
            }
            //end if
        }
        // end if
    }

    //---------------------------------
    //  ＹＺ符号化処理
public:
    void                               // out: 無し
    Encode(                            // ---: -----------------
        const long &  in_encode_size,  // in : 圧縮処理する長さ
        const long &  in_end_pos       // in : 処理するデータの終了位置（参照バッファEND）
    )
    {
        m_end_pos = in_end_pos ; // 処理するデータの終了位置（参照バッファEND）

        long  encodeSize = in_encode_size ;

        if ( encodeSize > m_end_pos )
        {
            encodeSize = m_end_pos ; // エリア外が指定された。
        }

        for ( ; m_pos < encodeSize ; )
        {
            //  検索バッファ内、検索
            //  検索テーブル m_dic_ptr 内に対象の文字列 m_pos が
            //  登録されているかどうか探す。

            long   find_str_size  = 0 ; // 一致データ列の長さ 0:無し
            short  find_str_pos   = 0 ; // 一致データ列の位置(いくつ前のキーか) 

            StrSearch( find_str_size, find_str_pos ) ;

            //-------------------------------------------------------------------

            if ( find_str_size < 2 )
            {
                //  一致データ列、なし（参照バッファ内、先頭データ出力）
                //  検索テーブル m_dic_ptr に無い場合、対象の文字 m_pos をそのまま出力

                unsigned short code =
                    (unsigned short)(m_area[m_pos] + YZ2_ENC_DIC_SIZE * 2) ; // 長さ）
                m_entropy_encode.code_Put( /*last_code,*/ code ) ; //  先頭データ
            
                // debug_code.Count( code ) ;

                m_pos += 1 ;   // 参照バッファ、移動（シフト）
            }
            else
            {
                //  一致データ列、あり（位置を出力）
                //  検索テーブル m_dic_ptr に登録されていた場合、
                //  一致した文字列の長さを確認する

                // debug_code.Count( find_str_pos ) ;
                
                m_entropy_encode.code_Put( /*last_code,*/ find_str_pos ) ;    // 位置

                if ( find_str_pos < YZ2_ENC_DIC_SIZE )         // 圧縮率向上（同じ長さ）
                {
                    //  一致データ列、あり（長さを出力）

                    // debug_leng.Count( find_str_size - 2 ) ;

                    m_entropy_encode.length_Put( (long)find_str_size - 2 ) ; //  長さ
                }
                m_pos += find_str_size ; // 参照バッファ、移動（シフト）
            }
            // end if

            //  検索バッファへ、登録

            //-------------------------------------------------------------------
            //  検索バッファ m_dic_ptr へ、登録
            {
                if ( m_key_pos < m_pos-1 ) // １文字以上ヒットしていたら、辞書に登録。
                {
                    //  辞書登録
                    YzDictionary &    dic = m_dic_tbl[ m_area[ m_key_pos ] ] ;

                    dic.Set(
                        & m_area[m_key_pos + 1], // 登録文字列
                        find_str_size            // 文字列のサイズ
                    ) ;

                    m_key_pos = m_pos-1 ;
                }
                // end if
            }

            //-------------------------------------------------------------------
        }
        // end for
    }
} ;

//===================================================================

#ifdef __UnitTest__yz2Encode_cxx__

#include "../../cxl/FileNameExp.cxx" //class CXL::FileNameExp"
#include "../../cxl/HeapMemory.cxx"  //class CXL::HeapMemory
#include "../../cxl/BinFileIn.cxx"   //class CXL::BinFileIn
#include "OutEv.cxx"                 //class OutEv
#include "InEv.cxx"                  //class InEv

#include "yz2Decode.cxx" //class yz2Decode

void encode_file(
    const CXL::FileName  &  in_filename
)
{
    // ファイルの読み込み
    CXL::HeapMemory  heap_mem ;
    {
        std::vector< unsigned char > buff ;

        CXL::BinFileIn( in_filename ).Get( buff ) ;
        heap_mem.Set( buff ) ;
    }

    // 圧縮
    {
        OutEv               out_ev( "debug.out" ) ; // 出力ファイル
        yz2Encode< OutEv >  yz2_enc( out_ev ) ;

        yz2_enc.EncodeArea(            // ---: -----------------------------
            (unsigned char *)heap_mem  // in : 処理するデータの先頭位置（検索バッファSTART）
        ) ;

        //  ＹＺ符号化処理
        yz2_enc.Encode(              // ---: -----------------
            heap_mem.size_Get(),     // in : 圧縮処理する長さ
            heap_mem.size_Get()      // in : 処理するデータの終了位置（参照バッファEND）
        ) ;

        // ファイルクローズ
        out_ev.m_write_bin_file.Close() ;

        const int p = out_ev.m_write_bin_file.write_size_Get() * 1000 / heap_mem.size_Get() ;
        printf( "%d byte -> %d byte (%d.%1d%%)\n",
            heap_mem.size_Get(),
            out_ev.m_write_bin_file.write_size_Get(),
            p / 10, p % 10
        ) ;
    }

    // 解凍
    CXL::HeapMemory   dec_mem( heap_mem.size_Get() ) ;
    {
        InEv              in_ev( "debug.out" ) ;
        yz2Decode< InEv > yz2_dec( in_ev ) ;

        yz2_dec.DecodeArea(            // ---: --------------------------------------------
            (unsigned char *)dec_mem,  // in : 伸長先データの先頭位置（検索バッファSTART）
            dec_mem.size_Get()         // in : 伸長先データの終了位置（参照バッファEND）
        ) ;

        //  ＹＺ復号化処理
        int rtn_code = yz2_dec.Decode(  // ---: ----------------
            dec_mem.size_Get()          // in : 伸長処理する長さ
        ) ;

        printf( "0=OK : %d\n", rtn_code ) ;
    }

    // 比較
    const int  size_max = heap_mem.size_Get() ;
    for ( int i=0 ; i<size_max ; ++i )
    {
        if ( heap_mem[i] != dec_mem[i] )
        {
            printf( "error:%d\n", i ) ;
            break ;
        }
    }
}

int main()
{
    encode_file( "yz2Encode.cxx" ) ;
    encode_file( "yz2Decode.cxx" ) ;

    printf( "Hit [enter] Key!" ) ; getchar() ;

    return 0 ;
}

#endif // __UnitTest__yz2Encode_cxx__


#endif // __yz2Encode_cxx__
