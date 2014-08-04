using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using GDStorage.Viewer.GDOStorageServiceReference;
using GisSharpBlog.NetTopologySuite.Geometries;
using GDStorage.FileUtils;
using System.Threading;
using GDStorage.SharedUI;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.Windows.Input;
using GeoAPI.Geometries;
using Geom = GDStorage.Viewer.GeomDataClient;

using GisSharpBlog.NetTopologySuite;

namespace GDStorage.Viewer
{
    [ComVisible(false)]
    public class RenderStatusChangedEventArgs : EventArgs
    {
        public RenderStatusChangedEventArgs(RenderStatusEnum newStatus)
        {
            NewRenderStatus = newStatus;
        }

        public RenderStatusEnum NewRenderStatus { get; set; }
    }

    [ComVisible(false)]
    public class GDODataModel
    {
        public Credentials Credentials { get; set; }
        public Credentials RefServerCredentials { get; set; }
        public string RefServerIP { get; set; }
        public bool RefServerLogonOK { get; set; }
        public bool IsServerHttp { get; set; }

        static DateTime dtLastUploadStart = DateTime.MinValue;
        static DateTime dtLastAttemptedUpload = DateTime.MinValue;
        double m_y0, m_x0, m_y1, m_x1;
                
        protected const int Accuracy = 4; // 4px;
        Dictionary<Guid, GDO> m_pCache = new Dictionary<Guid, GDO>();

        List<GDO> m_pData = new List<GDO>();
        public Settings Settings { get; set; }
        public string[] DisplayedRemoteServers { get; set; }

        private readonly GPSMapEdit.IViewPluginHost m_pHost;

        public event EventHandler RenderStatusChanged;

        private GeomDataClient m_GeomClient = null;
        private Object lockGeom = new Object();

        public GeomDataClient GeomDataClient { get { return m_GeomClient; } }

        private GDStorage.Viewer.GDOStorageServiceReference.FileType convertToFileType(ProtocolMessages.FileType type)
        {
            switch (type)
            {
                case ProtocolMessages.FileType.VIDEOTRACK:
                    return FileType.VideoTrack;
                case ProtocolMessages.FileType.NOTE:
                    return FileType.RoadNotes;
                case ProtocolMessages.FileType.TRACK:
                    return FileType.Other;
                case ProtocolMessages.FileType.FILTER:
                    return FileType.Filter;
                default: { return FileType.RoadNotes; }
            }
        }

        private List<ProtocolMessages.FileType> convertToFileType(GDStorage.Viewer.GDOStorageServiceReference.FileType[] type)//? type)
        {
            List<ProtocolMessages.FileType> convFileType = new List<ProtocolMessages.FileType>();

            foreach (var item in type)
            {
                switch (item)
                {
                    case GDStorage.Viewer.GDOStorageServiceReference.FileType.VideoTrack:
                        convFileType.Add(ProtocolMessages.FileType.VIDEOTRACK);
                        break;
                    case GDStorage.Viewer.GDOStorageServiceReference.FileType.RoadNotes:
                        convFileType.Add(ProtocolMessages.FileType.NOTE);
                        break;
                    case GDStorage.Viewer.GDOStorageServiceReference.FileType.Other:
                        convFileType.Add(ProtocolMessages.FileType.TRACK);
                        break;
                    case GDStorage.Viewer.GDOStorageServiceReference.FileType.Filter:
                        convFileType.Add(ProtocolMessages.FileType.FILTER);
                        break;
                    default: { continue; }
                }
            }

            return convFileType;
        }

        private GDStorage.Viewer.GDOStorageServiceReference.MediaFileStatus convertToMediaFileStatus(ProtocolMessages.FileStatus type)
        {
            switch (type)
            {
                case ProtocolMessages.FileStatus.GEOM_UPLOADED:
                    return MediaFileStatus.GeomUploaded;
                case ProtocolMessages.FileStatus.LOCAL:
                    return MediaFileStatus.Local;
                default: { return MediaFileStatus.Local; } 
            }
        }

