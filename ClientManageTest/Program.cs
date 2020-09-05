using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UpdateClientManage;
namespace ClientManageTest
{
    class Program
    {
        static void Main(string[] args)
        {
            UpdateClient client = new UpdateClient();
            Char[] sb = new Char[256];
            Char[] sb2 = new Char[256];
            String str = "DTA";
            Int32 res = client.Authenticate(str);
            double filesize = client.GetFileSize();
            //client.StopDownLoad();
            Int32 persent = client.GetDownLoadPercent();
            bool bRes = false;
            if (res == 0 || res == 1)
            { 
                bRes = client.DownLoad(str, true, true);
                client.IsSocketError();
            }
            else
            {
                client.DownLoad(str, false, false);
                client.IsSocketError();
            }

            if (bRes)
            {
                persent = client.GetDownLoadPercent();
                client.GetFullFilePath(sb, 256);
                Console.WriteLine(sb);
                client.GetProgramName(sb2, 256);
                Console.WriteLine(sb2);
                UInt32 nVersion = client.GetFileVersion();
                Console.WriteLine("{0:X}", nVersion);

            }

            String str2 = "FIRMWARE1";
            Int32 res2 = client.Authenticate("FIRMWARE1");
            if (res == 0 || res == 1)
                client.DownLoad(str2, true, false);
            else
                client.DownLoad(str2, false, false);
        }
    }
}
