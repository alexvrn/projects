using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Security.Cryptography;
using Microsoft.Win32;
using System.Windows.Forms;
using System.Text.RegularExpressions;
using System.Runtime.InteropServices;
using System.Drawing;

namespace GDStorage.FileUtils
{
    using DWORD = UInt32;
    using WORD = UInt16;
    using DATE = Double;
    using BYTE = Byte;
    using HANDLE = System.IO.FileStream;
    using size_t = UInt64;
    using IO = System.IO;
    using point_t = PointF;
    using System.Diagnostics;

    public class DigestWriterUtils
    {
        public static string FORMAT_DIGEST = ".digest";

        public static DWORD c_dwDvrFrameMarker = 0x56801234;

        public static float c_m_per_smile = 1609.3F;

        public static int c_nMaxBufferSize = 0x2000000; // =32M.

        const float c_PI = 3.14159265358979f;

        public class trackpoint_info_t
        {
            public trackpoint_info_t()
            {
                wFlags = 0;
            }

            public DATE dtTime;
            public float fAltitude;    // In m.
            public float fSpeed;       // In km/h.
            public float fDepth;       // In m.
            public float fTemperature; // In Celsius degrees.
            public float fDOP;
            public float fHDOP;
            public float fVDOP;
            public float fHeading;     // In degrees.
            public size_t cVideoData0;
            public size_t cVideoData1;

            public enum DefParam
            {
                defTime = 0x0001,
                defTimeMilliseconds = 0x0002, // In addition to defTime
                defAltitude = 0x0004,
                defSpeed = 0x0008,
                defDepth = 0x0010,
                defTemperature = 0x0020,
                defDOP = 0x0040,
                defHDOP = 0x0080,
                defVDOP = 0x0100,
                defSats = 0x0200,
                defFix = 0x0400,
                defHeading = 0x0800,
                defVideo = 0x1000
            };
            public enum Fix_t
            {
                fixNone,
                fix2D,
                fix3D,
                fixDGPS,
                fixPPS
            };

            public WORD wFlags;
            public BYTE btSats;
            public BYTE btFix;
        }


        [Serializable]
        struct CDvrDigestRecord
        {
            public ulong ullFrameHdrOffset;
            public float x, y;
            public DATE dtTime;
            public float fSpeed;
            public DWORD dwReserved0;
            public DWORD dwReserved1;
        };

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public class CDvrFrameHdr
        {
            public DWORD dwStartMarker;

            public BYTE wFramesRate;
            public WORD w0;

            public WORD wPrevFrameSizeBy256;
            public WORD w1;
            public WORD w21;
            public WORD w22;
            public WORD wFrameSizeBy256;
            public WORD w2;
            public WORD w11;
            public WORD w12;

            public BYTE btSecond;
            public BYTE btMinute;
            public BYTE btHour;
            public BYTE btDay;
            public BYTE btMonth;
            public BYTE btYear;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
            public BYTE[] bt1;

            public DWORD dwHdrAndJpgSize;

            public WORD w00;
            public WORD w01;
            public WORD w02;
            public WORD w03;
            public WORD w04;
            public WORD wFramePairIdx;
            public WORD w06;
            public WORD wSomeIdx0;
            public WORD wSomeIdx1;
            public WORD wIdxInSecond;
            public WORD wFrameIdx;
            public WORD wSomeIdx3;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 18)]
            public WORD[] w32;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public char[] strFirmware;

            public DWORD dwF0;
            public DWORD dwF1;
            public DWORD dwF2;

            public BYTE btParameters;
            public BYTE bt2;

            public BYTE btCompass;
            public BYTE bt4;

            public WORD wSpeed;
            public BYTE btDoors;
            public BYTE bt5;

            public DWORD dwLongitude;
            public DWORD dwLatitude;