        private void CreateGeom()
        {
            m_GeomClient = new GeomDataClient();
        }

        void OnRenderStatusChanged(RenderStatusEnum newStatus)
        {
            if (RenderStatusChanged != null)
                RenderStatusChanged(this, new RenderStatusChangedEventArgs(newStatus));
        }

        public GDODataModel(GPSMapEdit.IViewPluginHost pHost)
        {
            m_pHost = pHost;
            Settings = new Settings 
            { 
                RenderStatus = RenderStatusEnum.Ready, 
                ShowVideoTracks = true,
                ShowFilters = false,
                TrackMinDate = Plugin.Instance.GetDefaultTrackMinDate(),
                TrackMaxDate = System.DateTime.Today
            };
            DisplayedRemoteServers = new string[] { };
            
            CreateGeom();
        }

        public bool ViewParametersChanged(double _y0, double _x0, double _y1, double _x1, Settings settings)
        {
            lock (this)
            {
                return 
                    m_x0 != _x0 || m_x1 != _x1 || m_y0 != _y0 || m_y1 != _y1 || 
                    Settings == null ||
                    Settings.ShowVideoTracks != settings.ShowVideoTracks || 
                    Settings.ShowTracks != settings.ShowTracks || 
                    Settings.ShowRoadNotes != settings.ShowRoadNotes || 
                    Settings.ShowBackups != settings.ShowBackups ||
                    Settings.ShowRemoteData != settings.ShowRemoteData;
            }
        }

        public bool RemoteDisplayParametersChanged(string[] _newDisplayedServers)
        {
            return DisplayedRemoteServers.SequenceEqual(_newDisplayedServers);
        } 

        private bool CheckBreakLoadCondition(double _y0, double _x0, double _y1, double _x1, Settings settings)
        {
            return ViewParametersChanged(_y0, _x0, _y1, _x1, settings);
        }

        private GDOStorageServiceClient GetDistributedServerProxy()
        {
            GDOStorageServiceClient retProxy = null;

            using (GDOStorageServiceClient primaryProxy = GDOStorageServiceClient.Create (Credentials))
            {
                Credentials retCre = Credentials;
                retCre.Server = primaryProxy.GetVacantServer();
                retProxy = GDOStorageServiceClient.Create (retCre);
            }

            return retProxy;
        }

        private void TerminateLoadExec(Object arg) 
        {
            try
            {
                Object[] args = (Object[])arg;
                Credentials credentials = (Credentials)args[0];
                Guid sessionID = new Guid (args[1].ToString());

                using (GDOStorageServiceClient proxy = GDOStorageServiceClient.Create(Credentials))
                {
                    proxy.TerminateQuery(sessionID);
                }
            }
            catch (Exception e)
            {
                int aaa = 1;
            } 
        }

