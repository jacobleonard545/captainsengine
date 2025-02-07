using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;
using System.Globalization;

namespace StoryDesigner
{
    public partial class MainForm : ChildForm
    {
        public MainForm(Persistence persistence)
        {
            InitializeComponent();
            mPersistence = persistence;
            this.Location = mPersistence.MainFormPosition;
            this.StartPosition = FormStartPosition.Manual;
            this.WindowState = FormWindowState.Normal;
            mediaPool.NodeMouseDoubleClick += new TreeNodeMouseClickEventHandler(mediaPool_NodeMouseDoubleClick);
            mediaPool.MouseDown += new MouseEventHandler(mediaPool_MouseDown);
            mediaPool.AfterLabelEdit += new NodeLabelEditEventHandler(gamePool_AfterLabelEdit);
            mediaPool.DragOver += new DragEventHandler(mediaPool_DragOver);
            mediaPool.DragDrop += new DragEventHandler(mediaPool_DragDrop);
            gamePool.NodeMouseDoubleClick +=new TreeNodeMouseClickEventHandler(mediaPool_NodeMouseDoubleClick);
            gamePool.MouseDown +=new MouseEventHandler(mediaPool_MouseDown);
            gamePool.AfterLabelEdit += new NodeLabelEditEventHandler(gamePool_AfterLabelEdit);
            gamePool.DragOver += new DragEventHandler(mediaPool_DragOver);
            gamePool.DragDrop += new DragEventHandler(mediaPool_DragDrop);
            this.FormClosing += new FormClosingEventHandler(MainForm_FormClosing);
            newToolStripMenuItem_Click(null, null);
            mSelectedView = gamePool;
            mSelectedNode = gamePool.Nodes[0];
            mBackupTimer.Tick += new EventHandler(mBackupTimer_Tick);
            mBackupTimer.Interval = 1000*60*5;
        }

        void mBackupTimer_Tick(object sender, EventArgs e)
        {
            //write backup
            if (mPersistence.CreateBackups)
            {
                string dir = getBackupFileName();
                AdvFileWriter afw = new AdvFileWriter(mData, gamePool, mediaPool, mZipPwd);
                afw.writeProjectFile(dir);
            }
        }

        string getBackupFileName()
        {
            string dir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "StoryDesigner");
            dir = Path.Combine(dir, "backups");
            if (!Directory.Exists(dir))
                Directory.CreateDirectory(dir);
            DateTime dt = DateTime.Now;
            dir = Path.Combine(dir, dt.Year + " " + DateTimeFormatInfo.GetInstance(null).GetMonthName(dt.Month));
            if (!Directory.Exists(dir))
                Directory.CreateDirectory(dir);
            dir = Path.Combine(dir, dt.Day.ToString());
            if (!Directory.Exists(dir))
                Directory.CreateDirectory(dir);
            //Get number
            DirectoryInfo dirinfo = new DirectoryInfo(dir);
            FileInfo[] files = null;
            int instanceCount = 1;
            try
            {
                files = dirinfo.GetFiles();
                foreach (FileInfo info in files)
                {
                    if (info.Name.ToLower().StartsWith(mData.Settings.Projectname.ToLower()))
                        ++instanceCount;
                }
            }
            catch (Exception)
            {

            }
            string filename = mData.Settings.Projectname + " " + instanceCount + ".adv";
            dir = Path.Combine(dir, filename);
            return dir;
        }

        void mediaPool_DragDrop(object sender, DragEventArgs e)
        {
            Resource from = (Resource)e.Data.GetData(typeof(Resource));
            TreeView view = (TreeView)sender;
            Point p = view.PointToClient(new Point(e.X, e.Y));
            TreeNode node = view.GetNodeAt(p.X, p.Y);
            TreeNode toParent;
            determineTypeAndFolder(view, node, out toParent, true);
            from.Node.Remove();
            toParent.Nodes.Add(from.Node);
            view.Sort();
            view.SelectedNode = from.Node;
        }

        void mediaPool_DragOver(object sender, DragEventArgs e)
        {
            if (!e.Data.GetDataPresent(typeof(Resource)))
                e.Effect = DragDropEffects.None;
            else
            {
                Resource from = (Resource)e.Data.GetData(typeof(Resource));
                TreeView toview = (TreeView)sender;
                Point p = toview.PointToClient(new Point(e.X, e.Y));
                TreeNode node = toview.GetNodeAt(p.X, p.Y);
                TreeNode fromParent;
                ResourceID fromID = determineTypeAndFolder(from.Node.TreeView, from.Node, out fromParent, false);
                TreeNode toParent;
                ResourceID toID = determineTypeAndFolder(toview, node, out toParent, true);
                if (fromID != toID || fromParent == toParent)
                {
                    e.Effect = DragDropEffects.None;
                    return;
                }
                //check if to is under from
                TreeNode curr = node;
                while (curr != null)
                {
                    if (curr == from.Node)
                    {
                        e.Effect = DragDropEffects.None;
                        return;
                    }
                    curr = curr.Parent;
                }
                e.Effect = DragDropEffects.Copy;
            }
        }

