# **RE4_DASYZ2_TOOL**

Extract and Repack DAT/UDAS/MAP/DAS/DRS/DECMP files (RE4 Big Endian) [GC/WII/X360/PS3]
<br>**Important: Now this tool can extract/repack blocks compressed with YZ2;**

**Info:**
<br>License: MIT Licence / FreeBSD License
<br>Language: C++/CLI
<br>Platform: Windows
<br>Dependencies: Microsoft .NET Framework 4.8 / MSVC v143 - VS 2022 C++

# **YZ2 files can now be extracted and compressed.**
In the "DAS/UDAS" files in the ST folders, their content is compressed with YZ2 compression, now the tool will extract the content inside the compression, and recompress it automatically when repacking.
I also directly provide a tool to unpack and compress YZ2 files;

## Translate from Portuguese Brazil:

Programa destinado à extração e reempacotamento dos arquivos DAT/UDAS/MAP/DAS/DRS/DECMP do RE4 das versões Big Endian do jogo.

## RE4_DASYZ2_TOOL.exe EXTRACT

Destinado a extrair os arquivos dat/udas/map/das/drs/decmp, o programa vai criar uma pasta com o nome do arquivo, que vai conter os arquivos extraídos (sem a existência de subpastas), e também vai gerar um arquivo ".idxbig" (que é um formato próprio do programa).

## RE4_DASYZ2_TOOL.exe REPACK

Destinado a reconstruir os arquivos dat/udas/map/das/drs/decmp, ele aceita como entrada o arquivo ".idxbig", veja as especificações do arquivo mais abaixo.

## Arquivo .idxbig
Explicação do arquivo, vou usar como exemplo o arquivo "em10.udas" como referência. Ao extrair, vai gerar o arquivo "em10.idxbig" e uma pasta com o nome "em10" que vai ter os arquivos extraídos nela (sem subpastas).

## RE4_YZ2_COMPRESSION_TOOL.exe

Tool responsável por extrair os arquivos .YZ2 e recriá-los, usado para os arquivos de formato pack.yz2
<br> Para o bloco YZ2 dos arquivos DAS/UDAS a tool RE4_DASYZ2_TOOL.exe vai descompactar/compactar automaticamente.

# Acknowledgments
YZ2 compression by: Yamazaki Satoshi (Is the creator of compression)
<br>Thanks to: OAleex and Krisp (for locating the source code of the compression algorithm)

**At.te: JADERLINK**
<br>2025-05-01