        private Guid[] GetGeomObjectsIDs(object arg)
        {
            try
            {
                Object[] args = (Object[])arg;
                double _y0 = (double)args[0];
                double _x0 = (double)args[1];
                double _y1 = (double)args[2];
                double _x1 = (double)args[3];
                List<ProtocolMessages.FileType> fileType = (List<ProtocolMessages.FileType>)args[4];

                bool isBackup = ((bool)args[5]);
                DateTime? fromDateTime = (DateTime?)args[6];
                DateTime? toDateTime = (DateTime?)args[7];
                UInt64 dateFrom = fromDateTime.HasValue ? (UInt64)(fromDateTime.Value - new DateTime(1970, 1, 1)).TotalSeconds : 0;
                UInt64 dateTo = toDateTime.HasValue ? (UInt64)(toDateTime.Value - new DateTime(1970, 1, 1)).TotalSeconds : 0;
                ProtocolMessages.ClientRequest.DisplayedGeometryFilter.DateInterval requiredDate = null;
                if (dateFrom != 0 && dateTo != 0)
                {
                    requiredDate = new ProtocolMessages.ClientRequest.DisplayedGeometryFilter.DateInterval()
                    {
                        dateFrom = (Int64)dateFrom,
                        dateTo = (Int64)dateTo
                    };
                }
                
                ProtocolMessages.ClientRequest.DisplayedGeometryFilter obj = new ProtocolMessages.ClientRequest.DisplayedGeometryFilter()
                {
                    levelBits = (int)m_pHost.Scale,
                    boundRect = new ProtocolMessages.Rect()
                    {
                        x0 = _x0,
                        y0 = _y0,
                        x1 = _x1,
                        y1 = _y1
                    },
                    isBackup = isBackup,
                    fileStatus = GDStorage.Viewer.ProtocolMessages.FileStatus.NONE,//LOCAL,
                    requiredDate = requiredDate,
                };
                
                foreach (var itemType in fileType)
                    obj.fileType.Add(itemType);

                ProtocolMessages.ClientRequest objSend = new ProtocolMessages.ClientRequest()
                {
                    filter = obj,
                    messageType = ProtocolMessages.ClientRequest.MessageType.DATA_FILTER
                };
                
                ProtocolMessages.DisplayedGeometriesInfo resObj = m_GeomClient.Query(objSend, Credentials) as ProtocolMessages.DisplayedGeometriesInfo;
                Guid[] guids = new Guid[resObj.displayedTrackInfo.Count()];
                for (int g = 0; g < resObj.displayedTrackInfo.Count(); g++)
                    guids[g] = new Guid(resObj.displayedTrackInfo[g].geometryID);
                return guids;
            }
            catch (Exception ex)
            {
                return new Guid[] { }; 
            }
        }

        private GDO[] GetGeomObjectsByIDs(Guid SessionID, Guid[] guids)
        {
            ProtocolMessages.ClientRequest.GeometriesRequest geomRequest = new ProtocolMessages.ClientRequest.GeometriesRequest()
            {
                geometryFormat = ProtocolMessages.ClientRequest.GeometriesRequest.GeometryFormat.BINARY,
            };
            foreach (var g in guids)
                geomRequest.geometryID.Add(g.ToString());
            
            ProtocolMessages.ClientRequest objSend = new ProtocolMessages.ClientRequest()
            {
                geometryRequest = geomRequest,
                messageType = ProtocolMessages.ClientRequest.MessageType.GEOMETRIES
            };

            List<GDO> gdo = new List<GDO>();
            try
            {
                List<ProtocolMessages.DisplayedGeometry> resObj = m_GeomClient.Query(objSend, Credentials) as List<ProtocolMessages.DisplayedGeometry>;
                var wkb = new GisSharpBlog.NetTopologySuite.IO.WKBReader();
                
                for (int iRes = 0; iRes < resObj.Count; ++iRes)
                {
                    gdo.Add(
                            new GDO()
                            {
                                File = new FileInfo()
                                {
                                    FileType = convertToFileType(resObj[iRes].fileType),
                                    IsBackup = resObj[iRes].isBackup,
                                    Status = convertToMediaFileStatus(resObj[iRes].fileStatus),
                                    Task = String.Empty,
                                    LocalPath = resObj[iRes].trackFilePath
                                },
                                Geometry = wkb.Read(resObj[iRes].geometry) as Geometry,
                                GdoID = guids[iRes],
                            });
                }
                
                return gdo.ToArray();
            }
            catch (Exception ex)
            {
                return new GDO[] { }; 
            }
        }
        
        private void TerminateLoad(Credentials _credentials, Guid _sessionID)
        {
             Thread   tTerminate = new Thread(TerminateLoadExec);
             tTerminate.Start(new Object[] { _credentials, _sessionID.ToString() });
        }

        public void LoadData(double _y0, double _x0, double _y1, double _x1, Settings _settings)
        {
            Thread tLoad = new Thread(LoadDataProc);
            tLoad.Start(new Object[] { _y0, _x0, _y1, _x1, _settings });
        }

