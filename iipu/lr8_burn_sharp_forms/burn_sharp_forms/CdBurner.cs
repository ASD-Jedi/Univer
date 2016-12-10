using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
namespace burn_sharp_forms
{

    class CdBurner
    {
        static bool hasRecorder = false;
        [DllImport("shfolder.dll")]
        static extern int SHGetFolderPath(IntPtr hwndOwner, int nFolder,
                                          IntPtr hToken, int dwFlags,
                                          StringBuilder pszPath);

        const int CSIDL_CDBURN_AREA = 0x3B;
        const int SHGFP_TYPE_CURRENT = 0;
        static ICDBurn iface = null;
        static CdBurner()
        {
            Guid CLSID_CDBurn = new Guid("fbeb8a05-beee-4442-804e-409d6c4515e9");
            Type t = Type.GetTypeFromCLSID(CLSID_CDBurn);
            if (t != null)
            {
                iface = (ICDBurn)Activator.CreateInstance(t);
            }
        }
        public static string[] getCds()
        {
            iface.HasRecordableDrive(ref hasRecorder);
            if (hasRecorder)
            {
                StringBuilder driveLetter = new StringBuilder(4);
                iface.GetRecorderDriveLetter(driveLetter, 4);
                CdBurner.hasRecorder = false;
                return new string[] { driveLetter.ToString() };
            }
            return new string[] {"No CDS" };
        }
        public static string getFilePath()
        {
            StringBuilder szPath = new StringBuilder(1024);
            if (SHGetFolderPath((IntPtr)0, CSIDL_CDBURN_AREA, (IntPtr)0,
                SHGFP_TYPE_CURRENT, szPath) != 0)
                return null;
            else
                return szPath.ToString();
        }
        public static void Burn()
        {
            if (!hasRecorder || iface == null)
                return;
            iface.Burn(IntPtr.Zero);
        }
    }

    [ComImport]
    [Guid("3d73a659-e5d0-4d42-afc0-5121ba425c8d")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface ICDBurn
    {
        void GetRecorderDriveLetter([MarshalAs(UnmanagedType.LPWStr)]
                                StringBuilder pszDrive, uint cch);
        void Burn(IntPtr hwnd);
        void HasRecordableDrive(ref bool HasRecorder);
    }
}