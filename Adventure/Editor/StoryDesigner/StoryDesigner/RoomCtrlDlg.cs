using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class RoomCtrlDlg : Form
    {
        public class RedrawEventArgs : EventArgs
        {
        }
        public delegate void RedrawEventHandler(object sender, RedrawEventArgs e);
        public event RedrawEventHandler RedrawRoom;

        public RoomCtrlDlg(Room room, AdvData data, MainForm mf)
        {
            InitializeComponent();
            mRoom = room;
            mData = data;
            mParent = mf;
            for (int i = 1; i <= 9; ++i)
            {
                zoomfactor.Items.Add(i);
            }
            for (int i = 0; i <= 10; ++i)
            {
                obj_state.Items.Add(i);
            }
            zoomfactor.SelectedItem = mRoom.Zoom;
            doublewalkmap.Checked = mRoom.DoubleWalkmap;
            roomwidth.Minimum = data.WindowXRes;
            roomwidth.Maximum = data.Settings.Resolution.x * 3;
            roomheight.Minimum = data.Settings.Resolution.y;
            roomheight.Maximum = data.Settings.Resolution.y * 2;
            roomwidth.Value = mRoom.Size.x < roomwidth.Minimum ? roomwidth.Minimum : mRoom.Size.x;
            roomheight.Value = mRoom.Size.y;
            light_color.BackColor = mRoom.Lighting;
            mDragOffset.x = -1;
            scroller.Paint += new PaintEventHandler(scroller_Paint);
            scroller.MouseDown += new MouseEventHandler(scroller_MouseDown);
            scroller.MouseMove += new MouseEventHandler(scroller_MouseMove);
            scroller.MouseUp += new MouseEventHandler(scroller_MouseUp);
            roomwidth.ValueChanged += new EventHandler(roomwidth_ValueChanged);
            roomheight.ValueChanged += new EventHandler(roomheight_ValueChanged);
            mFxShapes = new FxShapeTabCtrl(mRoom.FXShapes, this);
            mFxShapes.Visible = false;
            mFxShapes.Location = objectgroup.Location;
            this.Controls.Add(mFxShapes);
            mInvSpacing = new NumericUpDown();
            mInvSpacing.Visible = false;
            mInvSpacing.Location = objectgroup.Location;
            mInvSpacing.Value = room.InvSpacing;
            mInvSpacing.Minimum = 2;
            mInvSpacing.Maximum = 20;
            this.Controls.Add(mInvSpacing);
            mInvSpacing.ValueChanged += new EventHandler(mInvSpacing_ValueChanged);
        }

        void mInvSpacing_ValueChanged(object sender, EventArgs e)
        {
            mRoom.InvSpacing = (int)mInvSpacing.Value;
        }

        void scroller_MouseUp(object sender, MouseEventArgs e)
        {
            mDragOffset.x = -1;
        }

        const float divider = 2.0f;

        void scroller_MouseMove(object sender, MouseEventArgs e)
        {
            if (mDragOffset.x == -1)
                return;
            int realx = (int)((e.X/divider - mDragOffset.x) + mScrollOffset.x);
            int realy = (int)((e.Y/divider - mDragOffset.y)+ mScrollOffset.y);
            if (realx < 0)
                realx = 0;
            if (realy < 0)
                realy = 0;
            mRoom.ScrollOffset.x = (int)(realx * (mData.Settings.Resolution.x * 3.0f / mData.WindowXRes) / scroller.Width * mData.WindowXRes);
            mRoom.ScrollOffset.y = (int)(realy * 2.0f / scroller.Height * mData.Settings.Resolution.y);
            if (mRoom.ScrollOffset.x > mRoom.Size.x - mData.WindowXRes)
                mRoom.ScrollOffset.x = mRoom.Size.x - mData.WindowXRes;
            if (mRoom.ScrollOffset.y > mRoom.Size.y - mData.Settings.Resolution.y)
                mRoom.ScrollOffset.y = mRoom.Size.y - mData.Settings.Resolution.y;
            scroller.Invalidate();
            redrawRoom();
        }

        void scroller_MouseDown(object sender, MouseEventArgs e)
        {
            mDragOffset.x = e.X/(int)divider;
            mDragOffset.y = e.Y/(int)divider;
            mScrollOffset.x = scroller.Width / (mData.Settings.Resolution.x * 3 / mData.WindowXRes) * mRoom.ScrollOffset.x / mData.WindowXRes;
            mScrollOffset.y = scroller.Height / 2.0f * mRoom.ScrollOffset.y / mData.Settings.Resolution.y;
        }

        void scroller_Paint(object sender, PaintEventArgs e)
        {
            PictureBox ctrl = (PictureBox)sender;
            e.Graphics.DrawRectangle(Pens.Black, 0, 0, ctrl.Width-1, ctrl.Height-1);
            Rectangle roomrect = new Rectangle(0, 0,
                (int)(ctrl.Width / 3.0f * mRoom.Size.x / mData.Settings.Resolution.x),
                (int)(ctrl.Height / 2.0f * mRoom.Size.y / mData.Settings.Resolution.y));
            e.Graphics.FillRectangle(Brushes.LightGray, roomrect);
            e.Graphics.DrawRectangle(Pens.Black, roomrect);
            roomrect.X = (int)(ctrl.Width / 3.0f * mRoom.ScrollOffset.x / mData.Settings.Resolution.x);
            roomrect.Y = (int)(ctrl.Height / 2.0f * mRoom.ScrollOffset.y / mData.Settings.Resolution.y);
            roomrect.Width = (int)(ctrl.Width / (3.0f * mData.Settings.Resolution.x / mData.WindowXRes));
            roomrect.Height = ctrl.Height / 2;
            e.Graphics.FillRectangle(Brushes.White, roomrect);
            e.Graphics.DrawRectangle(Pens.Black, roomrect);
        }

        public DrawableObject SelectedObject
        {
            get { return mObject; }
            set {
                mObject = value;
                objectgroup.Enabled = false;
                charactergroup.Enabled = false;
                if (mObject == null)
                    return;
                if (mObject is ObjectInstance)
                {
                    objectgroup.Enabled = true;
                    ObjectInstance obj = (ObjectInstance)mObject;
                    obj_instname.Text = obj.Name;
                    obj_locked.Checked = obj.Locked;
                    obj_state.SelectedIndex = obj.State;
                    switch (obj.Layer)
                    {
                        case 0:
                            draw_back.Checked = true;
                            break;
                        case 1:
                            draw_middle.Checked = true;
                            break;
                        case 2:
                            draw_front.Checked = true;
                            break;
                    }
                }
                else if (mObject is CharacterInstance)
                {
                    charactergroup.Enabled = true;
                    CharacterInstance chr = (CharacterInstance)mObject;
                    chr_instancename.Text = chr.Name;
                    chr_locked.Checked = chr.Locked;
                    chr_unmovable.Checked = chr.Unmovable;
                    switch (chr.LookDir)
                    {
                        case 1:
                            look_down.Checked = true;
                            break;
                        case 2:
                            look_up.Checked = true;
                            break;
                        case 3:
                            look_right.Checked = true;
                            break;
                        case 4:
                            look_left.Checked = true;
                            break;
                    }
                }
            }
        }

        public RoomDlg.ViewMode ViewMode
        {
            set
            {
                if (value == RoomDlg.ViewMode.Specialfx)
                {
                    objectgroup.Visible = false;
                    charactergroup.Visible = false;
                    mInvSpacing.Visible = false;
                    mFxShapes.Visible = true;
                }
                else if (value == RoomDlg.ViewMode.Inventory)
                {
                    objectgroup.Visible = false;
                    charactergroup.Visible = false;
                    mInvSpacing.Visible = true;
                    mFxShapes.Visible = false;
                }
                else
                {
                    objectgroup.Visible = true;
                    charactergroup.Visible = true;
                    mInvSpacing.Visible = false;
                    mFxShapes.Visible = false;
                }
            }
        }

        private Room mRoom;
        private AdvData mData;
        private MainForm mParent;
        private DrawableObject mObject;
        private Vec2i mDragOffset;
        private Vec2f mScrollOffset;
        private FxShapeTabCtrl mFxShapes;
        private NumericUpDown mInvSpacing;

        private void roomwidth_ValueChanged(object sender, EventArgs e)
        {
            NumericUpDown ctrl = (NumericUpDown)sender;
            mRoom.Size.x = (int)ctrl.Value;
            scroller.Invalidate();
        }

        private void roomheight_ValueChanged(object sender, EventArgs e)
        {
            NumericUpDown ctrl = (NumericUpDown)sender;
            mRoom.Size.y = (int)ctrl.Value;
            scroller.Invalidate();
        }

        private void zoomfactor_SelectedIndexChanged(object sender, EventArgs e)
        {
            mRoom.Zoom = Convert.ToInt32(zoomfactor.Text);
        }

        private void doublewalkmap_CheckedChanged(object sender, EventArgs e)
        {
            mRoom.DoubleWalkmap = doublewalkmap.Checked;
            redrawRoom();
        }

        public void redrawRoom()
        {
            if (RedrawRoom != null)
                RedrawRoom(this, new RedrawEventArgs());
        }

        private void look_up_CheckedChanged(object sender, EventArgs e)
        {
            CharacterInstance chr = (CharacterInstance)mObject;
            chr.setLookDir(2);
            redrawRoom();
        }

        private void look_right_CheckedChanged(object sender, EventArgs e)
        {
            CharacterInstance chr = (CharacterInstance)mObject;
            chr.setLookDir(3);
            redrawRoom();
        }

        private void look_down_CheckedChanged(object sender, EventArgs e)
        {
            CharacterInstance chr = (CharacterInstance)mObject;
            chr.setLookDir(1);
            redrawRoom();
        }

        private void look_left_CheckedChanged(object sender, EventArgs e)
        {
            CharacterInstance chr = (CharacterInstance)mObject;
            chr.setLookDir(4);
            redrawRoom();
        }

        private void chr_locked_CheckedChanged(object sender, EventArgs e)
        {
            CharacterInstance chr = (CharacterInstance)mObject;
            chr.Locked = chr_locked.Checked;
        }

        private void chr_unmovable_CheckedChanged(object sender, EventArgs e)
        {
            CharacterInstance chr = (CharacterInstance)mObject;
            chr.Unmovable = chr_unmovable.Checked;
        }

        private void obj_locked_CheckedChanged(object sender, EventArgs e)
        {
            ObjectInstance obj = (ObjectInstance)mObject;
            obj.Locked = obj_locked.Checked;
        }

        private void obj_state_SelectedIndexChanged(object sender, EventArgs e)
        {
            ObjectInstance obj = (ObjectInstance)mObject;
            obj.State = Convert.ToInt32(obj_state.Text);
            redrawRoom();
        }

        private void draw_back_CheckedChanged(object sender, EventArgs e)
        {
            ObjectInstance obj = (ObjectInstance)mObject;
            obj.Layer = 0;
            redrawRoom();
        }

        private void draw_middle_CheckedChanged(object sender, EventArgs e)
        {
            ObjectInstance obj = (ObjectInstance)mObject;
            obj.Layer = 1;
            redrawRoom();
        }

        private void draw_front_CheckedChanged(object sender, EventArgs e)
        {
            ObjectInstance obj = (ObjectInstance)mObject;
            obj.Layer = 2;
            redrawRoom();
        }

        private void chr_editscript_Click(object sender, EventArgs e)
        {
            CharacterInstance chr = (CharacterInstance)mObject;
            mParent.showScript(Script.Type.CHARACTER, chr.Name);
        }

        private void obj_editscript_Click(object sender, EventArgs e)
        {
            ObjectInstance obj = (ObjectInstance)mObject;
            mParent.showScript(Script.Type.OBJECT, Script.toScriptName(obj.Name, mRoom.Name));
        }

        private void room_editscript_Click(object sender, EventArgs e)
        {
            mParent.showScript(Script.Type.ROOM, mRoom.Name);
        }

        private void border_color_Click(object sender, EventArgs e)
        {
            Utilities.chooseColor(sender);
            mRoom.Lighting = light_color.BackColor;
        }
    }
}