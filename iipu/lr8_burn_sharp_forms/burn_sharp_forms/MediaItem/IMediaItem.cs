using IMAPI2.Interop;
using System;

namespace burn_sharp_forms.MediaItem
{
    interface IMediaItem
    {
        string FullName { get; set; }
        string ShortName { get; set; }
        Int64 SizeOnDisc { get; set; }
        bool AddToFileSystem(IFsiDirectoryItem rootItem);
    }
}