        private void LoadDataProc(Object arg)
        {
            DateTime dtThisAttemptedUpload = DateTime.Now;
            dtLastAttemptedUpload = dtThisAttemptedUpload;
            TimeSpan delay = dtLastAttemptedUpload.Subtract(dtLastUploadStart);
            if (!Plugin.Instance.ForceLoadData && delay.TotalSeconds < 5)
            {
                Thread.Sleep(1000 * (5 - (int)delay.TotalSeconds));
                if (dtThisAttemptedUpload < dtLastAttemptedUpload)
                    return;
            }

            if (Credentials == null || String.IsNullOrEmpty(Credentials.Login) ||
                    String.IsNullOrEmpty(Credentials.Server) || String.IsNullOrEmpty(Credentials.PasswordHash) &&
                    String.IsNullOrEmpty(Credentials.Password))
            {
                SetRenderStatus(RenderStatusEnum.Error);
                m_pHost.ReportMessage("Логин, пароль или адрес сервера не введены", true);
                return;
            }

            double _y0;
            double _x0;
            double _y1;
            double _x1;
            Settings settings;
            List<ProtocolMessages.FileType> fileTypeGeom;
            Nullable<FileType> fileType;
            Nullable<bool> isBackup;
            Nullable<DateTime> fromDate;
            Nullable<DateTime> toDate;
            Guid sessionID;
            FileType[] filter;
            Polygon region;
            GetRequestParams(arg, out _y0, out _x0, out _y1, out _x1, out settings, out fileTypeGeom, out fileType, out isBackup, out fromDate, out toDate, out sessionID, out filter, out region);

            if (!settings.ShowVideoTracks & !settings.ShowRoadNotes & !settings.ShowTracks)
            {
                SetRenderStatus(RenderStatusEnum.Ready);
                return;
            }

            if (IsServerHttp)
                LoadDataFromHttpProc(_y0, _x0, _y1, _x1, settings, fileTypeGeom, isBackup, fromDate, toDate, sessionID, filter, region);
            else
                LoadDataFromWCFProc(_y0, _x0, _y1, _x1, settings, fileType, isBackup, fromDate, toDate, sessionID, filter, region);
        }

        private void LoadDataFromHttpProc(
            double _y0, double _x0, double _y1, double _x1,
            Settings settings,
            List<ProtocolMessages.FileType> fileTypeGeom,
            Nullable<bool> isBackup,
            Nullable<DateTime> fromDate,
            Nullable<DateTime> toDate,
            Guid sessionID,
            FileType[] filter,
            Polygon region)
        {
            Plugin.Instance.ForceLoadData = false;

            try
            {
                List<GDO> newData = new List<GDO>();

                lock (lockGeom)
                {
                    Guid[] ids = GetGeomObjectsIDs(new Object[] { _y0, _x0, _y1, _x1, fileTypeGeom, isBackup, fromDate, toDate });
                    try
                    {
                        List<Guid> diff = AddFromCache(ids, newData);
                        if (diff.Count > 0)
                        {
                            int offset = 0;
                            GDO[] r;
                            Guid[] objIDs = null;
                            do
                            {
                                if (CheckBreakLoadCondition(_y0, _x0, _y1, _x1, settings))
                                {
                                    if (offset > 0)
                                        TerminateLoad(Credentials, sessionID);
                                    return;
                                }
                                objIDs = diff.Skip(offset).Take(1000).ToArray();
                                r = GetGeomObjectsByIDs(sessionID, objIDs);
                                if (r.Length != objIDs.Length)
                                    MessageBox.Show("Data integrity error! Please contact the administrator!");
                                AddRange(r, newData);
                                Refresh();
                                offset += r.Length;
                            } while (r.Length == 1000);
                        }

                        CommitNewData(newData, filter);
                        SetRenderStatus(RenderStatusEnum.Ready);
                    }
                    catch (Exception e)
                    {
                        SetRenderStatus(RenderStatusEnum.Error);
                        m_pHost.ReportMessage(e.Message, true);
                    }
                }
            }

            catch (Exception e)
            {
                SetRenderStatus(RenderStatusEnum.Error);
                m_pHost.ReportMessage(e.Message, true);
            }
        }

