#pragma once
#ifndef __yz2Decode_cxx__
#define __yz2Decode_cxx__

// Copyright(c) YAMAZAKI Satoshi 2002. All rights reserved.
// 詳細については、yz2-Copyright.txt をご覧下さい。

// #include "huffman/HuffDec.cxx"   //  class  HuffmanDecode
#include "yz2RangeDec.cxx"    //  class  yz2RangeDec

//-------------------------------------------------------------------

static const unsigned short  YZ2_DEC_KEY_1BYTE = 256 ; // KEY サイズ

static const short           YZ2_DEC_DIC_SIZE  = 512 ; // 辞書 サイズ


enum YZ_DECODE_CODE                  // 終了コード
{
     YZ_DECODE_OK               = 0, // 伸長正常終了(エリアに空きあり)
     YZ_DECODE_ERR_AREAOVER     = 1, // 指定エリアを超えて書き込もうとした（入力データが異常）
     YZ_DECODE_ERR_DATA_ILLEGAL = 2  // 入力データが異常である。(異常な解凍をするデータがある) 
} ;

//-------------------------------------------------------------------

template<
    class _InEv  // 入力データ（ファイル）
>
class yz2Decode
{ 
    //-----------------------------------------------------------
    class YzDictionary
    {
        //  YZ 辞書 の基本テーブル
        //  Key が 'A'（'A'で始まる文字列）の場合の意味
        //
        //        |<-- YZ2_DEC_DIC_SIZE -->|
        //        |      (256)     |    
        //        |       V--m_cnt:最新更新位置(short)
        //        |       V--m_max:テーブル使用位置(short)
        //        +---+---+--------+                        
        //        | 3 | 5 |        |:m_lng:'A'で始まる文字列の長さ(long)
        //        +---+---+--------+ 
        //        | * | * |        |:m_ptr:'A'で始まる文字列へのポインタ列(char*)
        //        +-|-+-|-+--------+ 
        //          |   |                               
        //          V   V
        //    ".....Axx.Axxxx....":BuffAdr                  
        //
    public:
        unsigned short   m_cnt ;            // 辞書 登録位置
        unsigned short   m_max ;            // 辞書 登録数
        unsigned char*   m_ptr[YZ2_DEC_DIC_SIZE] ;  // 辞書 文字列の先頭
        long             m_lng[YZ2_DEC_DIC_SIZE] ;  // 辞書 文字列の長さ / 圧縮率向上（同じ長さ）

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
        }

