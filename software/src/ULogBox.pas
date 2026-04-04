unit ULogBox;


{$reference System.Drawing.dll}
{$reference System.Windows.Forms.dll}

{$resource res\copy.png}
{$resource res\save.png}
{$resource res\clear.png}


uses System;
uses System.IO;
uses System.Drawing;
uses System.Windows.Forms;
uses UExtensions;


type
  LogBox = class(Control)
    {$region Fields}
    private _Log     : System.Windows.Forms.RichTextBox;
    private _LogMenu : System.Windows.Forms.ContextMenuStrip;
    {$endregion}
    
    {$region Override}
    protected procedure OnPaint(e: PaintEventArgs); override;
    begin
      inherited OnPaint(e);
      e.Graphics.DrawRectangle(Pens.Gray, new Rectangle(0, 0, Width-1, Height-1));
    end;
    
    protected procedure OnSizeChanged(e: EventArgs); override;
    begin
      inherited OnSizeChanged(e);
      _Log.Size := new System.Drawing.Size(Width - 2, Height - 2);
    end;
    {$endregion}
    
    {$region Accessors}
    private function GetSize() := inherited Size;
    
    private procedure SetSize(value: System.Drawing.Size);
    begin
      _Log.Size := new System.Drawing.Size(value.Width - 2, value.Height - 2);
      inherited Size := value;
    end;
    
    private function GetTabStop() := inherited TabStop;
    
    private procedure SetTabStop(value: boolean);
    begin
      inherited TabStop := value;
      _Log.TabStop      := value;
    end;
    {$endregion}
    
    {$region Ctors}
    public constructor ();
    begin
      var _LogMenuCopy   := new ToolStripMenuItem();
      _LogMenuCopy.Text  := 'Copy';
      _LogMenuCopy.Image := Resources.Image('copy.png');
      _LogMenuCopy.Click += LogMenuCopyClick;
      
      var _LogMenuSave   := new ToolStripMenuItem();
      _LogMenuSave.Text  := 'Save';
      _LogMenuSave.Image := Resources.Image('save.png');
      _LogMenuSave.Click += LogMenuSaveClick;
      
      var _LogMenuClear   := new ToolStripMenuItem();
      _LogMenuClear.Text  := 'Clear';
      _LogMenuClear.Image := Resources.Image('clear.png');
      _LogMenuClear.Click += LogMenuClearClick;
      
      _LogMenu := new System.Windows.Forms.ContextMenuStrip();
      _LogMenu.Items.AddRange(new ToolStripItem[] 
      (
        _LogMenuCopy,
        _LogMenuSave,
        _LogMenuClear
      ));
      
      _Log                  := new RichTextBox();
      _Log.Location         := new System.Drawing.Point(1, 1);
      _Log.Size             := new System.Drawing.Size(Width-2, Height-2);
      _Log.ReadOnly         := true;
      _Log.BackColor        := Color.White;
      _Log.BorderStyle      := BorderStyle.None;
      _Log.ScrollBars       := RichTextBoxScrollBars.ForcedVertical;
      _Log.Font             := new System.Drawing.Font('Consolas', 10, FontStyle.Regular, GraphicsUnit.Point);
      _Log.ContextMenuStrip := _LogMenu;
      Controls.Add(_Log);
    end;
    {$endregion}
    
    {$region Properties}
    public property Size: System.Drawing.Size read GetSize write SetSize;
    
    public property TabStop: boolean read GetTabStop write SetTabStop;
    {$endregion}
    
    {$region Methods}
    private procedure LogMenuClearClick(sender: object; e: EventArgs) := _Log.Clear();
    
    private procedure LogMenuCopyClick(sender: object; e: EventArgs) := _Log.Copy();
    
    private procedure LogMenuSaveClick(sender: object; e: EventArgs);
    begin
      var dialog         := new SaveFileDialog();
      dialog.Filter      := 'Plain text file (*.txt)|*.txt|Log file (*.log)|*.log';
      dialog.FilterIndex := 1;
      dialog.FileName    := DateTime.Now.ToString('yyyyMMdd-HHmmss')+'.log';
      
      if dialog.ShowDialog() = DialogResult.OK then
        &File.WriteAllText(dialog.FileName, _Log.Text);
      
      dialog.Dispose();
    end;
    
    public procedure Append(text: string; color: System.Drawing.Color; newline: boolean := true);
    begin
      _Log.SelectionColor := color;
      _Log.AppendText(text);
    
      if newline then
        begin
          _Log.SelectionColor := System.Drawing.Color.Black;
          _Log.AppendText(#13#10);
        end;
    
      _Log.ScrollToCaret();
    end;
    {$endregion}
  end;


end.