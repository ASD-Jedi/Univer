using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using IMAPI2.Interop;
using System.IO;
using System.Windows.Forms;

namespace burn_sharp_forms.MediaItem
{
    class DirectoryItem : IMediaItem
    {
        #region variables
        string fullName;
        string shortName;
        Int64 sizeOnDisc;
        List<IMediaItem> mediaItems = new List<IMediaItem>();
        #endregion

        #region getters and setters
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
        public DirectoryItem(string fullName)
        {
            if(!Directory.Exists(fullName))
            {
                throw new FileNotFoundException("Directory not found", fullName);
            }
            this.fullName = fullName;
            FileInfo fileInfo = new FileInfo(fullName);
            shortName = fileInfo.Name;
            string[] files = Directory.GetFiles(fullName);
            foreach(string file in files)
            {
                mediaItems.Add(new FileItem(file));
            }
            string[] directories = Directory.GetDirectories(fullName);
            foreach(string directory in directories)
            {
                mediaItems.Add(new DirectoryItem(directory));
            }
            sizeOnDisc = 0;
            foreach(IMediaItem item in mediaItems)
            {
                sizeOnDisc += item.SizeOnDisc;
            }
        }
        public bool AddToFileSystem(IFsiDirectoryItem rootItem)
        {
            try
            {
                rootItem.AddTree(fullName, true);
                return true;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error adding folder",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);
                return false;
            }
        }
    }
}