            public BYTE btVehicleKind;
            public BYTE bt6;//133
            public WORD wVehicleID;//134

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 14)]
            public WORD[] w33;
            public WORD dwHdrAndJpgSizeD6CLS;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 13)]
            public WORD[] w34;
            public BYTE btChannelD6CLS;
            public BYTE bt7;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 31)]
            public WORD[] w35;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 60)]
            public char[] strVehicleIDD6CLS;

            //////////////
            public BYTE bTurnRightSignal;
            public BYTE bTurnLeftSignal;
            public BYTE bReverse;
            public BYTE bSafetyAlarm;
            public BYTE bDimensionLights;
            public BYTE bBrake;
            public BYTE bSpeedInKMH;

            public BYTE btHeading;//:3
            public BYTE bLonWest;
            public BYTE bLatNorth;
        }

        enum DVR_WAV
        {
            DVR_WAV_LEN_max = 2560,
            DVR_WAV_LEN_min = 2048
        }

        public enum LoadStatus_t
        {
            lsOK,
            lsInterrupted,
            lsCanceled,
            lsFailed
        }

        public abstract class CMapLoader
        {
            private ulong m_ullFileOffsetBegin;
            private ulong m_ullFileOffsetEnd;
            private ulong m_ullFileSize;

            private byte[] m_pFilePart;
            private HANDLE m_hFileMapping;

            public ulong m_ullShift = 0;

            private bool m_bSilent;

            //////////
            private byte[] m_Buffer;
            private ulong m_ullCurrentTotalBytes;//размер принятых байтов
            private ulong m_ullIndexFrame;

            private byte[] m_pExcessDataBuffer;
            private int m_nExcessDataLength;

            private int m_nFilledBufferLength;//размер порции данных, над которым ведутся вычисления 

            private bool m_bFinalized;

            private List<CDvrDigestRecord> Digest;
            private List<DWORD> Tracks;

            private DWORD m_dwLastLongitude = 0;
            private DWORD m_dwLastLatitude = 0;

            DATE dt0;
            bool bTimeDefined;

            size_t cFrame;
            size_t cFirstFrameInTrack;
            size_t cLastFrameInTrack;

            float fCosY;

            WORD wPrevFrameIdx;

            private size_t cDeclaredFPS;
            private size_t cActualFPS;

            //////////

            public CMapLoader()
            {
                m_bSilent = false;
            }

            public void cout(string s)
            {
                Trace.WriteLine(s);
            }

            static public int _GetDvrFPS(CDvrFrameHdr _pFrameHdr)
            {
                switch (_pFrameHdr.wFramesRate)
                {
                    case 0x08: return 1;
                    case 0x10: return 2;
                    case 0x18: return 4;
                    case 0x20: return 4;
                    case 0x40: return 8;
                    case 0x30: return 8;
                    case 0x60: return 15;
                    case 0x80: return 12;
                    case 0xC8: return 25;
                    default: return -1;
                }
            }

            static float _ParseDvrPosition(DWORD _dw)
            {
                /*const*/
                WORD wHi = (WORD)(_dw >> 16);
                /*const*/
                WORD wLo = (WORD)(_dw & 0xFFFF);
                /*const*/
                int nDeg = (int)Math.Floor(wHi * 0.01f);
                int nMin = wHi - nDeg * 100;
                float fSec = wLo * 0.01f;

                return (float)(nDeg + (nMin + fSec / 60.0f) / 60.0f);
            }

            static public bool _ParseDvrHdr(CDvrFrameHdr _pFrameHdr, ref point_t _pt)
            {
                if (_pFrameHdr.dwLongitude == 0 || _pFrameHdr.dwLatitude == 0)
                    return false;

                _pt.X = _ParseDvrPosition(_pFrameHdr.dwLongitude);
                _pt.Y = _ParseDvrPosition(_pFrameHdr.dwLatitude);

                if (_pFrameHdr.bLonWest != 0)
                    _pt.X = -_pt.X;
                if (_pFrameHdr.bLatNorth == 0)
                    _pt.Y = -_pt.Y;

                return true;
            }

            static public void dt2date(int year, int month, int day, int hour, int min, int sec, ref DATE pOleDate)
            {
                if (pOleDate == null)
                {
                    return;
                }
                int[] MonthDays = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

                bool bLeapYear = ((year & 3) == 0) && ((year % 100) != 0 || (year % 400) == 0);

                long nDate = year * 365L + year / 4 - year / 100
                             + year / 400 + MonthDays[month - 1] + day;

                if (month <= 2 && bLeapYear)
                    --nDate;
                nDate -= 693959L;

                double dblTime = (hour * 3600L + min * 60L + sec) / 86400.0;

                pOleDate = (double)nDate + ((nDate >= 0) ? dblTime : -dblTime);
            }

            static public void _ParseDvrHdr(CDvrFrameHdr _pFrameHdr, ref trackpoint_info_t _tpi, bool _bAddMilliseconds)
            {
                DateTime datetime = new DateTime(
                    _pFrameHdr.btYear + 2000,
                    _pFrameHdr.btMonth,
                    _pFrameHdr.btDay,
                    _pFrameHdr.btHour,
                    _pFrameHdr.btMinute,
                    _pFrameHdr.btSecond);


                DATE dt = new DATE();//
                dt2date(_pFrameHdr.btYear + 2000,
                        _pFrameHdr.btMonth,
                        _pFrameHdr.btDay,
                        _pFrameHdr.btHour,
                        _pFrameHdr.btMinute,
                        _pFrameHdr.btSecond, ref dt);


                _tpi.wFlags |= (WORD)trackpoint_info_t.DefParam.defTime;
                _tpi.dtTime = dt;
                if (_bAddMilliseconds)
                {
                    _tpi.dtTime += _pFrameHdr.wIdxInSecond / (50.0F * 24 * 60 * 60);
                    _tpi.wFlags |= (WORD)trackpoint_info_t.DefParam.defTimeMilliseconds;
                }

                _tpi.wFlags |= (WORD)trackpoint_info_t.DefParam.defSpeed;
                _tpi.fSpeed = _pFrameHdr.bSpeedInKMH != 0 ? _pFrameHdr.wSpeed : _pFrameHdr.wSpeed * c_m_per_smile / 1000;

                _tpi.wFlags |= (WORD)trackpoint_info_t.DefParam.defHeading;
                int[] s_nHeadings = { 90, 270, 180, 0 };
                _tpi.fHeading = _pFrameHdr.btHeading == 0 || _pFrameHdr.btHeading > 4 ? 0 : s_nHeadings[_pFrameHdr.btHeading - 1];

            }

            protected void ParseFromBit(ref CDvrFrameHdr hdr)
            {
                System.Collections.BitArray bitData = new System.Collections.BitArray(
                    new byte[1] { Convert.ToByte(hdr.btParameters) });

                /*Упакованы в байте битами: 0SABCDEF
                  Действия водителей (0 – нет действия, 1 – есть действие):
                    F – TR (правый поворотник)
                    E – TL (левый поворотник)
                    D – BU (задний ход)
                    C – SA (аварийный сигнал)
                    B – AL (габаритные огни)
                    A – BR (нажатие на педаль тормоза)
                    S – размерность скорости
                    0 – мили/ч
                    1 – км/ч
                 */

                hdr.bTurnRightSignal = bitData[0] ? (BYTE)1 : (BYTE)0;
                hdr.bTurnLeftSignal  = bitData[1] ? (BYTE)1 : (BYTE)0;
                hdr.bReverse         = bitData[2] ? (BYTE)1 : (BYTE)0;
                hdr.bSafetyAlarm     = bitData[3] ? (BYTE)1 : (BYTE)0;
                hdr.bDimensionLights = bitData[4] ? (BYTE)1 : (BYTE)0;
                hdr.bBrake           = bitData[5] ? (BYTE)1 : (BYTE)0;
                hdr.bSpeedInKMH      = bitData[6] ? (BYTE)1 : (BYTE)0;

                bitData = new System.Collections.BitArray(
                    new byte[1] { Convert.ToByte(hdr.btCompass) });

                bool z1 = bitData.Get(2);
                bool z2 = bitData.Get(1);
                bool z3 = bitData.Get(0);

                hdr.btHeading = 0;//:3
                if (!z1 && !z2 && z3)
                    hdr.btHeading = 1;
                if (!z1 && z2 && !z3)
                    hdr.btHeading = 2;
                if (!z1 && z2 && z3)
                    hdr.btHeading = 3;
                if (z1 && !z2 && !z3)
                    hdr.btHeading = 4;

                hdr.bLonWest = bitData.Get(3) ? (BYTE)1 : (BYTE)0;
                hdr.bLatNorth = bitData.Get(4) ? (BYTE)1 : (BYTE)0;

            }

            // NOTE: _strFileName is supplied to get folder path, if this is required to locate some referred files.
            abstract protected LoadStatus_t Load(ref string _strFileName); //= 0;

            public void Init(HANDLE _hFileMapping, DWORD _dwFileSize)
            {
                m_pFilePart = null;
                m_hFileMapping = _hFileMapping;
                m_ullFileSize = _dwFileSize;
                m_ullFileOffsetBegin = 0;
                m_ullFileOffsetEnd = 0;
            }

            public CDvrFrameHdr InitMapFileByParts(HANDLE _hFileMapping, ulong _dwFileSize)
            {
                m_pFilePart = null;
                m_hFileMapping = _hFileMapping;
                m_ullFileSize = _dwFileSize;
                m_ullFileOffsetBegin = 0;
                m_ullFileOffsetEnd = 0;
                m_ullShift = 0;

                return GetFilePart(0, 1);
            }

            public void Init(ulong _FileSize)
            {
                m_bFinalized = false;

                m_ullFileSize = (_FileSize <= int.MaxValue) ? (_FileSize) : (_FileSize - 512);
                m_ullFileOffsetBegin = 0;
                m_ullFileOffsetEnd = 0;

                m_Buffer = null;
                m_ullCurrentTotalBytes = 0;
                m_ullIndexFrame = 0;

                m_nFilledBufferLength = 0;

                Digest = new List<CDvrDigestRecord>();
                Tracks = new List<DWORD>();

                dt0 = 0;
                bTimeDefined = false;

                cFrame = 0;

                cFrame = 0;
                cFirstFrameInTrack = 0;
                cLastFrameInTrack = 0;

                fCosY = 1;

                cDeclaredFPS = 0;
                cActualFPS = 0;

                m_Buffer = new byte[c_nMaxBufferSize];
                m_pExcessDataBuffer = new byte[c_nMaxBufferSize];
            }

            public void CreateDigest(string strDigestName)
            {
                using (IO.BinaryWriter hDigest = new IO.BinaryWriter(IO.File.Open(strDigestName, IO.FileMode.Create)))
                {
                    hDigest.Write(dt0);
                    hDigest.Write((DWORD)cActualFPS);
                    hDigest.Write((DWORD)Tracks.Count);
                    if (Tracks.Count > 0)
                    {
                        foreach (DWORD tr in Tracks)
                        {
                            hDigest.Write(tr);
                        } 
                    }
                    hDigest.Write((DWORD)Digest.Count);
                    if (Digest.Count > 0)
                    {
                        foreach (CDvrDigestRecord rec in Digest)
                        {
                            hDigest.Write(rec.ullFrameHdrOffset);
                            hDigest.Write(rec.x);
                            hDigest.Write(rec.y);
                            hDigest.Write(rec.dtTime);
                            hDigest.Write(rec.fSpeed);
                            hDigest.Write(rec.dwReserved0);
                            hDigest.Write(rec.dwReserved1);
                        }
                    }

                    hDigest.Flush();
                }//using
            }

            protected ulong GetFileSize()
            {
                return m_ullFileSize;
            }

            protected void PushNewDataToBuffer(byte[] _arrNewData)
            {
                bool bBufferFull = false;
                if (_arrNewData == null || m_Buffer == null)
                    return;

                try
                {
                    if (m_nFilledBufferLength + _arrNewData.Length > c_nMaxBufferSize)
                    {
                        m_nExcessDataLength = m_nFilledBufferLength + _arrNewData.Length - c_nMaxBufferSize;
                        Array.Copy(_arrNewData, 0, m_Buffer, m_nFilledBufferLength, _arrNewData.Length - m_nExcessDataLength);
                        Array.Copy(_arrNewData, _arrNewData.Length - m_nExcessDataLength, m_pExcessDataBuffer, 0, m_nExcessDataLength);
                        m_nFilledBufferLength = c_nMaxBufferSize;
                        bBufferFull = true;
                    }
                    else
                    {
                        Array.Copy(_arrNewData, 0, m_Buffer, m_nFilledBufferLength, _arrNewData.Length);
                        m_nFilledBufferLength += _arrNewData.Length;
                    }

                    m_ullCurrentTotalBytes += (ulong)_arrNewData.Length;

                    if (bBufferFull || m_ullCurrentTotalBytes >= m_ullFileSize)
                        CommitBuffer(false);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
            }

            private void CommitBuffer(bool _bLastCommit)
            {
                m_ullIndexFrame++;
                ulong ulLastFoundHeaderOffset = 0;
                ulong ulNextTotalProcessedLength = m_ullCurrentTotalBytes - (ulong)m_nExcessDataLength;
                ulong ulCurrentTotalProcessedLength = ulNextTotalProcessedLength - (ulong)m_nFilledBufferLength;

                while (m_ullFileOffsetBegin < ulNextTotalProcessedLength)
                {
                    //*********************************************
                    CDvrFrameHdr pFrameHdr = null;
                    bool bScanned = false;
                    while (m_ullFileOffsetBegin < m_ullCurrentTotalBytes)
                    {
                        pFrameHdr = GetFrame(m_ullFileOffsetBegin - ulCurrentTotalProcessedLength, 0);

                        if (pFrameHdr == null)
                            return;

                        if (pFrameHdr.dwStartMarker == c_dwDvrFrameMarker)
                        {
                            ulLastFoundHeaderOffset = m_ullFileOffsetBegin;
                            break;
                        }

                        if (!bScanned)
                        {
                            bScanned = true;
                            Trace.WriteLine(String.Format("Incorrect frame at offset {0}.{1}.", (DWORD)(m_ullFileOffsetBegin >> 32), (DWORD)m_ullFileOffsetBegin));
                        }

                        m_ullFileOffsetBegin += 256;
                    }
                    //
                    //*********************************************

                    int nUnprocessedPartOfBufferLen = (int)(ulNextTotalProcessedLength - ulLastFoundHeaderOffset);

                    // 
                    // Если размер заголовка(512) больше оставшейся части буффера
                    // Тогда сохраняем часть буффера для следующей итерации

                    if (nUnprocessedPartOfBufferLen >= 512)
                    {
                        ulong next_m_ullFileOffsetBegin = 0;
                        CalcFrame(pFrameHdr, m_ullFileOffsetBegin, ref next_m_ullFileOffsetBegin, bScanned);
                        m_ullFileOffsetBegin = next_m_ullFileOffsetBegin;
                    }
                    else if (!_bLastCommit)
                    {
                        //Копируем nUnprocessedPartOfBufferLen байт из конца буфера в начало буфера с избыточными данными
                        if (m_nExcessDataLength > 0)
                        {
                            // Сдвиг избыточных данных на nUnprocessedPartOfBufferLen
                            Array.Copy(
                                m_pExcessDataBuffer.Take(m_nExcessDataLength).ToArray(), 0,
                                m_pExcessDataBuffer, nUnprocessedPartOfBufferLen, m_nExcessDataLength
                            );
                        }
                        Array.Copy(m_Buffer, c_nMaxBufferSize - nUnprocessedPartOfBufferLen,
                            m_pExcessDataBuffer, 0, nUnprocessedPartOfBufferLen);

                        m_nExcessDataLength += nUnprocessedPartOfBufferLen;

                        break;
                    }
                    else // _bLastCommit, unprocessed tail shorter than 512 B
                        throw new ApplicationException("Unprocessed buffer size < 512 on last buffer commit");
                }

                m_nFilledBufferLength = 0;
                if (m_nExcessDataLength != 0)
                {
                    m_nFilledBufferLength = m_nExcessDataLength;
                    Array.Copy(m_pExcessDataBuffer, 0, m_Buffer, 0, m_nExcessDataLength);

                    m_nExcessDataLength = 0;
                }
            } 

            public void FinilizeDigest(string _strDigestName)
            {
                if (!m_bFinalized)
                {
                    CommitBuffer(true);

                    size_t cDigestRecords = (size_t)Digest.Count;
                    if (cDigestRecords != 0 && (Tracks.Count == 0 || (size_t)Tracks.Last() != cDigestRecords))
                        Tracks.Add((DWORD)cDigestRecords);

                    if (cActualFPS == 0 && cDigestRecords != 0)
                    {
                        DATE dtStart = Digest[0].dtTime;//const
                        DATE dtEnd = Digest[(int)(cDigestRecords - 1)].dtTime;//const
                        double fDurationInS = (dtEnd - dtStart) * 24 * 60 * 60;//const
                        if (fDurationInS > 0)
                            cActualFPS = (size_t)Math.Floor((cLastFrameInTrack - cFirstFrameInTrack) / fDurationInS + 0.5f);
                    }

                    m_bFinalized = true;
                }

                CreateDigest(_strDigestName);
            }
 
            protected CDvrFrameHdr GetFrame(ulong _begin, ulong _end)
            {
                IntPtr buff = Marshal.UnsafeAddrOfPinnedArrayElement(m_Buffer, (int)_begin);
                CDvrFrameHdr _Frame = (CDvrFrameHdr)Marshal.PtrToStructure(buff, typeof(CDvrFrameHdr));
                ParseFromBit(ref _Frame);
                return _Frame;
            }

            protected DWORD GetVehicleID()
            {
                DWORD dwVehicleID = 0;
                CDvrFrameHdr pFrameHdr = null;
                ulong ullOffset = 0;
                while (ullOffset < (ulong)m_nFilledBufferLength)
                {
                    ulong ullHdrEndOffset = Math.Min(ullOffset + 512, (ulong)m_nFilledBufferLength);
                    pFrameHdr = GetFrame(ullOffset, ullHdrEndOffset);

                    if (pFrameHdr == null)
                    {
                        return 0;
                    }
                    if (pFrameHdr.dwStartMarker == c_dwDvrFrameMarker)
                    {
                        break;
                    }

                    ullOffset += 256;
                }

                if (pFrameHdr != null)
                {
                    string strFirmware = new string(pFrameHdr.strFirmware);
                    if (strFirmware.Equals("D6CLS"))
                    {
                        string VehicleIDD6CLS = new string(pFrameHdr.strVehicleIDD6CLS.Skip(2).Where(Char.IsDigit).ToArray());
                        dwVehicleID = (DWORD)DWORD.Parse(VehicleIDD6CLS);
                    }
                    else
                    {
                        dwVehicleID = pFrameHdr.wVehicleID;
                    }
                }

                return dwVehicleID;
            }

            protected void CalcFrame(CDvrFrameHdr pFrameHdr, ulong ullOffset, ref ulong next_ullOffset, bool bScanned)
            {
                // Scan for next frame.
                ullOffset &= 0xFFFFFFFFFFFFFF00;


                if (!bTimeDefined)
                {
                    // Save start time.
                    trackpoint_info_t point_extra = new trackpoint_info_t();
                    _ParseDvrHdr(pFrameHdr, ref point_extra, false);

                    dt0 = point_extra.dtTime;
                    bTimeDefined = true;
                }

                bool bNewTrack = pFrameHdr.wFrameIdx == 1 && wPrevFrameIdx != 0;
                wPrevFrameIdx = pFrameHdr.wFrameIdx;

                // Mark next track, if any.
                if (bNewTrack || bScanned)
                {
                    size_t cDigestRecords = (size_t)Digest.Count;
                    if (cDigestRecords != 0 && (Tracks.Count == 0 || Tracks.Last() != cDigestRecords))
                        Tracks.Add((DWORD)cDigestRecords);

                    if (cActualFPS == 0 && cDigestRecords != 0)
                    {
                        DATE dtStart = Digest[0].dtTime;
                        DATE dtEnd = Digest[(int)(cDigestRecords - 1)].dtTime;
                        double fDurationInS = (dtEnd - dtStart) * 24 * 60 * 60;
                        if (fDurationInS > 0)
                            cActualFPS = (size_t)Math.Floor((cLastFrameInTrack - cFirstFrameInTrack) / fDurationInS + 0.5f);
                    }
                }

                if (cDeclaredFPS == 0)
                    cDeclaredFPS = (size_t)_GetDvrFPS(pFrameHdr);


                point_t pt = new point_t();
                if (_ParseDvrHdr(pFrameHdr, ref pt))
                {
                    if (Digest.Count == 0)
                        fCosY = (float)Math.Cos(pt.Y * c_PI / 180);

                    if (Digest.Count == 0 || pFrameHdr.dwLongitude != m_dwLastLongitude || pFrameHdr.dwLatitude != m_dwLastLatitude)
                    {
                        trackpoint_info_t point_extra = new trackpoint_info_t();
                        _ParseDvrHdr(pFrameHdr, ref point_extra, false);

                        bool bPosOK = true;
                        if (Digest.Count > 0)
                        {
                            // Check for jumping to incredibly large distance.
                            CDvrDigestRecord dr_prev = Digest.Last();
                            float fDiffInS = (float)Math.Abs(point_extra.dtTime - dr_prev.dtTime) * 24 * 60 * 60;
                            if (fDiffInS == 0)
                                fDiffInS = 1;
                            float d = Math.Abs(pt.X - dr_prev.x) * fCosY + Math.Abs(pt.Y - dr_prev.y);
                            float fDInM = (float)(d * 40e6 / 360.0F);
                            float fSpeed = fDInM / fDiffInS;

                            if (fSpeed > 1000)
                                bPosOK = false;
                        }

                        if (bPosOK)
                        {
                            if (cActualFPS == 0 && Digest.Count > 0)
                            {
                                float fDiffInS = (float)Math.Abs(point_extra.dtTime - Digest.Last().dtTime) * 24 * 60 * 60;
                                if (fDiffInS > 3)
                                {
                                    DATE dtStart = Digest[0].dtTime;
                                    DATE dtEnd = Digest[Digest.Count - 1].dtTime;
                                    double fDurationInS = (dtEnd - dtStart) * 24 * 60 * 60;
                                    if (fDurationInS > 0)
                                        cActualFPS = (size_t)Math.Floor((cLastFrameInTrack - cFirstFrameInTrack) / fDurationInS + 0.5F);
                                }
                            }

                            if (Digest.Count == 0)
                                cFirstFrameInTrack = cFrame;
                            cLastFrameInTrack = cFrame;

                            m_dwLastLongitude = pFrameHdr.dwLongitude;
                            m_dwLastLatitude = pFrameHdr.dwLatitude;

                            CDvrDigestRecord dr = new CDvrDigestRecord();
                            dr.ullFrameHdrOffset = ullOffset;
                            dr.x = pt.X;
                            dr.y = pt.Y;
                            dr.dtTime = point_extra.dtTime;
                            dr.fSpeed = point_extra.fSpeed;
                            Digest.Add(dr);
                        }
                    }
                }

                ulong cFrameSize = (ulong)pFrameHdr.wFrameSizeBy256 * 512;//const
                //assert (cFrameSize < 65536);
                ullOffset += cFrameSize;
                next_ullOffset = ullOffset;

                ++cFrame;
            }


            protected CDvrFrameHdr GetFilePart(ulong _ullBeginOffset, ulong _ullEndOffset)
            {
                DateTime dold = DateTime.Now;

                ulong ullFileSize = GetFileSize();

                //assert (_ullBeginOffset < _ullEndOffset);
                //assert (_ullEndOffset <= ullFileSize);

                if (m_ullFileOffsetBegin > _ullBeginOffset || _ullEndOffset > m_ullFileOffsetEnd)
                {
                    //assert (m_hFileMapping != NULL);

                    if (m_pFilePart != null)
                    {
                        m_pFilePart = null;
                    }

                    uint dwAllocationGranularity = 1;// si.dwAllocationGranularity;

                    m_ullFileOffsetBegin = ((ulong)_ullBeginOffset / dwAllocationGranularity) * dwAllocationGranularity;

                    m_ullFileOffsetEnd = _ullBeginOffset + Math.Max((ulong)(_ullEndOffset - _ullBeginOffset), (ulong)c_nMaxBufferSize);

                    if (m_ullFileOffsetEnd > ullFileSize)
                        m_ullFileOffsetEnd = ullFileSize;

                    //

                    m_ullShift = 0;
                    m_pFilePart = new byte[m_ullFileOffsetEnd - m_ullFileOffsetBegin];

                    m_hFileMapping.Seek((long)m_ullFileOffsetBegin, IO.SeekOrigin.Begin);

                    if (m_pFilePart == null)
                    {
                        Trace.WriteLine("Can't read file. The file probably does not fit virtual memory. (Error code = %d.)");
                        return null;
                    }
                }

                TimeSpan sp = DateTime.Now - dold;

                m_ullShift = _ullBeginOffset - m_ullFileOffsetBegin;

                dold = DateTime.Now;
                IntPtr buff = Marshal.UnsafeAddrOfPinnedArrayElement(m_pFilePart, (int)m_ullShift);//shiftPart, 0);
                CDvrFrameHdr result_data = (CDvrFrameHdr)Marshal.PtrToStructure(buff, typeof(CDvrFrameHdr));

                ///////////////////////////////////////////////////////////////
                ParseFromBit(ref result_data);
                ///////////////////////////////////////////////////////////////

                return result_data;
            }

            protected bool Silent()
            {
                return m_bSilent;
            }
        }

        public class DigestWriter : CMapLoader
        {
            private bool g_bStop = false;

            public bool CreateDigest(string fileName, string fileNameDigest)
            {
                if (!IO.File.Exists(fileName))
                    return false;

                IO.FileStream fDvr = new IO.FileStream(fileName, IO.FileMode.Open, IO.FileAccess.Read);


                if (fDvr.CanRead == true) // если можно производить поиск
                {
                    ulong Len = (ulong)fDvr.Length;

                    InitMapFileByParts(fDvr, Len);
                    string sDigest = fileNameDigest;
                    LoadStatus_t status = Load(ref sDigest);
                    return true;
                }
                else
                    return false;

            }

            public void PushChunk(byte[] bytes, int size)
            {
                //System.Drawing.PointF p = new System.Drawing.PointF(1, 1);

                if (bytes == null || bytes.Length == 0 || size == 0)
                    return;

                if (size < bytes.Length)
                    PushNewDataToBuffer(bytes.Take(size).ToArray());
                else
                    PushNewDataToBuffer(bytes);
            }

            protected override LoadStatus_t Load(ref string _strFileName)//, map_t * _pMap);
            {

                ulong ullFileSize = GetFileSize();

                // Report vehicle ID.
                DWORD dwVehicleID = 0;
                {
                    CDvrFrameHdr pFrameHdr = null;
                    ulong ullOffset = 0;
                    while (ullOffset < ullFileSize)
                    {
                        ulong ullHdrEndOffset = Math.Min(ullOffset + 512, ullFileSize);
                        pFrameHdr = (CDvrFrameHdr)(GetFilePart(ullOffset, ullHdrEndOffset));

                        if (pFrameHdr == null)
                        {
                            return LoadStatus_t.lsFailed;
                        }
                        if (pFrameHdr.dwStartMarker == c_dwDvrFrameMarker)
                        {
                            break;
                        }

                        ullOffset += 256;
                    }

                    if (pFrameHdr != null)
                    {
                        string strFirmware = new string(pFrameHdr.strFirmware);
                        if (strFirmware.Equals("D6CLS"))
                        {
                            string VehicleIDD6CLS = new string(pFrameHdr.strVehicleIDD6CLS.Skip(2).Where(Char.IsDigit).ToArray());
                            dwVehicleID = (DWORD)DWORD.Parse(VehicleIDD6CLS);
                        }
                        else
                        {
                            dwVehicleID = pFrameHdr.wVehicleID;
                        }
                    }
                }

                // Get time zone.
                //DateTime tzi;
                //TIME_ZONE_INFORMATION tzi;
                //if (::GetTimeZoneInformation (& tzi) == TIME_ZONE_ID_DAYLIGHT)
                //        tzi.Bias += tzi.DaylightBias;

                size_t cDeclaredFPS = 0;
                size_t cActualFPS = 0;

                List<CDvrDigestRecord> Digest = new List<CDvrDigestRecord>();
                List<DWORD> Tracks = new List<DWORD>();

                // Prepare name of the digest.
                string strDigestName = String.Format("{0}.digest", _strFileName);

                bool bCreateDigest = true;
                //Если файл существует

                if (IO.File.Exists(_strFileName))
                {
                    // Read the digest file.
                    /*DATE dt0 = 0;
                    */
                }

                if (bCreateDigest)
                {
                    // Parse original DVR file.

                    DATE dt0 = 0;
                    bool bTimeDefined = false;

                    size_t cFrame = 0;
                    size_t cFirstFrameInTrack = 0;
                    size_t cLastFrameInTrack = 0;

                    float fCosY = 1;

                    WORD wPrevFrameIdx = 0;

                    float sped = 13.0F;

                    ulong ullOffset = 0;

                    do
                    {
                        if (g_bStop)
                            return LoadStatus_t.lsInterrupted;

                        // Scan for next frame.
                        CDvrFrameHdr pFrameHdr = null;
                        ullOffset &= 0xFFFFFFFFFFFFFF00;

                        bool bScanned = false;

                        while (ullOffset < ullFileSize)
                        {
                            ulong ullHdrEndOffset = Math.Min(ullOffset + 512, ullFileSize);
                            pFrameHdr = (CDvrFrameHdr)(GetFilePart(ullOffset, ullHdrEndOffset));

                            if (pFrameHdr == null)
                                return LoadStatus_t.lsFailed;

                            if (pFrameHdr.dwStartMarker == null)
                            {
                                int sss = 1;
                            }
                            if (pFrameHdr.dwStartMarker == c_dwDvrFrameMarker)
                            {
                                //add_to_file(ref file_fff, pFrameHdr, 0, 0);
                                break;
                            }

                            if (!bScanned)
                            {
                                bScanned = true;
                                //ReportWarning (_T ("Incorrect frame at offset %08x.%08x."), DWORD (ullOffset >> 32), (DWORD) ullOffset);
                                cout(String.Format("Incorrect frame at offset {0}.{1}.", (DWORD)(ullOffset >> 32), (DWORD)ullOffset));
                            }

                            ullOffset += 256;
                        }
                        //cout("ullOffset=" + ullOffset.ToString() + " bLonWest=" + pFrameHdr.bLonWest.ToString() + " CLS=" + new string(pFrameHdr.strFirmware) + 

                        if (ullOffset >= ullFileSize)
                            break;

                        if (!bTimeDefined)
                        {
                            // Save start time.
                            trackpoint_info_t point_extra = new trackpoint_info_t();
                            _ParseDvrHdr(pFrameHdr, ref point_extra, false);

                            dt0 = point_extra.dtTime;
                            bTimeDefined = true;
                        }

                        bool bNewTrack = pFrameHdr.wFrameIdx == 1 && wPrevFrameIdx != 0;
                        wPrevFrameIdx = pFrameHdr.wFrameIdx;

                        // Mark next track, if any.
                        if (bNewTrack || bScanned)
                        {
                            size_t cDigestRecords = (size_t)Digest.Count;
                            if (cDigestRecords != 0 && (Tracks.Count == 0 || Tracks.Last() != cDigestRecords))
                                Tracks.Add((DWORD)cDigestRecords);

                            if (cActualFPS == 0 && cDigestRecords != 0)
                            {
                                DATE dtStart = Digest[0].dtTime;
                                DATE dtEnd = Digest[(int)(cDigestRecords - 1)].dtTime;
                                double fDurationInS = (dtEnd - dtStart) * 24 * 60 * 60;
                                if (fDurationInS > 0)
                                    cActualFPS = (size_t)Math.Floor((cLastFrameInTrack - cFirstFrameInTrack) / fDurationInS + 0.5f);
                            }
                        }

                        if (cDeclaredFPS == 0)
                            cDeclaredFPS = (size_t)_GetDvrFPS(pFrameHdr);


                        point_t pt = new point_t();
                        if (_ParseDvrHdr(pFrameHdr, ref pt))
                        {
                            if (Digest.Count == 0)
                                fCosY = (float)Math.Cos(pt.Y * c_PI / 180);

                            if (Digest.Count == 0 || pt.X != Digest.Last().x || pt.Y != Digest.Last().y)
                            {
                                trackpoint_info_t point_extra = new trackpoint_info_t();
                                _ParseDvrHdr(pFrameHdr, ref point_extra, false);

                                bool bPosOK = true;
                                if (Digest.Count > 0)
                                {
                                    // Check for jumping to incredibly large distance.
                                    CDvrDigestRecord dr_prev = Digest.Last();
                                    float fDiffInS = (float)Math.Abs(point_extra.dtTime - dr_prev.dtTime) * 24 * 60 * 60;
                                    if (fDiffInS == 0)
                                        fDiffInS = 1;
                                    float d = Math.Abs(pt.X - dr_prev.x) * fCosY + Math.Abs(pt.Y - dr_prev.y);
                                    float fDInM = (float)(d * 40e6 / 360.0F);
                                    float fSpeed = fDInM / fDiffInS;
                                    sped = fSpeed;
                                    if (fSpeed > 1000)
                                        bPosOK = false;
                                }

                                if (bPosOK)
                                {
                                    if (cActualFPS == 0 && Digest.Count > 0)
                                    {
                                        float fDiffInS = (float)Math.Abs(point_extra.dtTime - Digest.Last().dtTime) * 24 * 60 * 60;
                                        if (fDiffInS > 3)
                                        {
                                            DATE dtStart = Digest[0].dtTime;
                                            DATE dtEnd = Digest[Digest.Count - 1].dtTime;
                                            double fDurationInS = (dtEnd - dtStart) * 24 * 60 * 60;
                                            if (fDurationInS > 0)
                                                cActualFPS = (size_t)Math.Floor((cLastFrameInTrack - cFirstFrameInTrack) / fDurationInS + 0.5F);
                                        }
                                    }

                                    if (Digest.Count == 0)
                                        cFirstFrameInTrack = cFrame;
                                    cLastFrameInTrack = cFrame;

                                    //Digest.Add(new CDvrDigestRecord());
                                    CDvrDigestRecord dr = new CDvrDigestRecord();//Digest.Last();
                                    //::memset (& dr, 0, sizeof (CDvrDigestRecord));
                                    dr.ullFrameHdrOffset = ullOffset;
                                    dr.x = pt.X;
                                    dr.y = pt.Y;
                                    dr.dtTime = point_extra.dtTime;
                                    dr.fSpeed = point_extra.fSpeed;
                                    Digest.Add(dr);
                                }
                            }
                        }

                        ulong cFrameSize = (ulong)pFrameHdr.wFrameSizeBy256 * 512;//const
                        if (cFrameSize >= 65536)
                        {
                            //
                        }
                        //assert (cFrameSize < 65536);
                        ullOffset += cFrameSize;

                        ++cFrame;

                        //SetProgressStatus (50*ullOffset/ullFileSize);
                    } while (ullOffset < ullFileSize);


                    // Complete last track.
                    {
                        size_t cDigestRecords = (size_t)Digest.Count;
                        if (cDigestRecords != 0 && (Tracks.Count == 0 || (size_t)Tracks.Last() != cDigestRecords))
                            Tracks.Add((DWORD)cDigestRecords);

                        if (cActualFPS == 0 && cDigestRecords != 0)
                        {
                            DATE dtStart = Digest[0].dtTime;//const
                            DATE dtEnd = Digest[(int)(cDigestRecords - 1)].dtTime;//const
                            double fDurationInS = (dtEnd - dtStart) * 24 * 60 * 60;//const
                            if (fDurationInS > 0)
                                cActualFPS = (size_t)Math.Floor((cLastFrameInTrack - cFirstFrameInTrack) / fDurationInS + 0.5f);
                        }
                    }

                    //
                    // Create the digest file.
                    //
                    using (IO.BinaryWriter hDigest = new IO.BinaryWriter(IO.File.Open(strDigestName, IO.FileMode.Create)))
                    {
                        hDigest.Write(dt0);
                        hDigest.Write((DWORD)cActualFPS);
                        hDigest.Write((DWORD)Tracks.Count);
                        if (Tracks.Count > 0)
                        {
                            foreach (DWORD tr in Tracks)
                                hDigest.Write(tr);
                        }
                        hDigest.Write((DWORD)Digest.Count);
                        if (Digest.Count > 0)
                        {
                            foreach (CDvrDigestRecord rec in Digest)
                            {
                                hDigest.Write(rec.ullFrameHdrOffset);
                                hDigest.Write(rec.x);
                                hDigest.Write(rec.y);
                                hDigest.Write(rec.dtTime);
                                hDigest.Write(rec.fSpeed);
                                hDigest.Write(rec.dwReserved0);
                                hDigest.Write(rec.dwReserved1);
                            }
                        }
                    }//using
                }

                return LoadStatus_t.lsOK;
            }
        }
    }
}
