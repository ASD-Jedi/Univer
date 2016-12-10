using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using IMAPI2.Interop;
using burn_sharp_forms.MediaItem;

namespace burn_sharp_forms
{
    public partial class Form1 : Form
    {
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool GetDiskFreeSpaceEx(string lpDirectoryName,
        out ulong lpFreeBytesAvailable,
        out ulong lpTotalNumberOfBytes,
        out ulong lpTotalNumberOfFreeBytes);

        #region variables
        string clientName = "BurnMedia";
        private IMAPI_BURN_VERIFICATION_LEVEL verificationLevel = IMAPI_BURN_VERIFICATION_LEVEL.IMAPI_BURN_VERIFICATION_NONE;
        double totalSpace = 0;
        double busySpace = 0;
        double freeSpace = 0;
        bool stopWriteInFuture = false;
        bool ejectMedia = false;
        ProgressBarWithCaption progressBarSpaceOnDisc = new ProgressBarWithCaption();
        BurnData _burnData = new BurnData();
        List<IMediaItem> mediaItems = new List<IMediaItem>();
        Dictionary<string, MsftDiscRecorder2> discs = new Dictionary<string, MsftDiscRecorder2>();
        #endregion

        #region Form and conrols
        public Form1()
        {
            this.Controls.Add(progressBarSpaceOnDisc);
            InitializeComponent();
            progressBarSpaceOnDisc.Bounds = this.barFreeSpace.Bounds;
            this.barFreeSpace.Visible = false;
            progressBarSpaceOnDisc.Visible = true;
            progressBarSpaceOnDisc.DisplayStyle = ProgressBarDisplayText.CustomText;
            progressBarSpaceOnDisc.Minimum = 0;
            progressBarSpaceOnDisc.Maximum = 100;
            progressBarSpaceOnDisc.CustomText = String.Format("Select a disc");
            this.AllowDrop = true;
            this.DragEnter += new DragEventHandler(Form1_DragEnter);
            this.DragDrop += new DragEventHandler(Form1_DragDrop);
        }
        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop)) e.Effect = DragDropEffects.Copy;
        }
        private void addDirectory(string dirPath)
        {
            string[] filesPathes = Directory.GetFiles(dirPath);
            foreach (string filePath in filesPathes)
            {
                this.fileToBurn.Items.Add(filePath);
                busySpace += (double)(new FileInfo(filePath).Length);
            }
            string[] subDirs = Directory.GetDirectories(dirPath);
            foreach (string subdirPath in subDirs)
            {
                addDirectory(subdirPath);
            }
        }
        private void Form1_DragDrop(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
            foreach (string file in files)
            {
                FileAttributes attr = File.GetAttributes(file);
                if (attr.HasFlag(FileAttributes.Directory))
                {
                    addDirectory(file);
                    mediaItems.Add(new DirectoryItem(file));
                }
                else
                {
                    this.fileToBurn.Items.Add(file);
                    busySpace += (double)(new FileInfo(file).Length);
                    mediaItems.Add(new FileItem(file));
                }
            }
            updateProgressBar();
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            //get available CDS
            MsftDiscMaster2 discMaster = null;
            try
            {
                discMaster = new MsftDiscMaster2();
                if (!discMaster.IsSupportedEnvironment)
                    return;
                foreach (string uniqueRecorderId in discMaster)
                {
                    var discRecorder = new MsftDiscRecorder2();
                    discRecorder.InitializeDiscRecorder(uniqueRecorderId);
                    if (discRecorder.VolumePathNames.Count() > 0)
                    {
                        AvailableCD.Items.Add(discRecorder.VolumePathNames.First().ToString());
                        discs[discRecorder.VolumePathNames.First().ToString()] = discRecorder;
                    }
                }
            }
            catch (COMException ex)
            {
                MessageBox.Show(ex.Message,
                    string.Format("Error:{0} - Please install IMAPI2", ex.ErrorCode),
                    MessageBoxButtons.OK, MessageBoxIcon.Stop);
                return;
            }
            finally
            {
                if (discMaster != null)
                {
                    Marshal.ReleaseComObject(discMaster);
                }
            }
        }
        private void AvailableCD_SelectedIndexChanged(object sender, EventArgs e)
        {
            MsftDiscRecorder2 discRecorder = discs[AvailableCD.Items[AvailableCD.SelectedIndex].ToString()];
            MsftFileSystemImage fileSystemImage = null;
            MsftDiscFormat2Data discFormatData = null;
            try
            {
                discFormatData = new MsftDiscFormat2Data();
                if (!discFormatData.IsCurrentMediaSupported(discRecorder))
                {
                    MessageBox.Show("Media not supported");
                    return;
                }
                else
                {
                    discFormatData.Recorder = discRecorder;
                    fileSystemImage = new MsftFileSystemImage();
                    try
                    {
                        fileSystemImage.ChooseImageDefaultsForMediaType(discFormatData.CurrentPhysicalMediaType);
                    }
                    catch
                    {
                        MessageBox.Show(AvailableCD.SelectedItem.ToString() + " read only");
                        return;
                    }
                    if (!discFormatData.MediaHeuristicallyBlank)
                    {
                        fileSystemImage.MultisessionInterfaces = discFormatData.MultisessionInterfaces;
                        fileSystemImage.ImportFileSystem();
                    }
                    Int64 totalBlocks = fileSystemImage.UsedBlocks + fileSystemImage.FreeMediaBlocks;
                    this.totalSpace = getGbSize(totalBlocks);
                    this.freeSpace = getGbSize(fileSystemImage.FreeMediaBlocks);
                    this.busySpace = getGbSize(fileSystemImage.UsedBlocks);
                    this.updateProgressBar();
                }
            }
            catch (COMException exception)
            {
                MessageBox.Show(this, exception.Message, "Detect Media Error",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
                if (discFormatData != null)
                {
                    Marshal.ReleaseComObject(discFormatData);
                }

                if (fileSystemImage != null)
                {
                    Marshal.ReleaseComObject(fileSystemImage);
                }
            }
        }
        private void btnBurn_Click(object sender, EventArgs e)
        {
            if (this.fileToBurn.Items.Count == 0)
            {
                MessageBox.Show("Add files to burn", "Info", MessageBoxButtons.OK);
                return;
            } else
            {
                if(busySpace/1024/1024/1024 > totalSpace)
                {
                    MessageBox.Show("Too many data, delete some files");
                    return;
                }
                ejectMedia = CheckBoxEject.Checked;
                _burnData.uniqueRecorderId = discs[AvailableCD.SelectedItem.ToString()].ActiveDiscRecorder;
                _burnData.volumeName = discs[AvailableCD.SelectedItem.ToString()].VolumeName;
                backgroundWorker1.RunWorkerAsync(_burnData);
            }
        }
        private void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e)
        {
            MsftDiscRecorder2 discRecorder = null;
            MsftDiscFormat2Data discFormatData = null;

            try
            {
                discRecorder = new MsftDiscRecorder2();
                var burnData = (BurnData)e.Argument;
                discRecorder.InitializeDiscRecorder(burnData.uniqueRecorderId);

                discFormatData = new MsftDiscFormat2Data
                {
                    Recorder = discRecorder,
                    ClientName = clientName,
                    ForceMediaToBeClosed = stopWriteInFuture
                };

                var burnVerification = (IBurnVerification)discFormatData;
                burnVerification.BurnVerificationLevel = verificationLevel;

                object[] multisessionInterfaces = null;
                if (!discFormatData.MediaHeuristicallyBlank)
                {
                    multisessionInterfaces = discFormatData.MultisessionInterfaces;
                }

                IStream fileSystem;
                if (!CreateMediaFileSystem(discRecorder, multisessionInterfaces, out fileSystem, _burnData.volumeName))
                {
                    e.Result = -1;
                    return;
                }

                discFormatData.Update += discFormatData_Update;

                try
                {
                    discFormatData.Write(fileSystem);
                    e.Result = 0;
                }
                catch (COMException ex)
                {
                    e.Result = ex.ErrorCode;
                    MessageBox.Show(ex.Message, "IDiscFormat2Data.Write failed",
                        MessageBoxButtons.OK, MessageBoxIcon.Stop);
                }
                finally
                {
                    if (fileSystem != null)
                    {
                        Marshal.FinalReleaseComObject(fileSystem);
                    }
                }

                discFormatData.Update -= discFormatData_Update;
            }
            catch (COMException exception)
            {
                MessageBox.Show(exception.Message);
                e.Result = exception.ErrorCode;
            }
            finally
            {
                if (discRecorder != null)
                {
                    Marshal.ReleaseComObject(discRecorder);
                }

                if (discFormatData != null)
                {
                    Marshal.ReleaseComObject(discFormatData);
                }
            }

        }
        private void backgroundWorker1_ProgressChanged(object sender, ProgressChangedEventArgs e)
        { 
            var burnData = (BurnData)e.UserState;
            if (burnData.task == BURN_MEDIA_TASK.BURN_MEDIA_TASK_FILE_SYSTEM)
            {
                progressBarStatusWriting.CustomText = burnData.statusMessage;
            }
            else if (burnData.task == BURN_MEDIA_TASK.BURN_MEDIA_TASK_WRITING)
            {
                #region BigSwitch
                switch (burnData.currentAction)
                {
                    case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA:
                        progressBarStatusWriting.CustomText = "Validating current media...";
                        break;

                    case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_FORMATTING_MEDIA:
                        progressBarStatusWriting.CustomText = "Formatting media...";
                        break;

                    case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_INITIALIZING_HARDWARE:
                        progressBarStatusWriting.CustomText = "Initializing hardware...";
                        break;

                    case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_CALIBRATING_POWER:
                        progressBarStatusWriting.CustomText = "Optimizing laser intensity...";
                        break;

                    case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA:
                        long writtenSectors = burnData.lastWrittenLba - burnData.startLba;

                        if (writtenSectors > 0 && burnData.sectorCount > 0)
                        {
                            var percent = (int)((100 * writtenSectors) / burnData.sectorCount);
                            progressBarStatusWriting.CustomText = string.Format("Progress: {0}%", percent);
                            progressBarStatusWriting.Value = percent;
                        }
                        else
                        {
                            progressBarStatusWriting.CustomText = "Progress 0%";
                            progressBarStatusWriting.Value = 0;
                        }
                        break;

                    case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_FINALIZATION:
                        progressBarStatusWriting.CustomText = "Finalizing writing...";
                        break;

                    case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_COMPLETED:
                        progressBarStatusWriting.CustomText = "Completed!";
                        break;

                    case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_VERIFYING:
                        progressBarStatusWriting.CustomText = "Verifying";
                        break;
                }
                #endregion
            }
        }
        private void backgroundWorker1_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            MessageBox.Show("Burn ready", "Info", MessageBoxButtons.OK);
            progressBarStatusWriting.CustomText = "";
            progressBarStatusWriting.Value = 0;
            if(ejectMedia)
            {
                discs[AvailableCD.SelectedItem.ToString()].EjectMedia();
            }
        }
        private void btnClearFiles_Click(object sender, EventArgs e)
        {
            mediaItems.Clear();
            fileToBurn.Items.Clear();
            busySpace = 0;
            updateProgressBar();
        }
        #endregion

        #region helperFunctions
        private void updateProgressBar()
        {
            progressBarSpaceOnDisc.CustomText = String.Format("{0:0.##} of {1:0.00} gb", this.busySpace / 1024 / 1024 / 1024, this.totalSpace);
            progressBarSpaceOnDisc.Value = (int)((double)(this.busySpace / 1024 / 1024 / 1024 / totalSpace) * 100);
        }
        private double getGbSize(Int64 blocks)
        {
            return (double)((ulong)blocks * 2048) / 1024 / 1024 / 1024;
        }
        #endregion

        void discFormatData_Update([In, MarshalAs(UnmanagedType.IDispatch)] object sender, [In, MarshalAs(UnmanagedType.IDispatch)] object progress)
        {
            if (backgroundWorker1.CancellationPending)
            {
                var format2Data = (IDiscFormat2Data)sender;
                format2Data.CancelWrite();
                return;
            }

            var eventArgs = (IDiscFormat2DataEventArgs)progress;

            _burnData.task = BURN_MEDIA_TASK.BURN_MEDIA_TASK_WRITING;
            _burnData.elapsedTime = eventArgs.ElapsedTime;
            _burnData.remainingTime = eventArgs.RemainingTime;
            _burnData.totalTime = eventArgs.TotalTime;
            _burnData.currentAction = eventArgs.CurrentAction;
            _burnData.startLba = eventArgs.StartLba;
            _burnData.sectorCount = eventArgs.SectorCount;
            _burnData.lastReadLba = eventArgs.LastReadLba;
            _burnData.lastWrittenLba = eventArgs.LastWrittenLba;
            _burnData.totalSystemBuffer = eventArgs.TotalSystemBuffer;
            _burnData.usedSystemBuffer = eventArgs.UsedSystemBuffer;
            _burnData.freeSystemBuffer = eventArgs.FreeSystemBuffer;
            backgroundWorker1.ReportProgress(0, _burnData);
        }
        private bool CreateMediaFileSystem(IDiscRecorder2 discRecorder, object[] multisessionInterfaces, out IStream dataStream, string volumeName)
        {
            MsftFileSystemImage fileSystemImage = null;
            try
            {
                fileSystemImage = new MsftFileSystemImage();
                fileSystemImage.ChooseImageDefaults(discRecorder);
                fileSystemImage.FileSystemsToCreate =
                    FsiFileSystems.FsiFileSystemJoliet | FsiFileSystems.FsiFileSystemISO9660;
                DateTime now = DateTime.Now;
                fileSystemImage.VolumeName = now.Year + "_" + now.Month + "_" + now.Day;

                fileSystemImage.Update += fileSystemImage_Update;

                // If multisessions, then import previous sessions
                if (multisessionInterfaces != null)
                {
                    fileSystemImage.MultisessionInterfaces = multisessionInterfaces;
                    fileSystemImage.ImportFileSystem();
                }

                IFsiDirectoryItem rootItem = fileSystemImage.Root;

                foreach (IMediaItem mediaItem in mediaItems)
                {
                    if (backgroundWorker1.CancellationPending)
                    {
                        break;
                    }

                    mediaItem.AddToFileSystem(rootItem);
                }

                fileSystemImage.Update -= fileSystemImage_Update;

                if (backgroundWorker1.CancellationPending)
                {
                    dataStream = null;
                    return false;
                }

                dataStream = fileSystemImage.CreateResultImage().ImageStream;
            }
            catch (COMException exception)
            {
                MessageBox.Show(this, exception.Message, "Create File System Error",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
                dataStream = null;
                return false;
            }
            finally
            {
                if (fileSystemImage != null)
                {
                    Marshal.ReleaseComObject(fileSystemImage);
                }
            }

            return true;
        }
        void fileSystemImage_Update([In, MarshalAs(UnmanagedType.IDispatch)] object sender,
            [In, MarshalAs(UnmanagedType.BStr)]string currentFile, [In] int copiedSectors, [In] int totalSectors)
        {
            var percentProgress = 0;
            if (copiedSectors > 0 && totalSectors > 0)
            {
                percentProgress = (copiedSectors * 100) / totalSectors;
            }

            if (!string.IsNullOrEmpty(currentFile))
            {
                var fileInfo = new FileInfo(currentFile);
                _burnData.statusMessage = "Adding \"" + fileInfo.Name + "\" to image...";
                _burnData.task = BURN_MEDIA_TASK.BURN_MEDIA_TASK_FILE_SYSTEM;
                backgroundWorker1.ReportProgress(percentProgress, _burnData);
            }
        }

    }
}