        private void LoadDataFromWCFProc(
            double _y0, double _x0, double _y1, double _x1,
            Settings settings,
            Nullable<FileType> fileType,
            Nullable<bool> isBackup,
            Nullable<DateTime> fromDate,
            Nullable<DateTime> toDate,
            Guid sessionID,
            FileType[] filter,
            Polygon region)
        {
            Plugin.Instance.ForceLoadData = false;

            try
            {
                List<GDO> newData = new List<GDO>();

                GDOStorageServiceClient proxy = GDOStorageServiceClient.Create(Credentials);
                proxy.BeginGetObjectIDs6(
                    sessionID,
                    region, fileType, isBackup,
                    Settings.ShowRemoteData, DisplayedRemoteServers,
                    String.Empty, m_pHost.Scale, fromDate, toDate,
                    new AsyncCallback((async_result) =>
                    //proxy.BeginGetObjectIDs4(
                    //    sessionID,
                    //    region, fileType, isBackup, String.Empty, m_pHost.Scale, fromDate, toDate,
                    //    new AsyncCallback((async_result) =>
                    {
                        try
                        {
                            if (CheckBreakLoadCondition(_y0, _x0, _y1, _x1, settings))
                            {
                                TerminateLoad(Credentials, sessionID);
                                return;
                            }

                            dtLastUploadStart = DateTime.Now;
                            Guid[] ids = proxy.EndGetObjectIDs6(async_result);
                            //Guid[] ids = proxy.EndGetObjectIDs4(async_result);

                            using (GDOStorageServiceClient proxy2 = GDOStorageServiceClient.Create(Credentials))
                            {
                                List<Guid> diff = AddFromCache(ids, newData);
                                if (diff.Count > 0)
                                {
                                    int offset = 0;
                                    GDO[] r;
                                    Guid[] objIDs = null;
                                    do
                                    {
                                        if (CheckBreakLoadCondition(_y0, _x0, _y1, _x1, settings))
                                        {
                                            if (offset > 0)
                                                TerminateLoad(Credentials, sessionID);
                                            return;
                                        }
                                        objIDs = diff.Skip(offset).Take(1000).ToArray();
                                        r = proxy2.GetGDOByKeys2(sessionID, objIDs);
                                        AddRange(r, newData);
                                        Refresh();
                                        offset += r.Length;
                                    } while (r.Length == 1000);
                                }
                                CommitNewData(newData, filter);
                                SetRenderStatus(RenderStatusEnum.Ready);
                            }
                        }
                        catch (Exception e)
                        {
                            SetRenderStatus(RenderStatusEnum.Error);
                            m_pHost.ReportMessage(e.Message, true);
                        }
                        finally
                        {
                            try
                            {
                                proxy.Close();
                                //proxy2.Close();
                            }
                            catch (Exception e)
                            {
                                m_pHost.ReportMessage(e.Message, true);
                            }
                        }

                    }), (IAsyncResult)null);
            }

            catch (Exception e)
            {
                SetRenderStatus(RenderStatusEnum.Error);
                m_pHost.ReportMessage(e.Message, true);
            }
        }

