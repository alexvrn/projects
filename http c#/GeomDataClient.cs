using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using GDStorage.FileUtils;

using ProtoBuf;
using IO = System.IO;

namespace GDStorage.Viewer
{
    public class GeomDataClient
    {
        private  HttpWebRequest m_Request;
        private  HttpWebResponse m_Response;
        private  byte[] m_BufferOut;

        private Credentials m_credentials;

        private HttpWebRequest CreateRequest(Credentials _credentials)
        {
            string strServer = _credentials.Server.Contains("http") ?
                _credentials.Server : String.Format("{0}{1}", @"https://", _credentials.Server);

            HttpWebRequest outRequest = (HttpWebRequest)WebRequest.Create(strServer);

            //Authorization
            outRequest.ContentType = "application/x-www-form-urlencoded";

            byte[] credentialsBytes = new UTF8Encoding().GetBytes(_credentials.Login + ":" + _credentials.PasswordHash);
            outRequest.Headers["Authorization"] = "Basic " + Convert.ToBase64String(credentialsBytes);

            return outRequest;
        }

        private void Init(Credentials credentials)
        {
            m_credentials = credentials;
        }

        public bool HttpServerPresent(Credentials _credentials, out HttpStatusCode _httpStatus)
        {
            _httpStatus = HttpStatusCode.BadGateway;

            try
            {
                //отключение проверки сертификата
                ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };

                HttpWebRequest request = CreateRequest(_credentials);

                request.Method = "POST";
                request.ContentLength = 0;
                request.Timeout = 100000;
                HttpWebResponse response = (HttpWebResponse)request.GetResponse();
                _httpStatus = response.StatusCode;
                return (_httpStatus == HttpStatusCode.OK || _httpStatus == HttpStatusCode.Accepted);
            }
            catch (System.Net.WebException we)
            {
                if (we.Response != null)
                    _httpStatus = ((HttpWebResponse)we.Response).StatusCode;
                return (
                    _httpStatus == HttpStatusCode.MethodNotAllowed || 
                    _httpStatus == HttpStatusCode.NotFound || 
                    _httpStatus == HttpStatusCode.Forbidden ||
                    _httpStatus == HttpStatusCode.Unauthorized ||
                    _httpStatus == HttpStatusCode.Accepted
                    );
            }
            catch (Exception e)
            {
                return false;
            }
        }

        private T FromBytesToObject<T>(byte[] bytes)
        {
            try
            {
                using (IO.Stream mem = new IO.MemoryStream(bytes, 0, bytes.Length))
                {
                    T obj = ProtoBuf.Serializer.Deserialize<T>(mem);
                    return obj;
                }
            }
            catch (Exception ex)
            {
                return default(T);
            }
        }

        private byte[] FromObjectToBytes<T>(T obj)
        {
            try
            {
                using (var stream = new IO.MemoryStream())
                {
                    stream.Position = 0;
                    ProtoBuf.Serializer.Serialize<T>(stream, obj);
                    return stream.ToArray();
                }
            }
            catch (Exception ex)
            {
                return null;
            }
        }

        public object Query(ProtocolMessages.ClientRequest obj, Credentials credentials)
        {
            //отключение проверки сертификата
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };

            Init(credentials);

            byte[] data = FromObjectToBytes<ProtocolMessages.ClientRequest>(obj);
            InitParam(data.Length);

            using (m_Response = GetResponse(m_Request, data))
            {
                if (m_Response != null)
                {
                    switch (obj.messageType)
                    {
                        case ProtocolMessages.ClientRequest.MessageType.DATA_FILTER:
                            {
                                return ProtoBuf.Serializer.Deserialize<ProtocolMessages.DisplayedGeometriesInfo>(m_Response.GetResponseStream());
                            }
                        case ProtocolMessages.ClientRequest.MessageType.GEOMETRIES:
                            {
                                using (var inBinReader = new IO.BinaryReader(m_Response.GetResponseStream()))
                                {
                                    List<ProtocolMessages.DisplayedGeometry> lstGeometry = new List<ProtocolMessages.DisplayedGeometry>();

                                    char ch = '\0';
                                    string strHex = String.Empty;
                                    Int32 cntBytes = 0;
                                    while (true)
                                    {
                                        strHex = String.Empty;
                                        while ((ch = inBinReader.ReadChar()) != '\r')
                                            strHex += ch;
                                        inBinReader.ReadByte();//\n
                                        if (!int.TryParse(strHex,
                                                          System.Globalization.NumberStyles.HexNumber,
                                                          System.Globalization.CultureInfo.InvariantCulture, out cntBytes))
                                        {
                                            System.Windows.MessageBox.Show("Error: Can not parse the string!");
                                            return lstGeometry;
                                        }
                                        else
                                        {
                                            if (cntBytes == 0)
                                                break;
                                            lstGeometry.Add(
                                                ProtoBuf.Serializer.Deserialize<ProtocolMessages.DisplayedGeometry>(
                                                    new IO.MemoryStream(inBinReader.ReadBytes(cntBytes))));
                                        }
                                        inBinReader.ReadBytes(2);//\r\n
                                    }
                                    return lstGeometry;
                                }
                            }
                        default:
                            {
                                return null;
                            }
                    }
                }
                else
                    return null;
            }
        }

        private HttpWebResponse GetResponse(HttpWebRequest _request, byte[] data)
        {
            m_BufferOut = data;

            if (data.Length == 0)
                return (HttpWebResponse)_request.GetResponse();

            using (var newStream = _request.GetRequestStream())
            {
                newStream.Write(data, 0, data.Length);
                newStream.Close();
            }

            return (HttpWebResponse)_request.GetResponse();
        }

        private void InitParam(int ContentLength)
        {
            m_Request = null;
            m_Response = null;
            m_BufferOut = null;

            m_Request = CreateRequest(m_credentials);

            m_Request.Method = "POST";
            m_Request.ContentLength = ContentLength;
        }

    }
}
