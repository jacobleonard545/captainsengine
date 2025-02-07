using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Drawing;
using ICSharpCode.SharpZipLib.Zip;

namespace StoryDesigner
{
    public class AdvFileReader
    {
        protected readonly float FPS_MAX = 50.0f;
        protected readonly int STATES_MAX = 10;
        protected readonly int CHAR_STATES_MAX = 36;
        protected readonly int FRAMES_MAX = 25;
        protected readonly int FRAMES2_MAX = 30;
        protected readonly int PARTS_MAX = 2;
        protected readonly int FXSHAPES_MAX = 3;

        public AdvFileReader(Persistence persistence)
        {
            mIsAdv = true;
        }

        public AdvFileReader(AdvData data, TreeView mediapool, TreeView gamepool, Persistence persistence, string path)
        {
            ZipConstants.DefaultCodePage = 1252;
            mAdv = data;
            mMediaPool = mediapool;
            mGamePool = gamepool;
            mPath = path;
        }

        public AdvFileReader(string filename, TreeView mediapool, TreeView gamepool, Persistence persistence) : this(persistence)
        {
            ZipConstants.DefaultCodePage = 1252;
            mAdv = new AdvData(this, persistence);
            mMediaPool = mediapool;
            mGamePool = gamepool;

            if (System.IO.Path.GetExtension(filename) == ".dat")
            {
                mIsAdv = false;
                mAdv.Settings.Projectname = "game";
                string dir = System.IO.Path.GetDirectoryName(filename);
                try
                {
                    FileInfo[] files = System.IO.Directory.GetParent(dir).GetFiles("*.exe");
                    foreach (FileInfo file in files)
                    {
                        if (file.Name != "Setup.exe")
                        {
                            mAdv.Settings.Projectname = System.IO.Path.GetFileNameWithoutExtension(file.Name);
                        }
                    }
                }
                catch (DirectoryNotFoundException e)
                {
                    throw new FileNotFoundException("cannot load game.dat: Directory not found", e);
                }
            }
            else
            {
                mIsAdv = true;
                mAdv.Settings.Projectname = Path.GetFileNameWithoutExtension(filename);
            }

            ZipInputStream zis = new ZipInputStream(File.OpenRead(filename));
            ZipEntry entry;
            int ver_major = 0, ver_minor = 0;
            while ((entry = zis.GetNextEntry()) != null)
            {
                if (Path.GetExtension(entry.Name) == ".001")
                {
                    readSettings(zis, out ver_major, out ver_minor);
                }
                else if (Path.GetExtension(entry.Name) == ".002")
                {
                    readObjects(zis, ver_major, ver_minor);
                }
                else if (Path.GetExtension(entry.Name) == ".003")
                {
                    readScripts(zis);
                }
                else if (Path.GetExtension(entry.Name) == ".004")
                {
                    readLanguages(zis);
                }
                else if (Path.GetExtension(entry.Name) == ".005")
                {
                    readPassword(zis);
                }
                else if (Path.GetExtension(entry.Name) == ".010")
                {
                    readAdvancedSettings(zis);
                }
            }
            zis.Close();
            mPath = System.IO.Path.GetDirectoryName(filename);
        }