        private void GetRequestParams(Object arg,
            out double _y0, out double _x0, out double _y1, out double _x1,
            out Settings settings,
            out List<ProtocolMessages.FileType> fileTypeGeom,
            out Nullable<FileType> fileType,
            out Nullable<bool> isBackup,
            out Nullable<DateTime> fromDate,
            out Nullable<DateTime> toDate,
            out Guid sessionID,
            out FileType[] filter,
            out Polygon region)
        {
            Object[] args = (Object[])arg;

            _y0 = (double)args[0];
            _x0 = (double)args[1];
            _y1 = (double)args[2];
            _x1 = (double)args[3];
            settings = (Settings)args[4];

            filter = BuildFileTypeFilter(settings);
            fileType = (filter.Length == 1) ? filter[0] : (Nullable<FileType>)null;
            isBackup = settings.ShowBackups ? (Nullable<bool>)null : (Nullable<bool>)false;
            fileTypeGeom = convertToFileType(filter);//for http

            lock (this)
            {
                m_y0 = _y0; m_y1 = _y1; m_x0 = _x0; m_x1 = _x1;
                Settings = settings;
            }

            fromDate = Settings.RestrictDates ? Settings.TrackMinDate : (Nullable<DateTime>)null;
            toDate = Settings.RestrictDates ? Settings.TrackMaxDate : (Nullable<DateTime>)null;

            sessionID = Guid.NewGuid();

            SetRenderStatus(RenderStatusEnum.Request);

            region = new Polygon(new LinearRing(new Coordinate[] 
                { 
                    new Coordinate(_x0, _y0), 
                    new Coordinate(_x0, _y1), 
                    new Coordinate(_x1, _y1), 
                    new Coordinate(_x1, _y0), 
                    new Coordinate(_x0, _y0) 
                }));
        }

        internal GDO[] GetAllData()
        {
            return m_pData.ToArray();
        } 

        public void RenderData(int _hdc)
        {
            lock (this)
            {
                System.Drawing.Graphics g = System.Drawing.Graphics.FromHdc((IntPtr)_hdc);
                PluginRenderContext ctx = new PluginRenderContext(g, m_pHost);
                GeometryRenderer renderer = new GeometryRenderer(ctx);
                FileType[] filter = BuildFileTypeFilter(Settings);

             
                for (int i = 0; i < 2; ++i)
                {
                    ctx.PreparingRange = (i == 0);

                    foreach (GDO item in m_pData)
                    {
                        if (!filter.Contains(item.File.FileType) ||
                            item.File.Status == MediaFileStatus.Invisible ||
                            item.File.Status == MediaFileStatus.ForRemoval)
                        continue;

                        bool bIsRemote = item.File.Status != MediaFileStatus.Local;
                         
                        try
                        {
                            if (!ctx.PreparingRange)
                            {
                                if (!bIsRemote)
                                {
                                    if (item.File.FileType == FileType.VideoTrack)
                                        ctx.Pen = SettingsViewModel.ColorSettings.GetPen(
                                            item.File.IsBackup ? "COLOR_RESERVE_VIDEOTRACK" : "COLOR_VIDEOTRACK");
                                    else if (item.File.FileType == FileType.RoadNotes)
                                        ctx.Pen = SettingsViewModel.ColorSettings.GetPen("COLOR_ROADNOTE");
                                    else if (item.File.FileType == FileType.Filter)
                                        ctx.Pen = SettingsViewModel.ColorSettings.GetPen("COLOR_FILTER");
                                    else if (item.File.FileType == FileType.Other)
                                        ctx.Pen = SettingsViewModel.ColorSettings.GetPen(
                                            item.File.IsBackup ? "COLOR_RESERVE_TRACK" : "COLOR_TRACK");
                                }
                                else
                                {
                                    if (item.File.FileType == FileType.VideoTrack)
                                        ctx.Pen = SettingsViewModel.ColorSettings.GetPen("COLOR_REMOTE_VIDEOTRACK");
                                    else if (item.File.FileType == FileType.Other)
                                        ctx.Pen = SettingsViewModel.ColorSettings.GetPen("COLOR_REMOTE_TRACK");
                                    else if (item.File.FileType == FileType.RoadNotes)
                                        ctx.Pen = SettingsViewModel.ColorSettings.GetPen("COLOR_REMOTE_ROADNOTES");
                                } 
                            }

                            renderer.PrepareRangeOrRender((Geometry)item.Geometry);
                        }
                        catch (Exception e)
                        {
                            SetRenderStatus(RenderStatusEnum.Error);
                            m_pHost.ReportMessage(e.Message, true);
                        } 
                    }
                }
            }
        }