        void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (mRoomDlg != null)
                mPersistence.RoomDlgPosition = mRoomDlg.Location;
            if (mObjectDlg != null)
                mPersistence.ObjectDlgPosition = mObjectDlg.Location;
            if (mScriptDlg != null)
                mPersistence.ScriptDlgPosition = mScriptDlg.Location;
            if (mCharacterDlg != null)
                mPersistence.CharacterDlgPosition = mCharacterDlg.Location;
            if (mItemDlg != null)
                mPersistence.ItemDlgPosition = mItemDlg.Location;
            mPersistence.MainFormPosition = this.Location;
            mPersistence.save();
        }

        delegate void sort();

        void gamePool_AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            TreeView view = (TreeView)sender;
            view.LabelEdit = false;
            if (e.Label == null || e.Label.Length <= 0 || e.Node.Parent == null)
            {
                e.CancelEdit = true;
                return;
            }
            e.Node.EndEdit(false);
            ResourceID id = (ResourceID)e.Node.Tag;
            switch (id)
            {
                case ResourceID.ITEM:
                    {
                        Script scr = mData.removeScript(Script.Type.ITEM, e.Node.Text);
                        if (scr != null)
                        {
                            scr.Name = e.Label;
                            mData.addScript(scr);
                        }
                        Item it = mData.removeItem(e.Node.Text);
                        it.Name = e.Label;
                        mData.addItem(it);
                        if (mItemDlg != null && mItemDlg.Item == it)
                            mItemDlg.updateItem();
                    }
                    break;
                case ResourceID.CHARACTER:
                    {
                        AdvCharacter chr = mData.removeCharacter(e.Node.Text);
                        chr.Name = e.Label;
                        mData.addCharacter(chr);
                        if (mCharacterDlg != null && mCharacterDlg.Character == chr)
                            mCharacterDlg.updateCharacter();
                    }
                    break;
                case ResourceID.SCRIPT:
                    {
                        Script scr = mData.removeScript(Script.Type.CUTSCENE, e.Node.Text);
                        scr.Name = e.Label;
                        mData.addScript(scr);
                        if (mScriptDlg != null && mScriptDlg.Script == scr)
                            mScriptDlg.updateScript();
                    }
                    break;
                case ResourceID.OBJECT:
                    {
                        AdvObject obj = mData.removeObject(e.Node.Text);
                        obj.Name = e.Label;
                        mData.addObject(obj);
                        if (mObjectDlg != null && mObjectDlg.Object == obj)
                            mObjectDlg.updateObject();
                    }
                    break;
                case ResourceID.ROOM:
                    {
                        Room rm = mData.getRoom(e.Node.Text);
                        rm.rename(e.Label);
                        if (mRoomDlg != null && mRoomDlg.Room == rm)
                            mRoomDlg.updateRoom();
                    }
                    break;
                case ResourceID.FOLDER:
                    {
                        foreach (TreeNode node in e.Node.Parent.Nodes)
                        {
                            if (e.Label == node.Text)
                            {
                                e.CancelEdit = true;
                                return;
                            }
                        }
                    }
                    break;
            }
            //e.Node.Text = e.Label;
            //view.Sort(); does not work, use BeginInvoke instead
            view.SelectedNode = e.Node;
            view.BeginInvoke(new sort(view.Sort));
        }

        public void showScript(Script.Type type, string name)
        {
            if (mScriptDlg != null)
            {
                mPersistence.ScriptDlgPosition = mScriptDlg.Location;
                mScriptDlg.Close();
            }
            Script scr = mData.getScript(type, name.ToLower());
            if (scr == null)
            {
                scr = new Script(type);
                scr.Name = name;
                mData.addScript(scr);
            }
            mScriptDlg = new ScriptDlg(scr, mData);
            mScriptDlg.MdiParent = this.MdiParent;
            mScriptDlg.StartPosition = FormStartPosition.Manual;
            mScriptDlg.Location = mPersistence.ScriptDlgPosition;
            mScriptDlg.Show();
        }

        void mediaPool_MouseDown(object sender, MouseEventArgs e)
        {
            TreeView pool = (TreeView)sender;
            TreeNode node = pool.GetNodeAt(e.Location);
            if (node == null)
                return;
            mSelectedView = pool;
            mSelectedNode = node;
            if (e.Button == MouseButtons.Right)
            {
                ResourceID resID = (ResourceID)node.Tag;
                //Rename
                menuPool.Items[0].Visible = node.Parent != null && (pool != mediaPool || resID == ResourceID.FOLDER);
                //Duplicate
                menuPool.Items[2].Visible = pool != mediaPool && resID != ResourceID.FOLDER;
                //Delete
                menuPool.Items[3].Visible = node.Parent != null;
                //Export
                menuPool.Items[4].Visible = resID == ResourceID.CHARACTER || resID == ResourceID.ROOM;
                
                menuPool.Show(pool, e.X, e.Y);
            }
            if (e.Clicks > 1)
            {
                TreeNodeMouseClickEventArgs args = new TreeNodeMouseClickEventArgs(node, e.Button, e.Clicks, e.X, e.Y);
                mediaPool_NodeMouseDoubleClick(sender, args);
                return;
            }
            pool.SelectedNode = node;
            Resource res = new Resource(node);
            pool.DoDragDrop(res, DragDropEffects.Copy);
        }

        void mediaPool_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            ResourceID id = (ResourceID)e.Node.Tag;
            string name = e.Node.Text.ToLower();
            openResource(id, name);
        }

        private void openResource(ResourceID id, string name){
            switch (id)
            {
                case ResourceID.IMAGE:
                    System.Drawing.Bitmap bmp = mData.getImage(name);
                    if (bmp == null)
                    {
                        MessageBox.Show(Strings.cannotFindImage + name);
                        return;
                    }
                    mImageViewer = new ImageViewer(bmp);
                    mImageViewer.MdiParent = this.MdiParent;
                    mImageViewer.Show();
                    break;
                case ResourceID.SOUND:
                    string sound = mData.Sounds[name];
                    MediaPlyr plyr = new MediaPlyr(sound);
                    plyr.MdiParent = this.MdiParent;
                    plyr.Show();
                    break;
                case ResourceID.MUSIC:
                    string music = mData.Music[name];
                    MediaPlyr plyr2 = new MediaPlyr(music);
                    plyr2.MdiParent = this.MdiParent;
                    plyr2.Show();
                    break;
                case ResourceID.ITEM:
                    Item it = mData.getItem(name);
                    if (it == null)
                    {
                        MessageBox.Show("Cannot find item " + name);
                        return;
                    }
                    if (mItemDlg != null)
                    {
                        if (mItemDlg.Item == it && mItemDlg.Visible)
                        {
                            mItemDlg.Show();
                            break;
                        }
                        mPersistence.ItemDlgPosition = mItemDlg.Location;
                        mItemDlg.Close();
                    }
                    mItemDlg = new ItemDlg(it, this);
                    mItemDlg.MdiParent = this.MdiParent;
                    mItemDlg.StartPosition = FormStartPosition.Manual;
                    mItemDlg.Location = mPersistence.ItemDlgPosition;
                    mItemDlg.Show();
                    break;
                case ResourceID.OBJECT:
                    showObject(name);
                    break;
                case ResourceID.CHARACTER:
                    showCharacter(name);
                    break;
                case ResourceID.ROOM:
                    Room room = mData.getRoom(name);
                    if (room == null)
                    {
                        MessageBox.Show("Cannot find room " + name);
                        return;
                    }
                    Point pos = mPersistence.RoomDlgPosition;
                    if (mRoomDlg != null)
                    {
                        if (mRoomDlg.Room == room && mRoomDlg.Visible)
                        {
                            mRoomDlg.Show();
                            break;
                        }
                        mPersistence.RoomDlgPosition = mRoomDlg.Location;
                        mRoomDlg.Close();
                    }
                    mRoomDlg = new RoomDlg(room, mData, mRoomView, this, this.MdiParent);
                    mRoomDlg.MdiParent = this.MdiParent;
                    mRoomDlg.StartPosition = FormStartPosition.Manual;
                    mRoomDlg.Location = pos;
                    mRoomDlg.Show();
                    break;
                case ResourceID.SCRIPT:
                    showScript(Script.Type.CUTSCENE, name);
                    break;
                case ResourceID.FOLDER:
                    break;
                default:
                    Console.WriteLine("Clicked " + name + " " + id + " unhandled");
                    break;
            }
        }

        public void showObject(string name)
        {
            AdvObject obj = mData.getObject(name);
            if (obj == null)
            {
                MessageBox.Show("Cannot find object " + name);
                return;
            }
            if (mObjectDlg != null)
            {
                if (mObjectDlg.Object == obj && mObjectDlg.Visible)
                {
                    mObjectDlg.Show();
                    return;
                }
                mPersistence.ObjectDlgPosition = mObjectDlg.Location;
                mObjectDlg.Close();
            }
            mObjectDlg = new ObjectDlg(obj, mPersistence.FitImages);
            mObjectDlg.MdiParent = this.MdiParent;
            mObjectDlg.StartPosition = FormStartPosition.Manual;
            mObjectDlg.Location = mPersistence.ObjectDlgPosition;
            mObjectDlg.Show();
        }

        public void showCharacter(string name)
        {
            AdvCharacter chr = mData.getCharacter(name);
            if (chr == null)
            {
                MessageBox.Show("Cannot find character " + name);
                return;
            }
            if (mCharacterDlg != null)
            {
                if (mCharacterDlg.Character == chr && mCharacterDlg.Visible)
                {
                    mCharacterDlg.Show();
                    return;
                }
                mPersistence.CharacterDlgPosition = mCharacterDlg.Location;
                mCharacterDlg.Close();
            }
            mCharacterDlg = new CharacterDlg(chr, mData);
            mCharacterDlg.MdiParent = this.MdiParent;
            mCharacterDlg.StartPosition = FormStartPosition.Manual;
            mCharacterDlg.Location = mPersistence.CharacterDlgPosition;
            mCharacterDlg.Show();
        }

        private void menuStrip1_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {

        }

        private void beendenToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        internal void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Adventure project file|*.adv|Adventure game file|game.dat";
            fod.InitialDirectory = mPersistence.LastOpenPath;
            DialogResult dr = fod.ShowDialog();
            if (dr == DialogResult.OK)
            {
                string dir = Path.GetDirectoryName(fod.FileName);
                mPersistence.LastOpenPath = dir;
                newToolStripMenuItem_Click(null, null);
                loadFile(fod.FileName);
            }
        }

        public bool loadFile(string filename)
        {
            try
            {
                AdvFileReader reader = new AdvFileReader(filename, mediaPool, gamePool, mPersistence);
                mData = reader.Data;
                mZipPwd = reader.ZipPassword;
                if (Path.GetExtension(filename) == ".adv")
                {
                    mSavePath = filename;
                    mBackupTimer.Start();

                    allowSave(true);

                    //check if media path is ok
                    Dictionary<string, string> missingImages = new Dictionary<string, string>();
                    foreach (KeyValuePair<string, string> pair in mData.Images)
                    {
                        if (!File.Exists(pair.Value))
                            missingImages.Add(pair.Key, Path.GetFileName(pair.Value));
                    }
                    Dictionary<string, string> missingSounds = new Dictionary<string, string>();
                    foreach (KeyValuePair<string, string> pair in mData.Sounds)
                    {
                        if (!File.Exists(pair.Value))
                            missingSounds.Add(pair.Key, Path.GetFileName(pair.Value));
                    }
                    Dictionary<string, string> missingMusic = new Dictionary<string, string>();
                    foreach (KeyValuePair<string, string> pair in mData.Music)
                    {
                        if (!File.Exists(pair.Value))
                            missingMusic.Add(pair.Key, Path.GetFileName(pair.Value));
                    }
                    Dictionary<string, string> missingVideos = new Dictionary<string, string>();
                    foreach (KeyValuePair<string, string> pair in mData.Videos)
                    {
                        if (!File.Exists(pair.Value))
                            missingVideos.Add(pair.Key, Path.GetFileName(pair.Value));
                    }
                    if (missingImages.Count == 0 && missingSounds.Count == 0 && missingMusic.Count == 0 && missingVideos.Count == 0)
                        return true;
                    if (!resolveMedia(Path.GetDirectoryName(filename), ref missingImages, ref missingSounds, ref missingMusic, ref missingVideos))
                    {
                        MessageBox.Show("Not all media files were found. Project cannot work correctly.", Strings.error);
                        return false;
                    }
                }
                else
                    allowSave(false);
                return true;
            }
            catch (Exception e)
            {
                MessageBox.Show("Cannot load " + filename + e.ToString());
                return false;
            }
        }

        bool resolveMedia(string searchpath, ref Dictionary<string, string> images, ref Dictionary<string, string> sounds, ref Dictionary<string, string> music, ref Dictionary<string, string> videos)
        {
            DirectoryInfo dir = new DirectoryInfo(searchpath);
            FileInfo[] files = null;
            try
            {
                files = dir.GetFiles();
            }
            catch (Exception)
            {

            }
            DirectoryInfo[] subdirs = dir.GetDirectories();
            foreach (DirectoryInfo subdir in subdirs)
            {
                if (resolveMedia(subdir.FullName, ref images, ref sounds, ref music, ref videos))
                    return true;
            }
            foreach (FileInfo file in files)
            {
                Dictionary<string, string> stillMissing = new Dictionary<string, string>();
                foreach (KeyValuePair<string, string> pair in images)
                {
                    if (file.Name == pair.Value)
                    {
                        mData.Images[pair.Key] = file.FullName;
                    }
                    else
                    {
                        stillMissing[pair.Key] = pair.Value;
                    }
                }
                images = stillMissing;
                stillMissing = new Dictionary<string, string>();
                foreach (KeyValuePair<string, string> pair in sounds)
                {
                    if (file.Name == pair.Value)
                    {
                        mData.Sounds[pair.Key] = file.FullName;
                    }
                    else
                    {
                        stillMissing[pair.Key] = pair.Value;
                    }
                }
                sounds = stillMissing;
                stillMissing = new Dictionary<string, string>();
                foreach (KeyValuePair<string, string> pair in music)
                {
                    if (file.Name == pair.Value)
                    {
                        mData.Music[pair.Key] = file.FullName;
                    }
                    else
                    {
                        stillMissing[pair.Key] = pair.Value;
                    }
                }
                music = stillMissing;
                stillMissing = new Dictionary<string, string>();
                foreach (KeyValuePair<string, string> pair in videos)
                {
                    if (file.Name == pair.Value)
                    {
                        mData.Videos[pair.Key] = file.FullName;
                    }
                    else
                    {
                        stillMissing[pair.Key] = pair.Value;
                    }
                }
                videos = stillMissing;
            }
            return images.Count == 0 && sounds.Count == 0 && music.Count == 0 && videos.Count == 0;
        }

        void allowSave(bool allow)
        {
            ToolStripDropDownItem project = MainMenuStrip.Items[0] as ToolStripDropDownItem;
            ToolStripMenuItem save = project.DropDownItems[2] as ToolStripMenuItem;
            save.Enabled = allow;
            ToolStripMenuItem saveAs = project.DropDownItems[3] as ToolStripMenuItem;
            saveAs.Enabled = allow;
        }

        private AdvData mData;
        private ImageViewer mImageViewer;
        private MouseIcons mMouseIcons;
        private ItemDlg mItemDlg;
        private ObjectDlg mObjectDlg;
        private CharacterDlg mCharacterDlg;
        private ScriptDlg mScriptDlg;
        private RoomDlg mRoomDlg;
        private RoomDlg.ViewMode mRoomView = RoomDlg.ViewMode.Objects;
        private TreeView mSelectedView;
        private TreeNode mSelectedNode;
        private Persistence mPersistence;
        private string mSavePath = null;
        private Timer mBackupTimer = new Timer();
        private string mZipPwd = "";

        internal void projectSetupToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Settings settings = new Settings(mData);
            settings.ShowDialog(this);
        }

        internal void commandsAndBoolsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CommandsAndBools cabs = new CommandsAndBools(mData);
            cabs.ShowDialog(this);
        }

        internal void fontsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Fonts fnts = new Fonts(mData);
            fnts.ShowDialog(this);
        }

        internal void mouseiconsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (mMouseIcons != null)
                mMouseIcons.Close();
            mMouseIcons = new MouseIcons(mData);
            mMouseIcons.MdiParent = this.MdiParent;
            mMouseIcons.Show();
        }

        internal void imageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Image files|*.jpg;*.jpeg;*.png;*.bmp,*.tga;*.gif";
            fod.InitialDirectory = mPersistence.ImageLoadPath;
            fod.Multiselect = true;
            if (fod.ShowDialog() == DialogResult.OK)
            {
                string path = System.IO.Path.GetDirectoryName(fod.FileName);
                mPersistence.ImageLoadPath = path;
                foreach (string filename in fod.FileNames)
                {
                    string file = System.IO.Path.GetFileNameWithoutExtension(filename);
                    try
                    {
                        mData.Images.Add(file.ToLower(), filename);
                        if (mData.Persistence.CropTransparentImages)
                        {
                            Bitmap bmp = (Bitmap)Bitmap.FromFile(filename);
                            Bitmap result = Utilities.cropTransparent(bmp);
                            bmp.Dispose();
                            result.Save(filename);
                            result.Dispose();
                        }
                    }
                    catch (ArgumentException)
                    {
                        MessageBox.Show("Image with same name already added");
                        continue;
                    }
                    TreeNode parent;
                    ResourceID res = determineTypeAndFolder(mediaPool, out parent);
                    if (res != ResourceID.IMAGE)
                        parent = mediaPool.Nodes[0];
                    TreeNode newnode = new TreeNode(file);
                    newnode.Tag = ResourceID.IMAGE;
                    parent.Nodes.Add(newnode);
                }
            }
        }

        internal void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mBackupTimer.Stop();
            if (MainMenuStrip != null)
                allowSave(true);
            mData = new AdvData(mPersistence);
            mZipPwd = "";
            gamePool.Nodes.Clear();
            gamePool.Nodes.Add("1) "+Strings.Characters.ToUpper());
            gamePool.Nodes.Add("2) "+Strings.Scripts.ToUpper());
            gamePool.Nodes.Add("3) "+Strings.Items.ToUpper());
            gamePool.Nodes.Add("4) "+Strings.Objects.ToUpper());
            gamePool.Nodes.Add("5) "+Strings.Rooms.ToUpper());
            foreach (TreeNode node in gamePool.Nodes)
            {
                node.Tag = ResourceID.FOLDER;
                node.SelectedImageIndex = Utilities.ResourceIDToImageIndex(ResourceID.FOLDER);
                node.ImageIndex = node.SelectedImageIndex;
            }
            mediaPool.Nodes.Clear();
            mediaPool.Nodes.Add(Strings.Images.ToUpper());
            mediaPool.Nodes.Add(Strings.Music.ToUpper());
            mediaPool.Nodes.Add(Strings.Sounds.ToUpper());
            mediaPool.Nodes.Add(Strings.Videos.ToUpper());
            foreach (TreeNode node in mediaPool.Nodes)
            {
                node.Tag = ResourceID.FOLDER;
                node.SelectedImageIndex = Utilities.ResourceIDToImageIndex(ResourceID.FOLDER);
                node.ImageIndex = node.SelectedImageIndex;
            }
            mSavePath = null;
        }

        internal void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sod = new SaveFileDialog();
            sod.Filter = "Adventure project files|*.adv";
            sod.InitialDirectory = mPersistence.LastOpenPath;
            sod.FileName = mData.Settings.Projectname;
            DialogResult dr = sod.ShowDialog();
            if (dr == DialogResult.OK)
            {
                mPersistence.LastOpenPath = Path.GetDirectoryName(sod.FileName);
                AdvFileWriter afw = new AdvFileWriter(mData, gamePool, mediaPool, mZipPwd);
                afw.writeProjectFile(sod.FileName);
                mSavePath = sod.FileName;
                mZipPwd = afw.ZipPassword;
                if (!mBackupTimer.Enabled)
                    mBackupTimer.Start();
            }
        }

        WaitForm m_wait;
        string m_runtime_name;

        void showWhile(System.Threading.ThreadStart method)
        {
            Application.UseWaitCursor = true;
            m_wait = new WaitForm();
            m_wait.Show(this);
            MethodInvoker callback = delegate { 
                Application.UseWaitCursor = false;
                m_wait.Dispose();
                DialogResult start = MessageBox.Show(Strings.gameCreatedUnder + m_runtime_name + Strings.startNow, Strings.confirmation, MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (start == DialogResult.Yes)
                {
                    ProcessStartInfo info = new ProcessStartInfo(m_runtime_name);
                    info.WorkingDirectory = Path.GetDirectoryName(m_runtime_name);
                    Process.Start(info);
                }

            };
            AsyncCallback invoke = delegate { this.Invoke(callback); };
            method.BeginInvoke(invoke, null);
        }

        internal void createGameToolStripMenuItem_Click(object sender, EventArgs e)
        {
            bool showSave = true;
            SaveFileDialog sod = new SaveFileDialog();
            if (m_runtime_name != null && m_runtime_name.Length > 0)
            {
                sod.FileName = m_runtime_name;
                DialogResult update = MessageBox.Show(Strings.updatePrevious, Strings.confirmation, MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (update == DialogResult.Yes)
                    showSave = false;
            }
            else
            {
                sod.FileName = mData.Settings.Projectname;
            }
            sod.Filter = "Adventure game|*.exe";
            sod.InitialDirectory = mData.Settings.Directory;
            bool create = !showSave;
            if (showSave)
            {
                create = sod.ShowDialog() == DialogResult.OK;
            }
            if (create)
            {
                m_runtime_name = sod.FileName;
                showWhile(delegate
                {
                    AdvFileWriter writer = new AdvFileWriter(mData, gamePool, mediaPool, mZipPwd);
                    writer.writeGame(sod.FileName);
                    mData.Settings.Directory = Path.GetDirectoryName(sod.FileName);
                    mZipPwd = writer.ZipPassword;
                });
            }
        }

        public void setRoomViewMode(RoomDlg.ViewMode mode)
        {
            int num = (int)mode;
            ToolStripDropDownItem setup = MainMenuStrip.Items[2] as ToolStripDropDownItem;
            string[] names = { Strings.Walkmap, Strings.Deepmap, Strings.Inventoryfield, Strings.FXShapes };
            for (int i = 0; i < 4; ++i)
            {
                ToolStripMenuItem item = setup.DropDownItems[i + 6] as ToolStripMenuItem;
                item.Text = names[i] + " "+ ((i+1) == num ? Strings.off : Strings.on);
            }
            mRoomView = mode;
            if (mRoomDlg != null)
                mRoomDlg.Viewmode = mRoomView;
        }

        internal void walkmapOnToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (mRoomView != RoomDlg.ViewMode.Walkmap)
            {
                setRoomViewMode(RoomDlg.ViewMode.Walkmap);
            }
            else
            {
                setRoomViewMode(RoomDlg.ViewMode.Objects);
            }
        }

        private void gamepool_add_Click(object sender, EventArgs e)
        {
            TreeNode parent;
            ResourceID id = determineTypeAndFolder(mSelectedView, out parent);
            switch (id)
            {
                case ResourceID.ITEM:
                    itemToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.CHARACTER:
                    characterToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.SCRIPT:
                    scriptToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.OBJECT:
                    objectToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.ROOM:
                    roomToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.IMAGE:
                    imageToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.SOUND:
                    soundeffectToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.MUSIC:
                    musicfileToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.VIDEO:
                    videoToolStripMenuItem_Click(null, null);
                    break;
            }
        }

        private ResourceID determineTypeAndFolder(TreeView view, out TreeNode parent)
        {
            TreeNode node = view.SelectedNode;
            return determineTypeAndFolder(view, node, out parent, true);
        }

        private ResourceID determineTypeAndFolder(TreeView view, TreeNode node, out TreeNode parent, bool folderIsParentHimself)
        {
            if (node == null)
                node = view.Nodes[0];

            parent = node;
            if ((ResourceID)parent.Tag != ResourceID.FOLDER || !folderIsParentHimself)
                parent = node.Parent;

            while (node.Parent != null)
                node = node.Parent;
            
            if (view == gamePool)
            {
                if (node == gamePool.Nodes[0])
                    return ResourceID.CHARACTER;
                else if (node == gamePool.Nodes[1])
                    return ResourceID.SCRIPT;
                else if (node == gamePool.Nodes[2])
                    return ResourceID.ITEM;
                else if (node == gamePool.Nodes[3])
                    return ResourceID.OBJECT;
                else if (node == gamePool.Nodes[4])
                    return ResourceID.ROOM;
            }
            else if (view == mediaPool)
            {
                if (node == mediaPool.Nodes[0])
                    return ResourceID.IMAGE;
                else if (node == mediaPool.Nodes[1])
                    return ResourceID.MUSIC;
                else if (node == mediaPool.Nodes[2])
                    return ResourceID.SOUND;
                else if (node == mediaPool.Nodes[3])
                    return ResourceID.VIDEO;
            }
            return ResourceID.INVALID;
        }

        private void renameToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mSelectedView.LabelEdit = true;
            mSelectedNode.BeginEdit();
        }

        internal void itemToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Item it = new Item(mData, 10);
            addItem(it);
        }

        private void addItem(Item it){
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(gamePool, out parent);
            if (res != ResourceID.ITEM)
                parent = gamePool.Nodes[2];
            mData.addItem(it);
            TreeNode node = new TreeNode(it.Name);
            node.Tag = ResourceID.ITEM;
            parent.Nodes.Add(node);
            gamePool.Sort();
            gamePool.SelectedNode = node;
            openResource(ResourceID.ITEM, it.Name);
            gamePool.LabelEdit = true;
            node.BeginEdit();
        }

        internal void characterToolStripMenuItem_Click(object sender, EventArgs e)
        {       
            AdvCharacter chr = new AdvCharacter(mData, 36);
            addCharacter(chr);
        }

        private void addCharacter(AdvCharacter chr)
        {
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(gamePool, out parent);
            if (res != ResourceID.CHARACTER)
                parent = gamePool.Nodes[0];
            mData.addCharacter(chr);
            TreeNode node = new TreeNode(chr.Name);
            node.Tag = ResourceID.CHARACTER;
            parent.Nodes.Add(node);
            gamePool.Sort();
            gamePool.SelectedNode = node;
            openResource(ResourceID.CHARACTER, chr.Name);
            gamePool.LabelEdit = true;
            node.BeginEdit();
        }

        internal void scriptToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Script scr = new Script(Script.Type.CUTSCENE);
            scr.Name = "Script" + (mData.NumCutscenes + 1);
            addScript(scr);
        }

        private void addScript(Script scr)
        {
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(gamePool, out parent);
            if (res != ResourceID.SCRIPT)
                parent = gamePool.Nodes[1];
            mData.addScript(scr);
            TreeNode node = new TreeNode(scr.Name);
            node.Tag = ResourceID.SCRIPT;
            parent.Nodes.Add(node);
            gamePool.Sort();
            gamePool.SelectedNode = node;
            openResource(ResourceID.SCRIPT, scr.Name);
            gamePool.LabelEdit = true;
            node.BeginEdit();
        }

        internal void objectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AdvObject obj = new AdvObject(mData, 10);
            addObject(obj);
        }

        private void addObject(AdvObject obj){
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(gamePool, out parent);
            if (res != ResourceID.OBJECT)
                parent = gamePool.Nodes[3];
            mData.addObject(obj);
            TreeNode node = new TreeNode(obj.Name);
            node.Tag = ResourceID.OBJECT;
            parent.Nodes.Add(node);
            gamePool.Sort();
            gamePool.SelectedNode = node;
            //open object window
            openResource(ResourceID.OBJECT, obj.Name);
            //edit name
            gamePool.LabelEdit = true;
            node.BeginEdit();
        }

        internal void roomToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Room rm = new Room(mData);
            addRoom(rm);
        }

        private void addRoom(Room rm){
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(gamePool, out parent);
            if (res != ResourceID.ROOM)
                parent = gamePool.Nodes[4];
            mData.addRoom(rm);
            TreeNode node = new TreeNode(rm.Name);
            node.Tag = ResourceID.ROOM;
            parent.Nodes.Add(node);
            gamePool.Sort();
            gamePool.SelectedNode = node;
            openResource(ResourceID.ROOM, rm.Name);
            gamePool.LabelEdit = true;
            node.BeginEdit();
        }

        private void newSubdirectoryToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(mSelectedView, out parent);
            if (res == ResourceID.FOLDER)
                parent = mSelectedView.SelectedNode;
            int count = 0;
            foreach (TreeNode n in parent.Nodes)
            {
                if ((ResourceID)n.Tag == ResourceID.FOLDER)
                    ++count;
            }
            TreeNode node = new TreeNode("Folder"+(count+1));
            node.Tag = ResourceID.FOLDER;
            node.SelectedImageIndex = Utilities.ResourceIDToImageIndex(ResourceID.FOLDER);
            node.ImageIndex = node.SelectedImageIndex;
            parent.Nodes.Add(node);
            mSelectedView.Sort();
            mSelectedView.SelectedNode = node;
            mSelectedView.LabelEdit = true;
            node.BeginEdit();
        }

        internal void soundeffectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Sound files|*.wav;*.ogg;*.mp3;*.mp2;*.mp1";
            fod.InitialDirectory = mPersistence.SoundLoadPath;
            fod.Multiselect = true;
            if (fod.ShowDialog() == DialogResult.OK)
            {
                string path = System.IO.Path.GetDirectoryName(fod.FileName);
                mPersistence.SoundLoadPath = path;
                foreach (string filename in fod.FileNames)
                {
                    string file = System.IO.Path.GetFileNameWithoutExtension(filename);
                    try
                    {
                        mData.Sounds.Add(file.ToLower(), filename);
                    }
                    catch (ArgumentException)
                    {
                        MessageBox.Show("Sound effect with same name already added");
                        return;
                    }
                    TreeNode parent;
                    ResourceID res = determineTypeAndFolder(mediaPool, out parent);
                    if (res != ResourceID.SOUND)
                        parent = mediaPool.Nodes[2];
                    TreeNode newnode = new TreeNode(file);
                    newnode.Tag = ResourceID.SOUND;
                    parent.Nodes.Add(newnode);
                }
            }
        }

        internal void musicfileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Music files|*.wav;*.ogg;*.mp3;*.mp2;*.mp1;*.it;*.xm;*.mod;*.s3m;*.mo3;*.umx";
            fod.InitialDirectory = mPersistence.MusicLoadPath;
            fod.Multiselect = true;

            if (fod.ShowDialog() == DialogResult.OK)
            {
                string path = System.IO.Path.GetDirectoryName(fod.FileName);
                mPersistence.MusicLoadPath = path;

                foreach (string filename in fod.FileNames)
                {
                    string file = System.IO.Path.GetFileNameWithoutExtension(filename);
                    try
                    {
                        mData.Music.Add(file.ToLower(), filename);
                    }
                    catch (ArgumentException)
                    {
                        MessageBox.Show("Music with same name already added");
                        return;
                    }
                    TreeNode parent;
                    ResourceID res = determineTypeAndFolder(mediaPool, out parent);
                    if (res != ResourceID.MUSIC)
                        parent = mediaPool.Nodes[1];
                    TreeNode newnode = new TreeNode(file);
                    newnode.Tag = ResourceID.MUSIC;
                    parent.Nodes.Add(newnode);
                }
            }
        }

        internal void videoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Video files|*.avi;*.swf;*.mpeg;*.mpg;*.wmv";
            fod.InitialDirectory = mPersistence.VideoLoadPath;
            fod.Multiselect = true;

            if (fod.ShowDialog() == DialogResult.OK)
            {
                string path = System.IO.Path.GetDirectoryName(fod.FileName);
                mPersistence.VideoLoadPath = path;

                foreach (string filename in fod.FileNames)
                {
                    string file = System.IO.Path.GetFileNameWithoutExtension(filename);
                    try
                    {
                        mData.Videos.Add(file.ToLower(), filename);
                    }
                    catch (ArgumentException)
                    {
                        MessageBox.Show("Video with same name already added");
                        return;
                    }
                    TreeNode parent;
                    ResourceID res = determineTypeAndFolder(mediaPool, out parent);
                    if (res != ResourceID.VIDEO)
                        parent = mediaPool.Nodes[3];
                    TreeNode newnode = new TreeNode(file);
                    newnode.Tag = ResourceID.VIDEO;
                    parent.Nodes.Add(newnode);
                }
            }
        }

        internal void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (mSavePath != null)
            {
                if (mPersistence.CreateBackups)
                {
                    string backup = getBackupFileName();
                    File.Copy(mSavePath, backup);
                    mBackupTimer.Stop();
                    mBackupTimer.Start();
                }
                AdvFileWriter afw = new AdvFileWriter(mData, gamePool, mediaPool, mZipPwd);
                afw.writeProjectFile(mSavePath);
                mZipPwd = afw.ZipPassword;
            }
            else //open save as
                saveAsToolStripMenuItem_Click(sender, e);
        }

        internal void deepmapOnToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (mRoomView != RoomDlg.ViewMode.Deepmap)
            {
                setRoomViewMode(RoomDlg.ViewMode.Deepmap);
            }
            else
            {
                setRoomViewMode(RoomDlg.ViewMode.Objects);
            }
        }

        internal void inventoryfieldOnToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (mRoomView != RoomDlg.ViewMode.Inventory)
            {
                setRoomViewMode(RoomDlg.ViewMode.Inventory);
            }
            else
            {
                setRoomViewMode(RoomDlg.ViewMode.Objects);
            }
        }

        internal void fXShapesOnToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (mRoomView != RoomDlg.ViewMode.Specialfx)
            {
                setRoomViewMode(RoomDlg.ViewMode.Specialfx);
            }
            else
            {
                setRoomViewMode(RoomDlg.ViewMode.Objects);
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            mSelectedView.ExpandAll();
        }

        private void hideAll_Click(object sender, EventArgs e)
        {
            mSelectedView.CollapseAll();
        }

        private void gp_newFolder_Click(object sender, EventArgs e)
        {
            newSubdirectoryToolStripMenuItem_Click(null, null);
        }

        private void gamepool_delete_Click(object sender, EventArgs e)
        {
            if (mSelectedView.SelectedNode.Parent == null)
                return; //do not delete root folders
            DialogResult result = DialogResult.Yes;
            if (!mData.Settings.SilentDelete)
            {
                string entry = mSelectedView.SelectedNode.Text;
                string text;
                ResourceID res = (ResourceID)mSelectedView.SelectedNode.Tag;
                if (res == ResourceID.FOLDER)
                    text = "Delete subdirectory " + entry + " and all its entries?";
                else
                    text = "Delete " + entry + "?";
                result = MessageBox.Show(text, Strings.confirmation, MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            }
            if (result == DialogResult.Yes)
            {
                removeResource(mSelectedView.SelectedNode);
                mSelectedView.SelectedNode.Remove();
            }
        }

        private void removeResource(TreeNode node){
            ResourceID res = (ResourceID)node.Tag;
            string entry = node.Text;
            switch (res)
            {
                case ResourceID.IMAGE:
                    mData.Images.Remove(entry);
                    break;
                case ResourceID.SOUND:
                    mData.Sounds.Remove(entry);
                    break;
                case ResourceID.MUSIC:
                    mData.Music.Remove(entry);
                    break;
                case ResourceID.VIDEO:
                    mData.Videos.Remove(entry);
                    break;
                case ResourceID.ITEM:
                    mData.removeItem(entry);
                    break;
                case ResourceID.CHARACTER:
                    mData.removeCharacter(entry);
                    break;
                case ResourceID.OBJECT:
                    mData.removeObject(entry);
                    break;
                case ResourceID.ROOM:
                    mData.removeRoom(entry);
                    break;
                case ResourceID.SCRIPT:
                    mData.removeScript(Script.Type.CUTSCENE, entry);
                    break;
                case ResourceID.FOLDER:
                    foreach (TreeNode child in node.Nodes)
                    {
                        removeResource(child);
                    }
                    break;
            }
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            gamepool_delete_Click(null, null);
        }

        private static string findFreeName<T>(Dictionary<string, T> dict, string basename) where T: class
        {
            int count = 1;
            while (true)
            {
                string test = basename + count;
                if (!dict.ContainsKey(test))
                    return test;
                ++count;
            }
        }

        private void duplicateToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode node = mSelectedView.SelectedNode;
            switch ((ResourceID)node.Tag)
            {
                case ResourceID.CHARACTER:
                    AdvCharacter chr = mData.getCharacter(node.Text);
                    string newname = findFreeName(mData.Characters, chr.Name);
                    AdvCharacter newchr = chr.duplicate(newname);
                    addCharacter(newchr);
                    break;
                case ResourceID.SCRIPT:
                    Script scr = mData.getScript(Script.Type.CUTSCENE, node.Text);
                    newname = findFreeName(mData.getScripts(Script.Type.CUTSCENE), scr.Name);
                    Script newscr = scr.duplicate(newname);
                    addScript(newscr);
                    break;
                case ResourceID.ITEM:
                    Item it = mData.getItem(node.Text);
                    newname = findFreeName(mData.Items, it.Name);
                    Item newitem = it.duplicate(newname);
                    addItem(newitem);
                    break;
                case ResourceID.OBJECT:
                    AdvObject obj = mData.getObject(node.Text);
                    newname = findFreeName(mData.Objects, obj.Name);
                    AdvObject newobject = obj.duplicate(newname);
                    addObject(newobject);
                    break;
                case ResourceID.ROOM:
                    Room rm = mData.getRoom(node.Text);
                    newname = findFreeName(mData.Rooms, rm.Name);
                    Room newroom = rm.duplicate(newname);
                    addRoom(newroom);
                    break;
            }
        }

        internal void languageAndSoundToolsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mData.updateLanguageList();
            LanguageDlg dlg = new LanguageDlg(mData);
            dlg.Show(this);
        }

        internal void importToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Adventure export file|*.adz";
            fod.InitialDirectory = mPersistence.LastOpenPath;
            DialogResult dr = fod.ShowDialog();
            if (dr == DialogResult.OK)
            {
                string dir = Path.GetDirectoryName(fod.FileName);
                mPersistence.LastOpenPath = dir;
                string advfile;
                try
                {
                    advfile = AdvFileReader.unpackAdz(fod.FileName);
                }
                catch (Exception)
                {
                    FolderBrowserDialog fbd = new FolderBrowserDialog();
                    fbd.Description = mData.getLocalizedString("ChooseDestination");
                    fbd.SelectedPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
                    DialogResult res = fbd.ShowDialog(this);
                    if (res != DialogResult.OK)
                        return;
                    advfile = AdvFileReader.unpackAdz(fod.FileName, fbd.SelectedPath);

                }
                newToolStripMenuItem_Click(null, null);
                if (!loadFile(advfile))
                {
                    
                }
                else
                {
                    string import = mData.getLocalizedString("importAdz");
                    import = String.Format(import, Path.GetDirectoryName(advfile));
                    MessageBox.Show(import, "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            }
        }

        internal void exportToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "Adventure export file|*.adz";
            sfd.InitialDirectory = mPersistence.LastOpenPath;
            DialogResult dr = sfd.ShowDialog();
            if (dr == DialogResult.OK)
            {
                AdvFileWriter afw = new AdvFileWriter(mData, gamePool, mediaPool, mZipPwd);
                afw.writeExport(sfd.FileName);
            }
        }

        internal void importCharacterToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Character export file|*.adc";
            fod.InitialDirectory = mPersistence.LastOpenPath;
            DialogResult dr = fod.ShowDialog();
            if (dr == DialogResult.OK)
            {
                string savepath = "";
                if (mSavePath != null)
                    savepath = Path.GetDirectoryName(mSavePath);
                AdvFileReader rdr = new AdvFileReader(mData, mediaPool, gamePool, mPersistence, savepath);
                rdr.importCharacter(fod.FileName);
                MessageBox.Show(Strings.done, "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        internal void importRoomToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Room export file|*.adr";
            fod.InitialDirectory = mPersistence.LastOpenPath;
            DialogResult dr = fod.ShowDialog();
            if (dr == DialogResult.OK)
            {
                string savepath = "";
                if (mSavePath != null)
                    savepath = Path.GetDirectoryName(mSavePath);
                AdvFileReader rdr = new AdvFileReader(mData, mediaPool, gamePool, mPersistence, savepath);
                rdr.importRoom(fod.FileName);
                MessageBox.Show(Strings.done, "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        internal AdvData Data
        {
            get { return mData; }
        }
    }
}