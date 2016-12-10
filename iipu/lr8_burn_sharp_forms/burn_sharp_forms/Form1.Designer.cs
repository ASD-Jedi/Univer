namespace burn_sharp_forms
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.AvailableCD = new System.Windows.Forms.ListBox();
            this.fileToBurn = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.barFreeSpace = new System.Windows.Forms.ProgressBar();
            this.backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
            this.btnBurn = new System.Windows.Forms.Button();
            this.btnClearFiles = new System.Windows.Forms.Button();
            this.CheckBoxEject = new System.Windows.Forms.CheckBox();
            this.progressBarStatusWriting = new ProgressBarWithCaption();
            this.SuspendLayout();
            // 
            // AvailableCD
            // 
            this.AvailableCD.FormattingEnabled = true;
            this.AvailableCD.Location = new System.Drawing.Point(1, 23);
            this.AvailableCD.Name = "AvailableCD";
            this.AvailableCD.Size = new System.Drawing.Size(120, 238);
            this.AvailableCD.TabIndex = 0;
            this.AvailableCD.SelectedIndexChanged += new System.EventHandler(this.AvailableCD_SelectedIndexChanged);
            // 
            // fileToBurn
            // 
            this.fileToBurn.FormattingEnabled = true;
            this.fileToBurn.Location = new System.Drawing.Point(127, 23);
            this.fileToBurn.Name = "fileToBurn";
            this.fileToBurn.Size = new System.Drawing.Size(462, 160);
            this.fileToBurn.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 7);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(66, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "AailableCDS";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(124, 7);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(64, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Files to burn";
            // 
            // barFreeSpace
            // 
            this.barFreeSpace.Location = new System.Drawing.Point(127, 199);
            this.barFreeSpace.Name = "barFreeSpace";
            this.barFreeSpace.Size = new System.Drawing.Size(462, 35);
            this.barFreeSpace.TabIndex = 4;
            // 
            // backgroundWorker1
            // 
            this.backgroundWorker1.WorkerReportsProgress = true;
            this.backgroundWorker1.WorkerSupportsCancellation = true;
            this.backgroundWorker1.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker1_DoWork);
            this.backgroundWorker1.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.backgroundWorker1_ProgressChanged);
            this.backgroundWorker1.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.backgroundWorker1_RunWorkerCompleted);
            // 
            // btnBurn
            // 
            this.btnBurn.Location = new System.Drawing.Point(595, 23);
            this.btnBurn.Name = "btnBurn";
            this.btnBurn.Size = new System.Drawing.Size(75, 23);
            this.btnBurn.TabIndex = 5;
            this.btnBurn.Text = "Burn";
            this.btnBurn.UseVisualStyleBackColor = true;
            this.btnBurn.Click += new System.EventHandler(this.btnBurn_Click);
            // 
            // btnClearFiles
            // 
            this.btnClearFiles.Location = new System.Drawing.Point(596, 83);
            this.btnClearFiles.Name = "btnClearFiles";
            this.btnClearFiles.Size = new System.Drawing.Size(75, 23);
            this.btnClearFiles.TabIndex = 7;
            this.btnClearFiles.Text = "Clear files";
            this.btnClearFiles.UseVisualStyleBackColor = true;
            this.btnClearFiles.Click += new System.EventHandler(this.btnClearFiles_Click);
            // 
            // CheckBoxEject
            // 
            this.CheckBoxEject.AutoSize = true;
            this.CheckBoxEject.Location = new System.Drawing.Point(691, 28);
            this.CheckBoxEject.Name = "CheckBoxEject";
            this.CheckBoxEject.Size = new System.Drawing.Size(86, 17);
            this.CheckBoxEject.TabIndex = 8;
            this.CheckBoxEject.Text = "Eject on end";
            this.CheckBoxEject.UseVisualStyleBackColor = true;
            // 
            // progressBarStatusWriting
            // 
            this.progressBarStatusWriting.CustomText = "";
            this.progressBarStatusWriting.DisplayStyle = ProgressBarDisplayText.CustomText;
            this.progressBarStatusWriting.Location = new System.Drawing.Point(596, 53);
            this.progressBarStatusWriting.Name = "progressBarStatusWriting";
            this.progressBarStatusWriting.Size = new System.Drawing.Size(245, 23);
            this.progressBarStatusWriting.TabIndex = 6;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(853, 262);
            this.Controls.Add(this.CheckBoxEject);
            this.Controls.Add(this.btnClearFiles);
            this.Controls.Add(this.progressBarStatusWriting);
            this.Controls.Add(this.btnBurn);
            this.Controls.Add(this.barFreeSpace);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.fileToBurn);
            this.Controls.Add(this.AvailableCD);
            this.Name = "Form1";
            this.Text = "CdBurn";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox AvailableCD;
        private System.Windows.Forms.ListBox fileToBurn;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ProgressBar barFreeSpace;
        private System.ComponentModel.BackgroundWorker backgroundWorker1;
        private System.Windows.Forms.Button btnBurn;
        private ProgressBarWithCaption progressBarStatusWriting;
        private System.Windows.Forms.Button btnClearFiles;
        private System.Windows.Forms.CheckBox CheckBoxEject;
    }
}