        //-----------------------------
        // 辞書登録
    public :
        void                             // out: なし
        Set(                             // ---: ------------------
            unsigned char *  in_str,     // in : 登録 文字列
            const long &     in_str_size // in : 登録文字列のサイズ
        )
        {
            // テーブルに文字列のアドレスと長さを設定
            m_ptr[ m_cnt ] = in_str ;      // in : 登録 文字列
            m_lng[ m_cnt ] = in_str_size ; // in : 登録文字列のサイズ

            //  登録位置更新
            m_cnt ++ ;
            if ( m_max < m_cnt )
            {
                 m_max = m_cnt ;
            }
            m_cnt %= YZ2_DEC_DIC_SIZE ; // 最大値を超えないように mod する
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
    //        |  +------+----|--|--+------+----|--<---+--------+
    //        |<--YZ2_DEC_   |  |<--YZ2_DEC    |
    //        |  _DIC_SIZE-->|  |  _DIC_SIZE-->|
    //        |    (512)     |  |    (512)     |
    private: std::vector< YzDictionary >  m_dic_tbl ; // [YZ2_DEC_KEY_1BYTE] ; // 辞書

    //-------------------------------------------------------------------
    private: unsigned char *  m_area ;     // バッファの先頭アドレス
    private: long             m_end_pos ;  // 処理するデータの終了位置（参照バッファEND）
    private: long             m_pos ;      // 処理位置（参照バッファSTART）
    private: long             m_key_pos ;  // 検索キーの位置

    //   BuffAdr  (unsigned char*)
    //   +-----+-+-+---------------------------------+-+
    //   |     | | |                                 | |
    //   +-----+-+-+---------------------------------+-+
    //          A A                                   A
    //          | |                                   |
    //        Key AreaCount                           AreaEnd
    //            処理位置                            終了位置


    //-------------------------------------------------------------------
    //  復号化テーブル(ハフマン符号->Range符号)
//  private: HuffmanDecode< _InEv > m_entropy_decode ; // 復号化テーブル
    private: yz2RangeDec< _InEv >   m_entropy_decode ; // 復号化テーブル< 入力ファイル >

    //-----------------------
    //  コンストラクタ
public:
    yz2Decode(                               // ---: ------------------
        _InEv &       in_ev,                 // in : データ入力先
        const bool &  in_ppm_codec_flag_on   // in : true:on / false:off PPM符号化フラグ
    )
    : m_entropy_decode(        // 復号化テーブル
        in_ev,                 // in : データ入力先
        YZ2_DEC_DIC_SIZE*2 + YZ2_DEC_KEY_1BYTE,  // in : テーブルサイズ
        in_ppm_codec_flag_on   // in : true:on / false:off PPM符号化フラグ
    )
    {
        m_dic_tbl.resize( YZ2_DEC_KEY_1BYTE ) ; // 辞書
    }

    //-----------------------
    //  ＹＺ復号化データ指定
public:
    void                             // out: なし
    DecodeArea(                      // ---: --------------------------------------------
        unsigned char *  in_area,    // in : 伸長先データの先頭位置（検索バッファSTART）
        const long &     in_end_pos  // in : 伸長先データの終了位置（参照バッファEND）
    )
    {
        //-----------------------------
        //  テーブル初期化
        { int i = 0 ;
          for ( ; i<YZ2_DEC_KEY_1BYTE ; ++i )
        {
            m_dic_tbl[i].Init() ; // 辞書
        }}


        //-----------------------
        //   BuffAdr  (unsigned char*)
        //   +-----+-+-+---------------------------------+-+
        //   |     | | |                                 | |
        //   +-----+-+-+---------------------------------+-+
        //          A A                                   A
        //          | |                                   |
        //        Key AreaCount                           AreaEnd
        //            処理位置                            終了位置

        m_area    = in_area ;    // バッファの先頭アドレス
        m_end_pos = in_end_pos ; // 処理するデータの終了位置（参照バッファEND）
        m_pos     = 0 ;          // 処理位置（参照バッファSTART）
        m_key_pos = 0 ;          // 検索キーの位置

        //-----------------------
        // 頻度テーブルのクリア（圧縮率向上）
        m_entropy_decode.ReSet() ; // 復号化テーブル< 入力ファイル >

    }


    //-----------------------
    //  検索バッファ dic.m_ptr 内、一致データ列、抽出
    private: unsigned char * 
    YZ_DecodeTableSearch(
        const unsigned short &  r_code
    )
    {
        unsigned short  key = m_area[m_key_pos] ;
        YzDictionary &  dic = m_dic_tbl[ key ] ;
        unsigned short  cnt =
            (unsigned short)(( r_code + dic.m_cnt ) % YZ2_DEC_DIC_SIZE ) ;
        return  dic.m_ptr[cnt] ;
    }

    //-----------------------
    //  ＹＺ復号化処理
public:
    enum YZ_DECODE_CODE               // out: 終了コード
    Decode(                           // ---: ----------------
        const long &  in_decode_size  // in : 伸長処理する長さ
    )
    {
        long  decodeSize = in_decode_size ;

        if ( decodeSize > m_end_pos )
        {
            decodeSize = m_end_pos ; // エリア外が指定された。
        }

        // AreaCount が Size まで繰り返す
        for ( ; m_pos < decodeSize ; )
        {
            unsigned short  code = m_entropy_decode.code_Get() ; // コードを取得
            long            size = 1 ;

            // コードの種類により分岐
            // 
            // YZ2_DEC_DIC_SIZE が２５６の場合
            // code の値     ：意味
            // --------------：---------------
            //     ０～２５５：一致データ列あり（長さ指定あり）
            // ２５６～５１１：一致データ列あり（長さの変化無し）
            // ５１２～７６７：一致データ列なし（０～２５５のデータ）
            // 

            if ( code >= YZ2_DEC_DIC_SIZE*2 )
            {
                //  一致データ列なし（０～２５５のデータ）
                //  コードをそのまま BuffAdr に格納する

                m_area[m_pos] = (unsigned char)(code - YZ2_DEC_DIC_SIZE*2) ;
                m_pos += 1 ;
            }
            else
            {
                //  一致データ列、あり

                unsigned char *  moto ; // 転送元

                if ( code < YZ2_DEC_DIC_SIZE )
                {
                    // 一致データ列あり（長さ指定あり）

                    size = m_entropy_decode.length_Get() ; // 長さをデコード
                    size += 2 ;
                    moto = YZ_DecodeTableSearch( code ) ;
                }
                else
                {
                    //  一致文字列の長さをテーブルよりデコード

                    unsigned short  key = m_area[ m_key_pos ] ;
                    YzDictionary &  dic = m_dic_tbl[ key ] ;
                    unsigned short  wk  =
                        (unsigned short)(( code + dic.m_cnt ) % YZ2_DEC_DIC_SIZE ) ;

                    size = dic.m_lng[wk] ;

                    code -= YZ2_DEC_DIC_SIZE ;

                    moto = YZ_DecodeTableSearch( code ) ;
                    /*{
                        // 圧縮率向上（cnt にデータが有るなら、同じ長さを入れ替える cnt<->wk）

                        unsigned short  cnt = dic.m_cnt ;
                        unsigned char * cp  = dic.m_ptr[cnt] ;
                        if ( cp != 0 )
                        {
                            //  データ登録
                            dic.m_ptr[wk] = dic.m_ptr[cnt] ;
                            dic.m_lng[wk] = dic.m_lng[cnt] ;
                        }
                    }*/
                }

                // データエラー(size)チェック
                if ( m_pos+size > m_end_pos )
                {
                    // err:エリアを超えて書き込もうとしている
                    return YZ_DECODE_ERR_AREAOVER ;
                }

                // データエラー(moto)チェック
                if ( moto <    m_area 
                  || moto >= & m_area[ m_end_pos ]
                   )
                {
                    // err:入力データが異常である。
                    return YZ_DECODE_ERR_DATA_ILLEGAL ;
                }

                //  文字列転送
                {
                    unsigned char *  saki = & m_area[ m_pos ] ; // 転送先
                    long  i = 0  ;

                    for ( ; i<size ; i++ )
                    {
                        *saki++ = *moto++ ;
                    }
                    // 高速化（失敗：BC++ではうまく行かない）
                    // memcpy( & m_area[m_pos], moto, size ) ;
                }

                //  処理位置を進める
                m_pos += size ;
            }

            //-------------------------------------------------------------------
            //  検索バッファ dic.m_ptr へ、登録
            {
                if ( m_key_pos < m_pos-1 ) // １文字以上の文字列は登録
                {
                    //  登録
                    YzDictionary &  dic = m_dic_tbl[ m_area[ m_key_pos ] ] ;
                    dic.Set(
                        & m_area[ m_key_pos + 1 ], // in : 登録文字列
                        size                       // in : 登録文字列の長さ
                    ) ;

                    m_key_pos = m_pos - 1 ;
                }
            }
            //-------------------------------------------------------------------
        }
        // AreaCount が Size まで繰り返す end for ( ; m_pos < decodeSize ; )

        return  YZ_DECODE_OK ;
    }

} ;

//===================================================================

#endif  // __yz2Decode_cxx__
