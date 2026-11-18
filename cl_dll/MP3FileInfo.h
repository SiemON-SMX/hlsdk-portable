#ifndef MP3FILEINFO_H
#define MP3FILEINFO_H

  class MP3FileInfo
   {
    public:
      inline MP3FileInfo(void) :
        szFilename(0),
        bHasLyrics(false), bHasV1Tag(false), bHasV2Tag(false), nFileSize(-1),
        nCbrBitRate(0), nVbrBitRate(0), nBitRate(0), nSampleRate(0), nLength(0),
        szBitRate(0), szMpegLayer(0), szMpegVersion(0), szChannelMode(0),
        szArtist(0), szTitle(0), szAlbum(0), szComment(0), szTrack(0), nTrack(0), szYear(0), nYear(0), szGenre(0),
        szComposer(0), szCopyright(0), szEncodedBy(0), szOriginalArtist(0), szURL(0),
        m_bOK(true)
       {
       }

      virtual ~MP3FileInfo(void) { Free(); }

      bool Init(const char* cszFilename);
      char* GetTitle(const char* cszFilename);
      void Free(void);
      inline bool isOK(void) const { return m_bOK; }

      char* szFilename;
      bool  bHasLyrics, bHasV1Tag, bHasV2Tag;
      int   nFileSize;

      int nCbrBitRate, nVbrBitRate, nBitRate, nSampleRate, nLength;
      char* szBitRate;
      char* szMpegLayer;
      char* szMpegVersion;
      char* szChannelMode;

      char* szArtist;
      char* szTitle;
      char* szAlbum;
      char* szComment;
      char* szTrack; int nTrack;
      char* szYear; int nYear;
      char* szGenre;

      char* szComposer;
      char* szCopyright;
      char* szEncodedBy;
      char* szOriginalArtist;
      char* szURL;

      void  ShowAllFrames(void);

    protected:
      bool m_bOK;

      bool m_readTags(void);
   };

#endif