        public bool DeleteFile(Guid guid)
        {
            try
            {
                GDOStorageServiceClient proxy = GDOStorageServiceClient.Create(Credentials);
                proxy.DeleteFile(guid);
            }
            catch (Exception e)
            {
                SetRenderStatus(RenderStatusEnum.Error);
                m_pHost.ReportMessage(e.Message, true);
            }
            return false;
        } 

        internal GDO[] GetObjectsNear(int x, int y)
        {
            lock (this)
            {
                double latitude, longitude;
                m_pHost.ConvertView2Position(x, y, out latitude, out longitude);
                Point p = new Point(longitude, latitude);
                double minDistance = (360 * m_pHost.Scale * Accuracy) / (40 * 100) / (40000 * 1000);
                FileType[] filter = BuildFileTypeFilter(Settings);
                var selected = m_pData.Where<GDO>((o) => (filter.Contains(o.File.FileType) 
                    && ((Geometry)o.Geometry).Distance(p) < minDistance));
                var notes = selected.Where((o) => o.File.FileType == FileType.RoadNotes).ToArray();
                return (notes.Length > 0) ? notes : selected.Take(1).ToArray();
            }
        }

        internal GDO[] GetSelectedPolygonObjs()
        {
            List<GDO> lstGDO = new List<GDO>();
            lock (this)
            {
                List<MultiPolygon> lstPolygons = MapEditHelper.LoadPolygons(Plugin.Instance.Application, true);
                IPolygon[] polygons = MapEditHelper.BuildPolygonArray(lstPolygons.ToArray());

                foreach (Polygon p in polygons)
                {
                    IEnumerable<GDO> match = m_pData.Where<GDO>((o) =>
                        (o.File.FileType == FileType.Filter) &&
                        ((IPolygon)o.Geometry).Intersection(p).Area / p.Area > 0.8);

                    if (match.Count<GDO>() > 0)
                        lstGDO.AddRange(match);
                }
            }

            return lstGDO.ToArray();
        } 

        List<Guid> AddFromCache(Guid[] ids, List<GDO> newData)
        {
            lock (this)
            {
                List<Guid> result = new List<Guid>();
                foreach (var key in ids)
                {
                    if (m_pCache.ContainsKey(key))
                    {
                        newData.Add(m_pCache[key]);
                    }
                    else
                        result.Add(key);
                }
                m_pData.AddRange(newData); // TODO : Remove duplicates
                return result;
            }
        }

        void AddRange(GDO[] r, List<GDO> newData)
        {
            lock (this)
            {
                foreach (var ent in r)
                    m_pCache[ent.GdoID] = ent;
                m_pData.AddRange(r);
            }
            newData.AddRange(r);
        }

        internal FileType[] BuildFileTypeFilter(Settings settings)
        {
            List<FileType> filter = new List<FileType>();
            if (settings.ShowRoadNotes)
                filter.Add(FileType.RoadNotes);
            if (settings.ShowTracks)
                filter.Add(FileType.Other);
            if (settings.ShowVideoTracks)
                filter.Add(FileType.VideoTrack);
            return filter.ToArray();
        }

        void CommitNewData(List<GDO> newData, FileType[] filter)
        {
            lock (this)
            {
                m_pData = newData;
            }
            Refresh();
        }

        void SetRenderStatus(RenderStatusEnum newStatus)
        {
            this.Settings.RenderStatus = newStatus;
            this.OnRenderStatusChanged(newStatus);
        }

        void Refresh()
        {
            ThreadPool.QueueUserWorkItem(new WaitCallback((x) => m_pHost.Refresh()));
        }
    }

    public static class GDOHelpers
    {
        internal static string GetHint(this GDO o)
        {
            return String.Format("Task:{0} \nFile:{1}{4}\nType:{2}\nStatus:{3}",
                o.File.Task,
                System.IO.Path.GetFileName(o.File.LocalPath),
                o.File.FileType,
                o.File.Status,
                o.File.IsBackup ? "(Backup)" : "");
        }
    }
}
