using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Windows.Forms;
using IMAPI2.Interop;

namespace burn_sharp_forms.MediaItem
{
    class FileItem : IMediaItem
    {
        private const uint STGM_SHARE_DENY_WRITE = 0x00000020;
        private const uint STGM_SHARE_DENY_NONE = 0x00000040;
        private const uint STGM_READ = 0x00000000;
        private const uint STGM_WRITE = 0x00000001;
        private const uint STGM_READWRITE = 0x00000002;

        [DllImport("shlwapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true, PreserveSig = false, EntryPoint = "SHCreateStreamOnFileW")]
        static extern void SHCreateStreamOnFile(string fileName, uint mode, ref IStream stream);
        #region variables
        string fullName;
        string shortName;
        Int64 sizeOnDisc;
        #endregion

        #region getter and setters
        public string FullName
        {
            get
            {
                return fullName;
            }

            set
            {
                fullName = value;
            }
        }

        public string ShortName
        {
            get
            {
                return shortName;
            }

            set
            {
                shortName = value;
            }
        }

        long IMediaItem.SizeOnDisc
        {
            get
            {
                return sizeOnDisc;
            }

            set
            {
                sizeOnDisc = value;
            }
        }
        #endregion

        public FileItem(string fullName)
        {
            this.fullName = fullName;
            FileInfo fileInfo = new FileInfo(fullName);
            shortName = fileInfo.Name;
            sizeOnDisc = fileInfo.Length;
        }

        public bool AddToFileSystem(IFsiDirectoryItem rootItem)
        {
            IStream stream = null;

            try
            {
                SHCreateStreamOnFile(fullName, STGM_READ | STGM_SHARE_DENY_WRITE, ref stream);

                if (stream != null)
                {
                    rootItem.AddFile(shortName, stream);
                    return true;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error adding file",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
                if (stream != null)
                {
                    Marshal.FinalReleaseComObject(stream);
                }
            }

            return false;

        }
    }
}
