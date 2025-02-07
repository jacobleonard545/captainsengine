namespace StoryDesigner
{
    partial class CharacterDlg
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
            this.components = new System.ComponentModel.Container();
            this.text_color = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.panel_down = new System.Windows.Forms.Panel();
            this.label5 = new System.Windows.Forms.Label();
            this.startzoom = new System.Windows.Forms.NumericUpDown();
            this.ghostmode = new System.Windows.Forms.CheckBox();
            this.memory_resistant = new System.Windows.Forms.CheckBox();
            this.leftanim = new System.Windows.Forms.CheckBox();
            this.nozoom = new System.Windows.Forms.CheckBox();
            this.walkspeed = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.walksound = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.font = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.customize = new System.Windows.Forms.Button();
            this.customizeMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.copyActionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pasteActionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteActionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.renameActionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.equalizeAllSizesAndFootprintsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.equalizeAllActionsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.backgroundColorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.stateFrameImage1 = new StoryDesigner.StateFrameImage();
            this.panel_down.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.startzoom)).BeginInit();
            this.customizeMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // text_color
            // 
            this.text_color.BackColor = System.Drawing.Color.Lime;
            this.text_color.Location = new System.Drawing.Point(319, 26);
            this.text_color.Name = "text_color";
            this.text_color.Size = new System.Drawing.Size(23, 23);
            this.text_color.TabIndex = 2;
            this.text_color.UseVisualStyleBackColor = false;
            this.text_color.Click += new System.EventHandler(this.text_color_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(261, 31);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(52, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "TextColor";
            // 
            // panel_down
            // 
            this.panel_down.Controls.Add(this.label5);
            this.panel_down.Controls.Add(this.startzoom);
            this.panel_down.Controls.Add(this.ghostmode);
            this.panel_down.Controls.Add(this.memory_resistant);
            this.panel_down.Controls.Add(this.leftanim);
            this.panel_down.Controls.Add(this.nozoom);
            this.panel_down.Controls.Add(this.walkspeed);
            this.panel_down.Controls.Add(this.label4);
            this.panel_down.Controls.Add(this.walksound);
            this.panel_down.Controls.Add(this.label3);
            this.panel_down.Controls.Add(this.font);
            this.panel_down.Controls.Add(this.label2);
            this.panel_down.Location = new System.Drawing.Point(12, 395);
            this.panel_down.Name = "panel_down";
            this.panel_down.Size = new System.Drawing.Size(330, 127);
            this.panel_down.TabIndex = 4;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(207, 104);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(74, 13);
            this.label5.TabIndex = 11;
            this.label5.Text = "Start zoom (%)";
            // 
            // startzoom
            // 
            this.startzoom.Location = new System.Drawing.Point(282, 102);
            this.startzoom.Maximum = new decimal(new int[] {
            500,
            0,
            0,
            0});
            this.startzoom.Name = "startzoom";
            this.startzoom.Size = new System.Drawing.Size(45, 20);
            this.startzoom.TabIndex = 10;
            this.startzoom.ValueChanged += new System.EventHandler(this.startzoom_ValueChanged);
            // 
            // ghostmode
            // 
            this.ghostmode.AutoSize = true;
            this.ghostmode.Location = new System.Drawing.Point(210, 79);
            this.ghostmode.Name = "ghostmode";
            this.ghostmode.Size = new System.Drawing.Size(83, 17);
            this.ghostmode.TabIndex = 9;
            this.ghostmode.Text = "Ghost mode";
            this.ghostmode.UseVisualStyleBackColor = true;
            this.ghostmode.CheckedChanged += new System.EventHandler(this.ghostmode_CheckedChanged);
            // 
            // memory_resistant
            // 
            this.memory_resistant.AutoSize = true;
            this.memory_resistant.Location = new System.Drawing.Point(210, 58);
            this.memory_resistant.Name = "memory_resistant";
            this.memory_resistant.Size = new System.Drawing.Size(105, 17);
            this.memory_resistant.TabIndex = 8;
            this.memory_resistant.Text = "Memory resistant";
            this.memory_resistant.UseVisualStyleBackColor = true;
            this.memory_resistant.CheckedChanged += new System.EventHandler(this.memory_resistant_CheckedChanged);
            // 
            // leftanim
            // 
            this.leftanim.AutoSize = true;
            this.leftanim.Location = new System.Drawing.Point(210, 35);
            this.leftanim.Name = "leftanim";
            this.leftanim.Size = new System.Drawing.Size(118, 17);
            this.leftanim.TabIndex = 7;
            this.leftanim.Text = "Real left-animations";
            this.leftanim.UseVisualStyleBackColor = true;
            this.leftanim.CheckedChanged += new System.EventHandler(this.leftanim_CheckedChanged);
            // 
            // nozoom
            // 
            this.nozoom.AutoSize = true;
            this.nozoom.Location = new System.Drawing.Point(210, 12);
            this.nozoom.Name = "nozoom";
            this.nozoom.Size = new System.Drawing.Size(96, 17);
            this.nozoom.TabIndex = 6;
            this.nozoom.Text = "Do NOT Zoom";
            this.nozoom.UseVisualStyleBackColor = true;
            this.nozoom.CheckedChanged += new System.EventHandler(this.nozoom_CheckedChanged);
            // 
            // walkspeed
            // 
            this.walkspeed.FormattingEnabled = true;
            this.walkspeed.Location = new System.Drawing.Point(90, 80);
            this.walkspeed.Name = "walkspeed";
            this.walkspeed.Size = new System.Drawing.Size(100, 21);
            this.walkspeed.TabIndex = 5;
            this.walkspeed.SelectedIndexChanged += new System.EventHandler(this.walkspeed_SelectedIndexChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(17, 83);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(67, 13);
            this.label4.TabIndex = 4;
            this.label4.Text = "Walk speed:";
            // 
            // walksound
            // 
            this.walksound.Location = new System.Drawing.Point(90, 45);
            this.walksound.Name = "walksound";
            this.walksound.Size = new System.Drawing.Size(100, 20);
            this.walksound.TabIndex = 3;
            this.walksound.TextChanged += new System.EventHandler(this.walksound_TextChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 48);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(81, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Walking sound:";
            // 
            // font
            // 
            this.font.FormattingEnabled = true;
            this.font.Location = new System.Drawing.Point(90, 12);
            this.font.Name = "font";
            this.font.Size = new System.Drawing.Size(100, 21);
            this.font.TabIndex = 1;
            this.font.SelectedIndexChanged += new System.EventHandler(this.font_SelectedIndexChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(53, 15);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(31, 13);
            this.label2.TabIndex = 0;
            this.label2.Text = "Font:";
            // 
            // customize
            // 
            this.customize.Location = new System.Drawing.Point(180, 26);
            this.customize.Name = "customize";
            this.customize.Size = new System.Drawing.Size(75, 23);
            this.customize.TabIndex = 5;
            this.customize.Text = "Customize";
            this.customize.UseVisualStyleBackColor = true;
            this.customize.Click += new System.EventHandler(this.customize_Click);
            // 
            // customizeMenu
            // 
            this.customizeMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.copyActionToolStripMenuItem,
            this.pasteActionToolStripMenuItem,
            this.deleteActionToolStripMenuItem,
            this.toolStripSeparator1,
            this.renameActionToolStripMenuItem,
            this.toolStripSeparator2,
            this.equalizeAllSizesAndFootprintsToolStripMenuItem,
            this.equalizeAllActionsToolStripMenuItem,
            this.toolStripSeparator3,
            this.backgroundColorToolStripMenuItem});
            this.customizeMenu.Name = "customizeMenu";
            this.customizeMenu.Size = new System.Drawing.Size(249, 176);
            this.customizeMenu.Opening += new System.ComponentModel.CancelEventHandler(this.customizeMenu_Opening);
            // 
            // copyActionToolStripMenuItem
            // 
            this.copyActionToolStripMenuItem.Name = "copyActionToolStripMenuItem";
            this.copyActionToolStripMenuItem.Size = new System.Drawing.Size(248, 22);
            this.copyActionToolStripMenuItem.Text = "Copy Action";
            this.copyActionToolStripMenuItem.Click += new System.EventHandler(this.copyActionToolStripMenuItem_Click);
            // 
            // pasteActionToolStripMenuItem
            // 
            this.pasteActionToolStripMenuItem.Enabled = false;
            this.pasteActionToolStripMenuItem.Name = "pasteActionToolStripMenuItem";
            this.pasteActionToolStripMenuItem.Size = new System.Drawing.Size(248, 22);
            this.pasteActionToolStripMenuItem.Text = "Paste Action";
            this.pasteActionToolStripMenuItem.Click += new System.EventHandler(this.pasteActionToolStripMenuItem_Click);
            // 
            // deleteActionToolStripMenuItem
            // 
            this.deleteActionToolStripMenuItem.Name = "deleteActionToolStripMenuItem";
            this.deleteActionToolStripMenuItem.Size = new System.Drawing.Size(248, 22);
            this.deleteActionToolStripMenuItem.Text = "Delete Action";
            this.deleteActionToolStripMenuItem.Click += new System.EventHandler(this.deleteActionToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(245, 6);
            // 
            // renameActionToolStripMenuItem
            // 
            this.renameActionToolStripMenuItem.Enabled = false;
            this.renameActionToolStripMenuItem.Name = "renameActionToolStripMenuItem";
            this.renameActionToolStripMenuItem.Size = new System.Drawing.Size(248, 22);
            this.renameActionToolStripMenuItem.Text = "Rename Action";
            this.renameActionToolStripMenuItem.Click += new System.EventHandler(this.renameActionToolStripMenuItem_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(245, 6);
            // 
            // equalizeAllSizesAndFootprintsToolStripMenuItem
            // 
            this.equalizeAllSizesAndFootprintsToolStripMenuItem.Name = "equalizeAllSizesAndFootprintsToolStripMenuItem";
            this.equalizeAllSizesAndFootprintsToolStripMenuItem.Size = new System.Drawing.Size(248, 22);
            this.equalizeAllSizesAndFootprintsToolStripMenuItem.Text = "Equalize all sizes and footprints";
            this.equalizeAllSizesAndFootprintsToolStripMenuItem.Click += new System.EventHandler(this.equalizeAllSizesAndFootprintsToolStripMenuItem_Click);
            // 
            // equalizeAllActionsToolStripMenuItem
            // 
            this.equalizeAllActionsToolStripMenuItem.Name = "equalizeAllActionsToolStripMenuItem";
            this.equalizeAllActionsToolStripMenuItem.Size = new System.Drawing.Size(248, 22);
            this.equalizeAllActionsToolStripMenuItem.Text = "Equalize all actions (like selected)";
            this.equalizeAllActionsToolStripMenuItem.Click += new System.EventHandler(this.equalizeAllActionsToolStripMenuItem_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(245, 6);
            // 
            // backgroundColorToolStripMenuItem
            // 
            this.backgroundColorToolStripMenuItem.Name = "backgroundColorToolStripMenuItem";
            this.backgroundColorToolStripMenuItem.Size = new System.Drawing.Size(248, 22);
            this.backgroundColorToolStripMenuItem.Text = "Background color...";
            this.backgroundColorToolStripMenuItem.Click += new System.EventHandler(this.backgroundColorToolStripMenuItem_Click);
            // 
            // stateFrameImage1
            // 
            this.stateFrameImage1.AdaptFirstDrop = false;
            this.stateFrameImage1.AddScripts = true;
            this.stateFrameImage1.AutoSize = true;
            this.stateFrameImage1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.stateFrameImage1.BackgroundColor = System.Drawing.Color.Black;
            this.stateFrameImage1.Data = null;
            this.stateFrameImage1.FrameParts = 2;
            this.stateFrameImage1.Frames = 30;
            this.stateFrameImage1.Hotspot = true;
            this.stateFrameImage1.HotspotCoordScale = 1F;
            this.stateFrameImage1.Location = new System.Drawing.Point(12, 12);
            this.stateFrameImage1.Name = "stateFrameImage1";
            this.stateFrameImage1.PictureBoxSize = new System.Drawing.Size(120, 200);
            this.stateFrameImage1.ScaleImageToBox = false;
            this.stateFrameImage1.ShowAnimationButton = true;
            this.stateFrameImage1.ShowImageNames = true;
            this.stateFrameImage1.ShowStateDropDown = true;
            this.stateFrameImage1.Size = new System.Drawing.Size(339, 424);
            this.stateFrameImage1.TabIndex = 0;
            // 
            // CharacterDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(356, 534);
            this.Controls.Add(this.customize);
            this.Controls.Add(this.panel_down);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.text_color);
            this.Controls.Add(this.stateFrameImage1);
            this.Name = "CharacterDlg";
            this.Text = "CharacterDlg";
            this.panel_down.ResumeLayout(false);
            this.panel_down.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.startzoom)).EndInit();
            this.customizeMenu.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private StateFrameImage stateFrameImage1;
        private System.Windows.Forms.Button text_color;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Panel panel_down;
        private System.Windows.Forms.ComboBox font;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox walksound;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox walkspeed;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.CheckBox nozoom;
        private System.Windows.Forms.NumericUpDown startzoom;
        private System.Windows.Forms.CheckBox ghostmode;
        private System.Windows.Forms.CheckBox memory_resistant;
        private System.Windows.Forms.CheckBox leftanim;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button customize;
        private System.Windows.Forms.ContextMenuStrip customizeMenu;
        private System.Windows.Forms.ToolStripMenuItem copyActionToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pasteActionToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteActionToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem renameActionToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripMenuItem equalizeAllSizesAndFootprintsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem equalizeAllActionsToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripMenuItem backgroundColorToolStripMenuItem;
    }
}