        protected bool readSettings(Stream strm, out int ver_major, out int ver_minor)
        {
            StreamReader rdr = new StreamReader(strm, Encoding.GetEncoding(1252));
            string str = rdr.ReadLine();
            ver_major = Int32.Parse(str.Substring(0, 1));
            ver_minor = Int32.Parse(str.Substring(2, 1));
            mVerMajor = ver_major;
            mVerMinor = ver_minor;
            if (str.Substring(4) != "Point&Click Project File. DO NOT MODIFY!!")
                return false;
            str = rdr.ReadLine();
			
			PlatformID pid = Environment.OSVersion.Platform;
			bool onUnix = (int)pid == 4 || (int)pid == 128;

            if (ver_major > 1)
                mAdv.Settings.Directory = rdr.ReadLine();
            else
                mAdv.Settings.Directory = "";

            str = rdr.ReadLine();
            if (str == "Resolution X : 640")
            {
                mAdv.Settings.Resolution = new Vec2i(640, 480);
            }
            else if (str == "Resolution X : 800")
            {
                mAdv.Settings.Resolution = new Vec2i(800, 600);
            }
            else if (str == "Resolution X : 320")
            {
                mAdv.Settings.Resolution = new Vec2i(320, 240);
            }
            else
            {
                throw new UnexpectedValueException("Resolution is invalid");
            }
            string font = rdr.ReadLine();
            while (font.Substring(0, 11) != "GameFont : ")
            {
                string[] fontparts = font.Split(';');
                FontInfo info = new FontInfo();
                info.name = fontparts[0];
                info.bold = Convert.ToInt32(fontparts[1]) == -1;
                info.italic = Convert.ToInt32(fontparts[2]) == -1;
                info.size = Convert.ToInt32(fontparts[3]);
                info.outline = Convert.ToInt32(fontparts[4]);
                if (ver_major > 1)
                    info.charset = Convert.ToInt32(fontparts[5]);
                else
                    info.charset = 0;
                if (ver_major > 2 || (ver_major == 2 && ver_minor > 0))
                {
                    info.shadow = Convert.ToInt32(fontparts[6]);
                    info.fill = Convert.ToInt32(fontparts[7]);
                    info.fading = Convert.ToInt32(fontparts[8]);
                }
                else
                {                  
                    info.shadow = 0;
                    info.fill = 0;
                    info.fading = 0;
                }
                if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5))
                    info.spacing = Convert.ToInt32(fontparts[9]);
                else
                    info.spacing = 0;
                mAdv.Settings.Fonts.Add(info);
                font = rdr.ReadLine();
            } 
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            if (ver_major > 1)
            {
                mAdv.Settings.GameIcon = str.Substring(11);
                str = rdr.ReadLine();
                if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5))
                {
                    string[] data = str.Split(';');
                    mAdv.Settings.LoadingImage = data[0];
                    mAdv.Settings.SettingsPicture = data[1];
                }
                else
                    mAdv.Settings.LoadingImage = str;
                str = rdr.ReadLine();
                mAdv.Settings.TsUseBgImage = str == "-1";
                mAdv.Settings.TsBackground = rdr.ReadLine();
            }
            else
            {
                mAdv.Settings.GameIcon = "";
                mAdv.Settings.LoadingImage = "";
                mAdv.Settings.TsUseBgImage = false;
                mAdv.Settings.TsBackground = "";
            }
            str = rdr.ReadLine();
            if (str.Substring(0, 14) != "Startskript : ")
            {
                throw new UnexpectedValueException("expected Startskript : ");
            }
            mAdv.Settings.Startscript = str.Substring(14);
            mAdv.Settings.Mainscript = rdr.ReadLine();
            mAdv.Settings.AnywhereRoom = rdr.ReadLine();
            str = rdr.ReadLine();
            mAdv.Settings.ScreenChange = (ScreenChange)Convert.ToInt32(str.Substring(15));
            str = rdr.ReadLine().Substring(9);
            mAdv.Settings.TextOnOff = str[0] == '1';
            mAdv.Settings.DrawDraggedItemIcons = str[1] == '1';
            mAdv.Settings.ActionText = str[2] == '1';
            mAdv.Settings.ShowTaskbar = str[3] == '1';
            if (ver_major > 2 || (ver_major == 2 && ver_minor > 0))
            {
                mAdv.Settings.NotAntialiased = str[4] == '1';
                mAdv.Settings.TaskbarFromTop = str[5] == '1';
            }
            else
            {
                mAdv.Settings.NotAntialiased = false;
                mAdv.Settings.TaskbarFromTop = false;
            }
            if (ver_major >= 3)
            {
                mAdv.Settings.GroupItems = str[6] == '1';
                mAdv.Settings.ProtectGameFile = str[7] == '1';
            }
            else{
                mAdv.Settings.GroupItems = false;
                mAdv.Settings.ProtectGameFile = false;
            }
            if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5))
            {
                mAdv.Settings.Is16to9 = str[8] == '1';
            }
            else
            {
                mAdv.Settings.Is16to9 = false;
            }
            if (ver_major > 2 || (ver_major == 2 && ver_minor > 0))
            {
                str = rdr.ReadLine();
                mAdv.Settings.ActionTextHeight = Convert.ToInt32(str);
            }
            else
            {
                mAdv.Settings.ActionTextHeight = 0;
            }
            str = rdr.ReadLine();
            mAdv.Settings.CustomMenu = str == "-1";
            mAdv.Settings.CustomMenuRoom = rdr.ReadLine();
            str = rdr.ReadLine();
            mAdv.Settings.TaskHeight = Convert.ToInt32(str.Substring(13));
            str = rdr.ReadLine();
            mAdv.Settings.TaskRoom = str.Substring(11);
            str = rdr.ReadLine();
            mAdv.Settings.TaskPopup = Convert.ToInt32(str.Substring(12));
            str = rdr.ReadLine();
            string addBools = str.Substring(8, 2);
            mAdv.Settings.SilentDelete = addBools[0] == '1';
            mAdv.Settings.InfoLine = addBools[1] == '1';
            if (ver_major > 1)
            {
                string taskshow = str.Substring(11, 4);
                if (taskshow == "hide")
                    mAdv.Settings.TaskHideCompletely = true;
                else
                    mAdv.Settings.TaskHideCompletely = false;
                mAdv.Settings.RightClick = Convert.ToInt32(str.Substring(16));
            }
            else{
                mAdv.Settings.TaskHideCompletely = false;
                mAdv.Settings.RightClick = 0;
            }
            if (ver_major > 2 || (ver_major == 2 && ver_minor > 0))
            {
                str = rdr.ReadLine();
                mAdv.Settings.UseMouseWheel = (str == "-1");
                str = rdr.ReadLine();
                mAdv.Settings.MenuFading = Convert.ToInt32(str);
                str = rdr.ReadLine();
                mAdv.Settings.TextSceneFading = Convert.ToInt32(str);
                str = rdr.ReadLine();
                mAdv.Settings.AnywhereTransparency = Convert.ToInt32(str);
            }
            else
            {
                mAdv.Settings.UseMouseWheel = false;
                mAdv.Settings.MenuFading = 0;
                mAdv.Settings.TextSceneFading = 0;
                mAdv.Settings.AnywhereTransparency = 0;
            }
            str = rdr.ReadLine();
            mAdv.Settings.TargaColor = Convert.ToUInt32(str.Substring(13));
            str = rdr.ReadLine();
            mAdv.Settings.BorderColor = Convert.ToUInt32(str.Substring(14));
            str = rdr.ReadLine();
            mAdv.Settings.BackgroundColor = Convert.ToUInt32(str.Substring(18));
            str = rdr.ReadLine();
            mAdv.Settings.TextColor = Convert.ToUInt32(str.Substring(12));
            str = rdr.ReadLine().Substring(15);
            string[] colors = str.Split(';');
            mAdv.Settings.OffspeechColor = Convert.ToUInt32(colors[0]);
            if (ver_major > 1)
                mAdv.Settings.InfotextColor = Convert.ToUInt32(colors[1]);
            else
                mAdv.Settings.InfotextColor = 16777215;
            str = rdr.ReadLine();
            mAdv.Settings.TsStyle = Convert.ToInt32(str) - 1;
            str = rdr.ReadLine();
            mAdv.Settings.TsBorderStyle = Convert.ToInt32(str) - 1;
            str = rdr.ReadLine();
            mAdv.Settings.TsBorderColor = Convert.ToUInt32(str);
            str = rdr.ReadLine();
            mAdv.Settings.TsAreaColor = Convert.ToUInt32(str);
            str = rdr.ReadLine();
            mAdv.Settings.TsSelectionColor = Convert.ToUInt32(str);
            str = rdr.ReadLine();
            mAdv.Settings.TsTextColor = Convert.ToUInt32(str);
            if (ver_major >= 3 || (ver_major >= 2 && ver_minor >= 7))
            {
                str = rdr.ReadLine();
                mAdv.Settings.TsUseSymbols = str == "-1";
                str = rdr.ReadLine();
                mAdv.Settings.MuteMusicWhenSpeech = str == "-1";
                str = rdr.ReadLine();
                mAdv.Settings.CoinActivated = str == "-1";
                str = rdr.ReadLine();
                mAdv.Settings.CoinAutoPopup = str == "-1";
                mAdv.Settings.CoinRoom = rdr.ReadLine();
                str = rdr.ReadLine();
                mAdv.Settings.CoinFading = Convert.ToInt32(str);
                str = rdr.ReadLine();
                mAdv.Settings.CoinCenter.y = Convert.ToInt32(str);
                str = rdr.ReadLine();
                mAdv.Settings.CoinCenter.x = Convert.ToInt32(str);
            }
            if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5))
            {
                //dsp effects
                for (int i = 0; i < 25; ++i)
                {
                    str = rdr.ReadLine();
                    string [] header = str.Split(';');
                    DSPEffect effect = new DSPEffect();
                    effect.name = header[2];
                    int etype = Convert.ToInt32(header[3]);
                    effect.type = (DSPEffect.Type)etype;
                    for (int line = 0; line < 6; ++line)
                    {
                        str = rdr.ReadLine();
                        string[] vals = str.Split(';');
                        for (int num = 0; num < 6; ++num)
                        {
                            effect.param[line, num] = Convert.ToInt32(vals[num]);
                        }
                    }
                    if (etype > 0)
                        mAdv.Settings.DSPEffects.Add(effect);
                }
            }
            str = rdr.ReadLine();
            mAdv.Settings.LinkText = str.Substring(11);
            str = rdr.ReadLine();
            mAdv.Settings.GiveLink = str.Substring(11);
            str = rdr.ReadLine();
            mAdv.Settings.WalkText = str.Substring(11);
            str = rdr.ReadLine();
            if (str == "Booleans :")
            {
                mAdv.Settings.Booleans = new Dictionary<string, bool>();
                str = rdr.ReadLine();
                while (str != "Commands :")
                {
                    string name;
                    string val = str.Substring(str.Length - 1);
                    if (val == "1")
                        name = str.Substring(0, str.Length - 2);
                    else
                        name = str.Substring(0, str.Length - 1);
                    mAdv.Settings.Booleans.Add(name, val == "1");
                    str = rdr.ReadLine();
                }
            }
            if (str == "Commands :")
            {
                mAdv.Settings.Commands = new Dictionary<string, string>();
                str = rdr.ReadLine();
                while (str != "Mediapool :")
                {
                    string pretty_name = rdr.ReadLine();
                    mAdv.Settings.Commands.Add(str, pretty_name);
                    str = rdr.ReadLine();
                }
            }
            str = rdr.ReadLine();
            TreeNode root = new TreeNode("Mediapool");
            TreeNode node = root;
            mMediaPool.Nodes.Add(node);
            while (str != "Gamepool :")
            {
                node = insertTreeElement(mMediaPool.Nodes, str, node, node.Level-1);
                str = rdr.ReadLine();
            }
            TreeNode[] arr = new TreeNode[root.Nodes.Count];
            root.Nodes.CopyTo(arr, 0);
            mMediaPool.Nodes.Clear();
            mMediaPool.Nodes.AddRange(arr);

            str = rdr.ReadLine();
            root = new TreeNode("Gamepool");
            node = root;
            mGamePool.Nodes.Add(node);
            while (str != "Images :")
            {
                node = insertTreeElement(mGamePool.Nodes, str, node, node.Level - 1);
                str = rdr.ReadLine();
            }
            arr = new TreeNode[root.Nodes.Count];
            root.Nodes.CopyTo(arr, 0);
            mGamePool.Nodes.Clear();
            mGamePool.Nodes.AddRange(arr);

            if (str == "Images :")
            {
                str = rdr.ReadLine();
                while (str != "Sounds :")
                {
                    string filename = rdr.ReadLine();
					if (onUnix){
						filename = filename.Replace("\\", "/");
						if (filename[1] == ':')
							filename = filename.Remove(0, 2);
					}
                    mAdv.Images.Add(str.ToLower(), filename);
                    str = rdr.ReadLine();
                }
            }

            if (str == "Sounds :")
            {
                str = rdr.ReadLine();
                while (str != "Music :")
                {
                    string filename = rdr.ReadLine();
                    mAdv.Sounds.Add(str.ToLower(), filename);
                    str = rdr.ReadLine();
                }
            }

            if (str == "Music :")
            {
                str = rdr.ReadLine();
                while (str != "Movie :")
                {
                    string filename = rdr.ReadLine();
                    mAdv.Music.Add(str.ToLower(), filename);
                    str = rdr.ReadLine();
                }
            }

            if (str == "Movie :")
            {
                str = rdr.ReadLine();
                while (!rdr.EndOfStream)
                {
                    string filename = rdr.ReadLine();
                    mAdv.Videos.Add(str.ToLower(), filename);
                    str = rdr.ReadLine();
                }
            }
            return true;
        }

        protected bool readObjects(Stream strm, int ver_major, int ver_minor)
        {
            StreamReader rdr = new StreamReader(strm, Encoding.GetEncoding(1252));
            string str = rdr.ReadLine();
            int local_ver_major = Int32.Parse(str.Substring(0, 1));
            int local_ver_minor = Int32.Parse(str.Substring(2, 1));
            if (str.Substring(4) != "Point&Click Project File. DO NOT MODIFY!!")
                return false;
            str = rdr.ReadLine();
            for (int state = 0; state < STATES_MAX; ++state)
            {
                CursorState cs;
                cs.frames = new System.Collections.ArrayList();
                for (int frames = 0; frames < FRAMES_MAX; ++frames)
                {
                    str = rdr.ReadLine();
                    if (str.Length > 0)
                        cs.frames.Add(str);
                }
                str = rdr.ReadLine();
                cs.command = Convert.ToInt32(str);
                str = rdr.ReadLine();
                cs.fpsDivider = Convert.ToInt32(str);
                str = rdr.ReadLine();
                cs.highlight.x = Convert.ToInt32(str);
                str = rdr.ReadLine();
                cs.highlight.y = Convert.ToInt32(str);
                if (ver_major > 3 || (ver_major == 3 && ver_minor >= 6))
                {
                    str = rdr.ReadLine();
                    cs.itemoffset.x = Convert.ToInt32(str);
                    str = rdr.ReadLine();
                    cs.itemoffset.y = Convert.ToInt32(str);
                }
                else
                {
                    cs.itemoffset = new Vec2i(16, 16);
                }
                mAdv.Cursor.Add(cs);
            }
            return readObjectsLoop(rdr, ver_major, ver_minor, null);
        }

        protected bool readObjectsLoop(StreamReader rdr, int ver_major, int ver_minor, TreeNode parent)
        {
            string str;
            while (!rdr.EndOfStream)
            {
                str = rdr.ReadLine();
                if (rdr.EndOfStream)
                    return true;
                if (str.Length < 2)
                    continue;
                string rest = str.Substring(2);
                //TODO check
                string[] typename = new string[2];
                int idx = rest.IndexOf(' ');
                typename[0] = rest.Substring(0, idx);
                typename[1] = rest.Substring(idx+1);
                //ITEM
                if (typename[0] == "Item")
                {
                    Item it = new Item(mAdv);
                    it.Name = typename[1];
                    int numStates = STATES_MAX;
                    int delim = 2;
                    if (ver_major == 2 || (ver_major == 3 && ver_minor == 0))
                    {
                        numStates = 1;
                        delim = 1;
                    }
                    for (int state = 0; state < numStates; ++state)
                    {
                        ItemState ist;
                        ist.frames = new System.Collections.ArrayList();
                        ist.scripts = new System.Collections.ArrayList();
                        for (int frames = 0; frames < FRAMES_MAX; ++frames)
                        {
                            str = rdr.ReadLine();
                            if (str.Length > 0)
                            {
                                if (delim == 2)
                                {
                                    string[] split = str.Split(';');
                                    ist.frames.Add(split[0]);
                                    if (split.Length > 1)
                                        ist.scripts.Add(split[1].Replace('\xaf', ';'));
                                    else
                                        ist.scripts.Add("");
                                }
                                else
                                {
                                    ist.frames.Add(str.Substring(0, str.Length + 1 - delim));
                                    ist.scripts.Add("");
                                }
                            }
                        }
                        str = rdr.ReadLine();
                        ist.fpsDivider = Convert.ToInt32(str);
                        it.Add(ist);
                    }
                    if (ver_major == 2 || (ver_major == 3 && ver_minor == 0))
                    {
                        for (int i = 1; i < STATES_MAX; ++i)
                        {
                            ItemState ist;
                            ist.frames = new System.Collections.ArrayList();
                            ist.scripts = new System.Collections.ArrayList();
                            ist.fpsDivider = 20;
                            it.Add(ist);
                        }
                    }
                    mAdv.addItem(it);
                }
                //OBJECT
                else if (typename[0] == "Object")
                {
                    AdvObject obj = mAdv.getObject(typename[1]);
                    if (obj == null)
                    {
                        obj = new AdvObject(mAdv);
                        obj.Name = typename[1];
                        mAdv.addObject(obj);
                    }
                    str = rdr.ReadLine();
                    int x = Convert.ToInt32(str);
                    str = rdr.ReadLine();
                    int y = Convert.ToInt32(str);
                    obj.setSize(0, new Vec2i(x, y));
                    str = rdr.ReadLine();
                    x = Convert.ToInt32(str);
                    obj.Lighten = x != 0;
                    for (int state = 0; state < STATES_MAX; ++state)
                    {
                        ObjectState os = new ObjectState();
                        os.fpsDivider = readExtendedFrames(rdr, os.frames);
                        obj.Add(os);
                    }
                }
                //CHARACTER
                else if (typename[0] == "Character")
                {
                    AdvCharacter chr = new AdvCharacter(mAdv);
                    chr.Name = typename[1];
                    str = rdr.ReadLine();
                    chr.TextColor = Convert.ToUInt32(str);
                    chr.WalkSpeed = Convert.ToInt32(rdr.ReadLine());
                    if (ver_major > 0)
                    {
                        int tmp = Convert.ToInt32(rdr.ReadLine());
                        chr.NoZoom = tmp != 0;
                        if (ver_major > 3 || (ver_major == 3 && ver_minor > 0))
                        {
                            tmp = Convert.ToInt32(rdr.ReadLine());
                            chr.RealLeftAnimations = tmp != 0;
                        }
                        tmp = Convert.ToInt32(rdr.ReadLine());
                        chr.MemoryReistent = tmp != 0;
                        tmp = Convert.ToInt32(rdr.ReadLine());
                        chr.Ghost = tmp != 0;
                        chr.Walksound = rdr.ReadLine();
                    }
                    if (ver_major >= 3)
                    {
                        str = rdr.ReadLine();
                        string[] names = str.Split(';');
                        for (int i = 0; i < names.Length - 1; ++i)
                        {
                            chr.setStateName(16 + i, names[i]);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < 20; ++i)
                            chr.setStateName(16 + i, "");
                    }
                    if (ver_major > 0)
                    {
                        chr.Font = Convert.ToInt32(rdr.ReadLine());
                        chr.Zoom = Convert.ToInt32(rdr.ReadLine());
                    }
                    else
                        chr.Zoom = 100;
                    for (int state = 0; state < CHAR_STATES_MAX; ++state)
                    {
                        CharacterState cs = new CharacterState();
                        cs.size.x = Convert.ToInt32(rdr.ReadLine());
                        cs.size.y = Convert.ToInt32(rdr.ReadLine());
                        cs.basepoint.x = Convert.ToInt32(rdr.ReadLine());
                        cs.basepoint.y = Convert.ToInt32(rdr.ReadLine());
                        cs.fpsDivider = readExtendedFrames(rdr, cs.frames);
                        chr.Add(cs);
                    }
                    mAdv.addCharacter(chr);
                }
                //CHARACTER INSTANCE
                else if (typename[0] == "Rcharacter")
                {
                    string chr = rdr.ReadLine();
                    CharacterInstance charinst = new CharacterInstance(mAdv.getCharacter(chr), mAdv);
                    if (charinst.Character == null)
                        throw new UnexpectedValueException("No character for character instance found");
                    charinst.Name = typename[1];
                    charinst.Room = rdr.ReadLine();
                    Vec2i pos;
                    pos.x = Convert.ToInt32(rdr.ReadLine());
                    pos.y = Convert.ToInt32(rdr.ReadLine());
                    charinst.RawPosition = pos;
                    charinst.LookDir = Convert.ToInt32(rdr.ReadLine());
                    charinst.Unmovable = Convert.ToInt32(rdr.ReadLine()) == 0;
                    charinst.Locked = Convert.ToInt32(rdr.ReadLine()) != 0;
                    if (!mAdv.CharacterInstances.ContainsKey(charinst.Room.ToLower()))
                    {
                        mAdv.CharacterInstances[charinst.Room.ToLower()] = new System.Collections.ArrayList();
                    }
                    mAdv.CharacterInstances[charinst.Room.ToLower()].Add(charinst);
                }
                //ROOM
                else if (typename[0] == "Room")
                {
                    Room room = new Room();
                    room.Data = mAdv;
                    room.Name = typename[1];
                    room.Size.x = Convert.ToInt32(rdr.ReadLine());
                    room.Size.y = Convert.ToInt32(rdr.ReadLine());
                    room.ScrollOffset.x = Convert.ToInt32(rdr.ReadLine());
                    room.ScrollOffset.y = Convert.ToInt32(rdr.ReadLine());
                    room.Depthmap.x = Convert.ToInt32(rdr.ReadLine());
                    room.Depthmap.y = Convert.ToInt32(rdr.ReadLine());
                    room.Zoom = Convert.ToInt32(rdr.ReadLine());
                    room.Background = rdr.ReadLine();
                    room.ParallaxBackground = rdr.ReadLine();
                    room.FXShapes = new System.Collections.ArrayList();
                    if (ver_major >= 3)
                    {
                        int tmp = Convert.ToInt32(rdr.ReadLine());
                        room.DoubleWalkmap = tmp != 0;
                    }
                    else
                    {
                        room.DoubleWalkmap = false;
                    }
                    if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5))
                    {
                        str = rdr.ReadLine();
                        string[] col = str.Split(';');
                        room.Lighting = Color.FromArgb(Convert.ToInt32(col[0]), Convert.ToInt32(col[1]), Convert.ToInt32(col[2]));
                    }
                    else
                        room.Lighting = Color.FromArgb(255, 255, 255);
                    if (ver_major >= 3 || (ver_major == 2 && ver_minor >= 8))
                    {
                        for (int i = 0; i < FXSHAPES_MAX; ++i)
                        {
                            FxShape shape = new FxShape();
                            str = rdr.ReadLine();
                            string[] split = str.Split(';');
                            shape.Active = Convert.ToInt32(split[0]) != 0;
                            shape.DependingOnRoomPosition = Convert.ToInt32(split[1]) != 0;
                            shape.Effect = (FxShape.FxEffect)Convert.ToInt32(rdr.ReadLine());
                            shape.Room = rdr.ReadLine();
                            shape.Depth = Convert.ToInt32(rdr.ReadLine());
                            shape.MirrorOffset.x = Convert.ToInt32(rdr.ReadLine());
                            shape.MirrorOffset.y = Convert.ToInt32(rdr.ReadLine());
                            shape.Strength = Convert.ToInt32(rdr.ReadLine());
                            str = rdr.ReadLine();
                            split = str.Split(';');
                            for (int pos = 0; pos < 4; ++pos)
                            {
                                shape.Positions[pos].x = Convert.ToInt32(split[2 * pos]);
                                shape.Positions[pos].y = Convert.ToInt32(split[2 * pos + 1]);
                            }
                            room.FXShapes.Add(shape);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < FXSHAPES_MAX; ++i)
                        {
                            FxShape fs = new FxShape(i);
                            room.FXShapes.Add(fs);
                        }
                    }
                    //inventory
                    room.HasInventory = Convert.ToInt32(rdr.ReadLine()) != 0;
                    str = rdr.ReadLine();
                    string[] inventory = str.Split(';');
                    room.InvPos.x = Convert.ToInt32(inventory[0]);
                    room.InvPos.y = Convert.ToInt32(inventory[1]);
                    room.InvSize.x = Convert.ToInt32(inventory[2]);
                    room.InvSize.y = Convert.ToInt32(inventory[3]);
                    if (ver_major < 3)
                    {
                        if (room.InvSize.x == 0 || room.InvSize.y == 0)
                            room.HasInventory = false;
                        else
                            room.HasInventory = true;
                    }
                    System.Globalization.NumberFormatInfo info = new System.Globalization.NumberFormatInfo();
                    info.NumberDecimalSeparator = ",";
                    try
                    {
                        room.InvScale.x = Single.Parse(inventory[4], info);
                        room.InvScale.y = Single.Parse(inventory[5], info);
                    }
                    catch (Exception)
                    {
                        info.NumberDecimalSeparator = ".";
                        room.InvScale.x = Single.Parse(inventory[4], info);
                        room.InvScale.y = Single.Parse(inventory[5], info);
                    }
                    if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5)){
                        room.InvSpacing = Convert.ToInt32(inventory[6]);
                    }
                    else
                        room.InvSpacing = 10;
                    //walkmap
                    str = rdr.ReadLine();
                    int walkmapX = 32;
                    int walkGridSize = mAdv.Settings.Resolution.x / walkmapX;
                    int walkmapY = mAdv.Settings.Resolution.y / walkGridSize;
                    walkmapX *= 3;
                    walkmapY *= 2;
                    int walkmapXOut = walkmapX * 2;
                    int walkmapYOut = walkmapY * 2;
                    if (ver_major >= 3)
                    {
                        walkmapX *= 2;
                        walkmapY *= 2;
                    }
                    room.Walkmap = new Room.WalkMapEntry[walkmapXOut, walkmapYOut];
                    for (int i = 0; i < walkmapX * walkmapY; ++i)
                    {
                        int x = i / walkmapY;
                        int y = i % walkmapY;
                        room.Walkmap[x, y].isFree = str[2 * i] != '1';
                        room.Walkmap[x, y].hasScript = str[2 * i + 1] == '1';
                    }
                    if (mAdv.CharacterInstances.ContainsKey(room.Name.ToLower()))
                        room.Characters = mAdv.CharacterInstances[room.Name.ToLower()];
                    mAdv.addRoom(room);
                    mLastRoom = room;
                }
                //OBJECT INSTANCE
                else if (typename[0] == "Roomobject")
                {
                    string obj = rdr.ReadLine();
                    AdvObject newobj = mAdv.getObject(obj);
                    if (newobj == null)
                    {
                        //this happens during room import
                        newobj = new AdvObject(mAdv);
                        newobj.Name = obj;
                        mAdv.addObject(newobj);
                        //auto add objects
                        if (parent != null)
                        {
                            TreeNode tno = new TreeNode(obj);
                            tno.Tag = ResourceID.OBJECT;
                            tno.ImageIndex = Utilities.ResourceIDToImageIndex((ResourceID)tno.Tag);
                            tno.SelectedImageIndex = tno.ImageIndex;
                            parent.Nodes.Add(tno);
                        }
                    }
                    ObjectInstance objinst = new ObjectInstance(mAdv.getObject(obj), mAdv);
                    objinst.Name = typename[1];
                    objinst.Position.x = Convert.ToInt32(rdr.ReadLine());
                    objinst.Position.y = Convert.ToInt32(rdr.ReadLine());
                    objinst.State = Convert.ToInt32(rdr.ReadLine());
                    objinst.Layer = Convert.ToInt32(rdr.ReadLine());
                    objinst.Depth = Convert.ToInt32(rdr.ReadLine())*2;
                    if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5))
                    {
                        objinst.Depth = Convert.ToInt32(rdr.ReadLine());
                    }
                    objinst.Locked = Convert.ToInt32(rdr.ReadLine()) != 0;
                    mLastRoom.Objects.Add(objinst);
                }
            }
            return true;
        }

        protected bool readScripts(Stream strm)
        {
            StreamReader rdr = new StreamReader(strm, Encoding.GetEncoding(1252));
            string str = rdr.ReadLine();
            int ver_major = Int32.Parse(str.Substring(0, 1));
            int ver_minor = Int32.Parse(str.Substring(2, 1));
            if (str.Substring(4) != "Point&Click Project File. DO NOT MODIFY!!")
                return false;
            return readScriptsLoop(rdr, ver_major, ver_minor);
        }

        protected bool readScriptsLoop(StreamReader rdr, int ver_major, int ver_minor)
        {      
            string str = rdr.ReadLine();
            mLastScript = null;
            while (!rdr.EndOfStream)
            {
                str = rdr.ReadLine();
                if (str.Length >= 2 && str.StartsWith("//"))
                {
                    string rest = str.Substring(2);
                    int idx = rest.IndexOf(' ');
                    string[] typename = new string[2];
                    typename[0] = rest.Substring(0, idx);
                    typename[1] = rest.Substring(idx + 1);
                    Script.Type scrType;
                    if (typename[0] == "Cutscene")
                        scrType = Script.Type.CUTSCENE;
                    else if (typename[0] == "Item")
                        scrType = Script.Type.ITEM;
                    else if (typename[0] == "Character")
                        scrType = Script.Type.CHARACTER;
                    else if (typename[0] == "Room")
                        scrType = Script.Type.ROOM;
                    else if (typename[0] == "Object")
                        scrType = Script.Type.OBJECT;
                    else if (typename[0] == "Walkmap")
                        scrType = Script.Type.WALKMAP;
                    else
                        return false;
                    Script scr = new Script(scrType);
                    scr.Name = typename[1];
                    if (scrType == Script.Type.WALKMAP)
                    {
                        string roomname;
                        Vec2i pos;
                        if (ver_major >= 3)
                        {
                            roomname = scr.Name.Substring(7);
                            pos.x = Convert.ToInt32(scr.Name.Substring(1, 3));
                            pos.y = Convert.ToInt32(scr.Name.Substring(4, 3));
                        }
                        else
                        {
                            roomname = scr.Name.Substring(4);
                            pos.x = Convert.ToInt32(scr.Name.Substring(0, 2));
                            pos.y = Convert.ToInt32(scr.Name.Substring(2, 2));
                        }
                        mAdv.addWalkmapScript(scr, pos, roomname);
                    }
                    else
                    {
                        mAdv.addScript(scr);
                    }
                    mLastScript = scr;
                }
                else
                {
                    if (mLastScript.Text == null)
                        mLastScript.Text += str;
                    else
                        mLastScript.Text += "\n" + str;
                }
            }
            return true;
        }

        protected bool readPassword(Stream strm){
          StreamReader rdr = new StreamReader(strm, Encoding.GetEncoding(1252));
          string str = rdr.ReadLine();
          string pwd = "";
          for (int i = 0; i < 20; ++i){
            int val = Convert.ToInt32(str.Substring(i*3, 3));
            char curr;
            if (i >= 15)
              curr = (char)(val/3);
            else if (i >= 10)
              curr = (char)(val/6);
            else if (i >= 5)
              curr = (char)(val/4);
            else
              curr = (char)(val/5);
            pwd += curr;
          }
          mZipPwd = pwd;
          return true;
        }

        protected bool readLanguages(Stream strm)
        {
            StreamReader rdr = new StreamReader(strm, Encoding.GetEncoding(1252));
            string language = null;
            Language.Section section = Language.Section.NumSections;
            while (!rdr.EndOfStream)
            {
                string str = rdr.ReadLine();
                if (str.StartsWith("*/*")){
                    string[] parts = str.Split(';');
                    language = parts[0].Substring(3);
                    switch (parts[1])
                    {
                        case "speech":
                            section = Language.Section.Speech;
                            break;
                        case "voicespeech":
                            section = Language.Section.Speech_Sounds;
                            break;
                        case "offspeech":
                            section = Language.Section.Offspeech;
                            break;
                        case "voiceoffspeech":
                            section = Language.Section.Offspeech_Sounds;
                            break;
                        case "showinfo":
                            section = Language.Section.Showinfo;
                            break;
                        case "textout":
                            section = Language.Section.Textout;
                            break;
                        case "setstring":
                            section = Language.Section.Setstring;
                            break;
                        case "row":
                            section = Language.Section.Textscenes;
                            break;
                        case "kommandos":
                            section = Language.Section.Commands;
                            break;
                    }
                    continue;
                }
                mAdv.addLanguage(language, section, str);
            }
            mAdv.buildLanguageIndex();
            return true;
        }

        protected int readExtendedFrames(StreamReader rdr, System.Collections.ArrayList frames)
        {
            string str;
            int realFrames = 0;
            for (int frms = 0; frms < FRAMES2_MAX; ++frms)
            {
                ExtendedFrame frm = new ExtendedFrame();
                bool[] set = new bool[PARTS_MAX];
                for (int parts = 0; parts < PARTS_MAX; ++parts)
                {
                    str = rdr.ReadLine();
                    if (str.Length > 0)
                    {
                        set[parts] = true;
                        frm.names.Add(str);
                    }
                }
                //read offsets
                str = rdr.ReadLine();
                string[] split = str.Split(';');
                for (int i = 0; i < PARTS_MAX; ++i)
                {
                    if (!set[i])
                        continue;
                    Vec2i offset = new Vec2i(Convert.ToInt32(split[2*i]), Convert.ToInt32(split[2*i+1]));
                    frm.offsets.Add(offset);
                }
                if (set[0] || set[1])
                    realFrames = frms+1;
                if (split.Length > PARTS_MAX * 2)
                    frm.script = split[PARTS_MAX * 2].Replace('\xaf', ';');
                frames.Add(frm);
            }
            frames.RemoveRange(realFrames, FRAMES2_MAX - realFrames);
            str = rdr.ReadLine();
            return Convert.ToInt32(str);
        }

        TreeNode insertTreeElement(TreeNodeCollection nodes, string name, TreeNode current, int curr_level)
        {
            int level = 0;
            for (int i = 0; i < name.Length; ++i)
            {
                if (name[i] == '"')
                    ++level;
                else
                    break;
            }
            string idstr = name.Substring(level, 2);
            string label = name.Substring(level + 2);
            int leveldiff = level - curr_level;
            while (leveldiff <= 0)
            {
                current = current.Parent;
                ++leveldiff;
            }
            TreeNode node = new TreeNode(label);
            node.Tag =  (ResourceID)Convert.ToInt32(idstr);
            node.ImageIndex = Utilities.ResourceIDToImageIndex((ResourceID)node.Tag);
            node.SelectedImageIndex = node.ImageIndex;
            current.Nodes.Add(node);
            return node;
        }

        public System.Drawing.Bitmap getImage(string filename)
        {
            if (mIsAdv)
            {
                Bitmap file = (Bitmap)Bitmap.FromFile(filename);
                Bitmap ret = new Bitmap(file);
                file.Dispose();
                return ret;
            }
            string zipname = mPath + System.IO.Path.DirectorySeparatorChar + "gfx.dat";
            string imagename = System.IO.Path.GetFileName(filename);
            ZipInputStream zis = new ZipInputStream(File.OpenRead(zipname));
            if (mZipPwd != null && mZipPwd.Length > 0)
                zis.Password = mZipPwd;
            ZipEntry entry;
            /*char[] arr = imagename.ToCharArray();
            for (int i = 0; i < imagename.Length; ++i)
            {
                if (arr[i] == '\xf6')
                    arr[i] = '\xf7';
                else if (arr[i] == '�')
                    arr[i] = '�';
                else if (arr[i] == '�')
                    arr[i] = '�';
                else if (arr[i] == '�')
                    arr[i] = '\x2580';
            }
            imagename = new string(arr);*/
            
            System.Drawing.Bitmap img = null;
            while ((entry = zis.GetNextEntry()) != null)
            {
                if (entry.Name == imagename)
                {
                    img = (Bitmap)System.Drawing.Bitmap.FromStream(zis);
                    if (System.IO.Path.GetExtension(imagename) == ".pnj")
                    {
                        System.Drawing.Bitmap alpha = getImage(System.IO.Path.ChangeExtension(filename, ".pna"));
                        System.Drawing.Bitmap newImage = new System.Drawing.Bitmap(img.Width, img.Height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                        for (int i = 0; i < img.Width; ++i){
                            for (int j = 0; j < img.Height; ++j)
                            {
                                newImage.SetPixel(i,j,Color.FromArgb(alpha.GetPixel(i,j).R, img.GetPixel(i,j)));
                            }
                        }
                        img = newImage;
                    }
                    break;
                }
            }
            zis.Close();
            return img;
        }

        protected bool readAdvancedSettings(Stream strm)
        {
            StreamReader rdr = new StreamReader(strm, Encoding.GetEncoding(1252));
            string str = rdr.ReadLine();
            int ver_major = Int32.Parse(str.Substring(0, 1));
            int ver_minor = Int32.Parse(str.Substring(2, 1));
            mVerMajor = ver_major;
            mVerMinor = ver_minor;
            if (str.Substring(4) != "Point&Click Project File. DO NOT MODIFY!!")
                return false;
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            mAdv.Settings.PngToJpeg = str == "-1";
            if (ver_major == 3 && ver_minor < 5)
                return true;
            str = rdr.ReadLine();
            mAdv.Settings.ScriptingLanguage = (ProjectSettings.ScriptLang)Enum.Parse(typeof(ProjectSettings.ScriptLang), str);
            return true;
        }

        private AdvData mAdv;

        public AdvData Data
        {
            get { return mAdv; }
        }

        public int MajorVersion
        {
            get { return mVerMajor; }
        }

        public int MinorVersion
        {
            get { return mVerMinor; }
        }

        public static string unpackAdz(string filename)
        {
            string unpackpath = Path.GetDirectoryName(filename);
            unpackpath = Path.Combine(unpackpath, Path.GetFileNameWithoutExtension(filename));
            return unpackAdz(filename, unpackpath);
        }

        public static string unpackAdz(string filename, string destfolder)
        {
            Directory.CreateDirectory(destfolder);
            ZipInputStream zis = new ZipInputStream(File.OpenRead(filename));
            ZipEntry entry;
            string advname = null;
            while ((entry = zis.GetNextEntry()) != null)
            {
                if (Path.GetExtension(entry.Name) == ".adv")
                {
                    advname = Path.Combine(destfolder, entry.Name);
                    AdvFileWriter.writeFile(zis, Path.Combine(destfolder, entry.Name));
                }
                else if (Path.GetExtension(entry.Name) == ".dat")
                {
                    ZipInputStream input = new ZipInputStream(zis);
                    input.IsStreamOwner = false;
                    AdvFileWriter.writeFileRecursive(input, destfolder);
                    input.Close();
                }
            }
            zis.Close();
            return advname;
        }

        private void addImageList(string dirname, string mediaDir, Stream zis)
        {
            TreeNode dir = null;
            foreach (TreeNode node in mMediaPool.Nodes[0].Nodes)
            {
                if (node.Text == dirname)
                {
                    dir = node;
                    break;
                }
            }
            if (dir == null)
            {
                dir = new TreeNode(dirname);
                dir.Tag = ResourceID.FOLDER;
                dir.SelectedImageIndex = Utilities.ResourceIDToImageIndex(ResourceID.FOLDER);
                dir.ImageIndex = dir.SelectedImageIndex;
                mMediaPool.Nodes[0].Nodes.Add(dir);
            }

            StreamReader rdr = new StreamReader(zis, Encoding.GetEncoding(1252));
            while (!rdr.EndOfStream)
            {
                string file = rdr.ReadLine();
                string key = Path.GetFileNameWithoutExtension(file);
                if (!mAdv.Images.ContainsKey(key))
                {
                    string value = Path.Combine(mediaDir, Path.GetFileName(file));
                    mAdv.Images.Add(key.ToLower(), value);
                    TreeNode image = new TreeNode(key);
                    image.Tag = ResourceID.IMAGE;
                    image.ImageIndex = Utilities.ResourceIDToImageIndex((ResourceID)image.Tag);
                    image.SelectedImageIndex = image.ImageIndex;
                    dir.Nodes.Add(image);
                }
            }
            mMediaPool.Sort();
        }

        public void importCharacter(string filename)
        {
            ZipInputStream zis = new ZipInputStream(File.OpenRead(filename));
            ZipEntry entry;
            string dirname = Path.GetFileNameWithoutExtension(filename);
            string mediaDir = Path.Combine(mPath, dirname);
            while ((entry = zis.GetNextEntry()) != null)
            {
                if (entry.Name == "char.dat")
                {
                    StreamReader rdr = new StreamReader(zis, Encoding.GetEncoding(1252));
                    readObjectsLoop(rdr, 0, 0, null);
                    TreeNode chr = new TreeNode(dirname);
                    chr.Tag = ResourceID.CHARACTER;
                    chr.ImageIndex = Utilities.ResourceIDToImageIndex((ResourceID)chr.Tag);
                    chr.SelectedImageIndex = chr.ImageIndex;
                    mGamePool.Nodes[0].Nodes.Add(chr);
                    
                }
                else if (entry.Name == "gfx.chr")
                {
                    Directory.CreateDirectory(mediaDir);
                    ZipInputStream input = new ZipInputStream(zis);
                    input.IsStreamOwner = false;
                    AdvFileWriter.writeFileRecursive(input, mediaDir);
                    input.Close();
                }
                else if (entry.Name == "list.gfx")
                {
                    addImageList(dirname, mediaDir, zis);
                }
            }
            zis.Close();
        }

        public void importRoom(string filename)
        {
            ZipInputStream zis = new ZipInputStream(File.OpenRead(filename));
            ZipEntry entry;
            string dirname = Path.GetFileNameWithoutExtension(filename);
            string mediaDir = Path.Combine(mPath, dirname);
            while ((entry = zis.GetNextEntry()) != null)
            {
                if (entry.Name == "room.dat")
                {
                    TreeNode objects = new TreeNode(dirname);
                    objects.Tag = ResourceID.FOLDER;
                    objects.ImageIndex = Utilities.ResourceIDToImageIndex((ResourceID)objects.Tag);
                    objects.SelectedImageIndex = objects.ImageIndex;
                    mGamePool.Nodes[3].Nodes.Add(objects);
                    StreamReader rdr = new StreamReader(zis, Encoding.GetEncoding(1252));
                    readObjectsLoop(rdr, 0, 0, objects);
                    TreeNode room = new TreeNode(dirname);
                    room.Tag = ResourceID.ROOM;
                    room.ImageIndex = Utilities.ResourceIDToImageIndex((ResourceID)room.Tag);
                    room.SelectedImageIndex = room.ImageIndex;
                    mGamePool.Nodes[4].Nodes.Add(room);

                }
                else if (entry.Name == "gfx.roo")
                {
                    Directory.CreateDirectory(mediaDir);
                    ZipInputStream input = new ZipInputStream(zis);
                    input.IsStreamOwner = false;
                    AdvFileWriter.writeFileRecursive(input, mediaDir);
                    input.Close();
                }
                else if (entry.Name == "script.dat")
                {
                    StreamReader rdr = new StreamReader(zis, Encoding.GetEncoding(1252));
                    readScriptsLoop(rdr, 0, 0);
                }
                else if (entry.Name == "list.gfx")
                {
                    addImageList(dirname, mediaDir, zis);
                }
            }
            zis.Close();
        }

        public string ZipPassword
        {
            get { return mZipPwd; }
        }

        private TreeView mMediaPool;
        private TreeView mGamePool;
        private string mPath;
        private Script mLastScript;
        private Room mLastRoom;
        private string mZipPwd;
        private bool mIsAdv;
        private int mVerMajor;
        private int mVerMinor;
    }
}
